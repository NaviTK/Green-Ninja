#include "green-ninja/Game.hpp"
#include "green-ninja/TextureManager.hpp"
#include "green-ninja/ConfigLoader.hpp"
#include "green-ninja/Projectile.hpp"
#include "green-ninja/RoomType.hpp"
#include "green-ninja/Mapache.hpp"
#include "green-ninja/ProjectileModifiers.hpp"
#include <string>
#include <iostream>
#include <cmath>

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

// --- FUNCIÓN MATEMÁTICA DE COLISIÓN (AABB) ---
static bool checkCollisionAABB(const SDL_Rect &a, const SDL_Rect &b)
{
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

// ============================================================================
// CONSTRUCTOR Y DESTRUCTOR
// ============================================================================

Game::Game() : window(nullptr), renderer(nullptr), isRunning(false), player(nullptr), currentRoom(nullptr), baseRoom(nullptr) {}

Game::~Game()
{
    if (playerTexture)
        SDL_DestroyTexture(playerTexture);
    if (mapacheTexture)
        SDL_DestroyTexture(mapacheTexture);
    if (projectileTexture)
        SDL_DestroyTexture(projectileTexture);
}

// ============================================================================
// INICIALIZACIÓN
// ============================================================================

bool Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen)
{
    Inicialize(width, height);

    if (!initSDL(title, xpos, ypos, width, height, fullscreen))
        return false;

    initGameWorld();
    return true;
}

bool Game::initSDL(const char *title, int xpos, int ypos, int width, int height, bool fullscreen)
{
    int flags = 0;
    if (fullscreen)
        flags = SDL_WINDOW_FULLSCREEN;

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer)
            return true;
    }
    return false;
}

void Game::Inicialize(int width, int height)
{
    windowWidht = width;
    windowHeight = height;
    camara = {0, 0, windowWidht, windowHeight};
    mapa = Mapa(2 * depth + 1, std::vector<int>(2 * depth + 1, -1));
}

void Game::initGameWorld()
{
    baseRoom = Room::GenerarNivel(depth, rooms, mapa);
    currentRoom = baseRoom;

    int roomPixelWidth = currentRoom->getGrid()->getCols() * currentRoom->getGrid()->getTileSize();
    int roomPixelHeight = currentRoom->getGrid()->getRows() * currentRoom->getGrid()->getTileSize();

    SDL_RenderSetLogicalSize(renderer, roomPixelWidth, roomPixelHeight);

    LoadAllTextures(renderer);

    int tileSize = currentRoom->getGrid()->getTileSize();
    int colCentro = currentRoom->getGrid()->getCols() / 2;
    int filaCentro = currentRoom->getGrid()->getRows() / 2;

    float spawnX = colCentro * tileSize;
    float spawnY = filaCentro * tileSize;

    player = new Player(spawnX, spawnY, renderer, playerTexture);

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    isRunning = true;

    player->setShootCallback([this](float pX, float pY, int mX, int mY, float projectileSpeed, float projectileSize, float range, float damage) -> Projectile *
                             {
        float realSpeed = projectileSpeed * 100.0f;
        float realRange = range * 100.0f;

        Projectile* p = new Projectile(pX, pY, mX, mY, this->projectileTexture, realSpeed, realRange, damage);
        p->setSize(projectileSize);
        
        this->projectiles.push_back(p); 
        return p; });
}

void Game::LoadAllTextures(SDL_Renderer *renderer)
{
    auto spritePaths = ConfigLoader::LoadSprites("../assets/sprites.txt");

    playerTexture = TextureManager::LoadTexture(spritePaths["player"].c_str(), renderer);
    mapacheTexture = TextureManager::LoadTexture(spritePaths["mapache"].c_str(), renderer);
    projectileTexture = TextureManager::LoadTexture(spritePaths["projectile"].c_str(), renderer);

    if (!rooms.empty())
    {
        for (auto const &pair : rooms)
        {
            Room *room = pair.second;
            if (room && room->getGrid())
            {
                room->getGrid()->loadTextures(renderer, spritePaths);
            }
        }
    }
    else
    {
        std::cerr << "Error: No hay habitaciones generadas en LoadAllTextures" << std::endl;
    }
}

// ============================================================================
// EVENTOS E INPUT
// ============================================================================

void Game::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            isRunning = false;

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_ESCAPE)
                isRunning = false;
        }
    }
}

// ============================================================================
// LÓGICA DEL JUEGO (UPDATE)
// ============================================================================

void Game::update(double deltaTime)
{
    if (!player || !currentRoom)
        return;

    checkRoomTransition();
    updateEntities(deltaTime);
    updateProjectiles(deltaTime);
    checkCollisions();
    checkRoomCleared();
}

void Game::updateEntities(double deltaTime)
{
    player->update(deltaTime, currentRoom->getGrid());

    for (auto e : enemies)
        e->update(deltaTime, currentRoom->getGrid());

    currentRoom->update(deltaTime);
    camara.x = 0;
    camara.y = 0;
}

void Game::updateProjectiles(double deltaTime)
{
    Grid *grid = currentRoom->getGrid();
    int tileSize = grid->getTileSize();

    for (int i = 0; i < (int)projectiles.size(); i++)
    {
        projectiles[i]->update(deltaTime, grid);

        bool destroy = false;
        SDL_Rect projRect = projectiles[i]->getDestRect();

        if (projectiles[i]->isExpired())
        {
            destroy = true;
        }
        else
        {
            int gridX = (projRect.x + projRect.w / 2) / tileSize;
            int gridY = (projRect.y + projRect.h / 2) / tileSize;

            if (gridX >= 0 && gridX < grid->getCols() && gridY >= 0 && gridY < grid->getRows())
            {
                Tile currentTile = grid->GetTileAt(gridX, gridY);
                if (currentTile.hasType(TileType::WALL) ||
                    currentTile.hasType(TileType::ROCK1) ||
                    currentTile.hasType(TileType::ROCK2))
                {
                    destroy = true;
                }
            }
            else
            {
                destroy = true; // Salió del mapa
            }
        }

        if (destroy)
        {
            delete projectiles[i];
            projectiles.erase(projectiles.begin() + i);
            i--; // Ajustamos el índice tras borrar
        }
    }
}

void Game::checkCollisions()
{
    SDL_Rect playerRect = player->getDestRect();

    // 1. Enemigo colisiona con Jugador
    for (auto e : enemies)
    {
        SDL_Rect enemyRect = e->getDestRect();
        if (checkCollisionAABB(playerRect, enemyRect))
        {
            player->takeDamage(e->getDamage(), e->getCoord());
        }
    }

    // 2. Proyectil colisiona con Enemigo
    for (int i = 0; i < (int)projectiles.size(); i++)
    {
        SDL_Rect projRect = projectiles[i]->getDestRect();
        bool projectileHit = false;

        for (int j = 0; j < (int)enemies.size(); j++)
        {
            SDL_Rect enemyRect = enemies[j]->getDestRect();

            if (checkCollisionAABB(projRect, enemyRect))
            {
                enemies[j]->takeDamage(projectiles[i]);
                projectileHit = true;

                if (enemies[j]->isDead())
                {
                    delete enemies[j];
                    enemies.erase(enemies.begin() + j);
                    j--; // Ajustamos el índice
                }
                break; // Un proyectil solo impacta a un enemigo
            }
        }

        if (projectileHit)
        {
            delete projectiles[i];
            projectiles.erase(projectiles.begin() + i);
            i--; // Ajustamos el índice
        }
    }
}

// ============================================================================
// GESTIÓN DE SALAS Y TRANSICIONES
// ============================================================================

void Game::checkRoomCleared()
{
    if (currentRoom && !currentRoom->IsCleared() && enemies.empty())
    {
        currentRoom->setCleared(true);
        std::cout << "¡Habitación " << currentRoom->getId() << " completada!" << std::endl;
        // Lógica de abrir puertas...
    }
}

void Game::checkRoomTransition()
{
    if (!player || !currentRoom)
        return;

    Grid *grid = currentRoom->getGrid();
    int tileSize = grid->getTileSize();

    int playerCol = static_cast<int>(player->getX() + (player->getDestRect().w / 2.0f)) / tileSize;
    int playerRow = static_cast<int>(player->getY() + (player->getDestRect().h / 2.0f)) / tileSize;

    if (playerCol < 0 || playerCol >= grid->getCols() || playerRow < 0 || playerRow >= grid->getRows())
        return;

    if (weAreInADoor(playerCol, playerRow, grid))
    {
        Room *nextRoom = nullptr;
        float newPlayerX = player->getX();
        float newPlayerY = player->getY();

        if (playerRow == 0)
        {
            nextRoom = currentRoom->getTopRoom();
            newPlayerY = (grid->getRows() - 2) * tileSize;
        }
        else if (playerRow == grid->getRows() - 1)
        {
            nextRoom = currentRoom->getBottomRoom();
            newPlayerY = 1 * tileSize;
        }
        else if (playerCol == 0)
        {
            nextRoom = currentRoom->getLeftRoom();
            newPlayerX = (grid->getCols() - 2) * tileSize;
        }
        else if (playerCol == grid->getCols() - 1)
        {
            nextRoom = currentRoom->getRightRoom();
            newPlayerX = 1 * tileSize;
        }

        if (nextRoom != nullptr)
        {
            clearRoomEntities();

            currentRoom = nextRoom;
            player->setX(newPlayerX);
            player->setY(newPlayerY);

            if (!currentRoom->IsCleared())
            {
                spawnRoomEnemies();
            }
        }
    }
}

bool Game::weAreInADoor(int col, int row, Grid *grid)
{
    if (grid->GetTileAt(col, row).hasType(TileType::RIGHTDOOR))
        return true;
    if (grid->GetTileAt(col, row).hasType(TileType::LEFTDOOR))
        return true;
    if (grid->GetTileAt(col, row).hasType(TileType::TOPDOOR))
        return true;
    if (grid->GetTileAt(col, row).hasType(TileType::BOTTOMDOOR))
        return true;
    return false;
}

void Game::clearRoomEntities()
{
    for (auto e : enemies)
        delete e;
    enemies.clear();

    for (auto p : projectiles)
        delete p;
    projectiles.clear();
}

void Game::spawnRoomEnemies()
{
    const auto &spawns = currentRoom->getSpawnsEnemigos();
    for (const auto &spawn : spawns)
    {
        if (spawn.tipo == 'm')
        {
            MapCoord pos = {spawn.col, spawn.row};
            Mapache *nuevoMapache = new Mapache(pos, player, currentRoom, mapacheTexture);
            enemies.push_back(nuevoMapache);
        }
    }
    std::cout << "¡Generados " << enemies.size() << " enemigos en la sala " << currentRoom->getId() << "!" << std::endl;
}

// ============================================================================
// RENDERIZADO
// ============================================================================

void Game::render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (currentRoom)
        currentRoom->render(renderer, camara);

    for (auto e : enemies)
        e->render(renderer, camara);

    if (player)
        player->render(renderer, camara);

    for (auto p : projectiles)
        p->render(renderer, camara);

    renderHUD();

    SDL_RenderPresent(renderer);
}

void Game::renderHUD()
{
    if (!player)
        return;

    renderHealth();
    renderMinimap();
}

void Game::renderHealth()
{
    int margin = 20;
    int heartSize = 30;
    int spacing = 5;
    int heartsToDraw = static_cast<int>(player->getHealth()) / 20;

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < heartsToDraw; i++)
    {
        SDL_Rect heartRect = {margin + (i * (heartSize + spacing)), margin, heartSize, heartSize};
        SDL_RenderFillRect(renderer, &heartRect);
    }
}

void Game::renderMinimap()
{
    int miniMapTileSize = 24;
    int mapMarginTop = 60;
    int mapMarginRight = 60;

    int logicalW, logicalH;
    SDL_RenderGetLogicalSize(renderer, &logicalW, &logicalH);

    int currentX = -1, currentY = -1;
    for (size_t x = 0; x < mapa.size(); x++)
    {
        for (size_t y = 0; y < mapa[x].size(); y++)
        {
            int roomId = mapa[x][y];
            if (roomId != -1 && rooms.find(roomId) != rooms.end() && rooms[roomId] == currentRoom)
            {
                currentX = x;
                currentY = y;
                break;
            }
        }
        if (currentX != -1)
            break;
    }

    int uiCenterX = logicalW - mapMarginRight;
    int uiCenterY = mapMarginTop;

    if (currentX != -1 && currentY != -1)
    {
        for (size_t x = 0; x < mapa.size(); x++)
        {
            for (size_t y = 0; y < mapa[x].size(); y++)
            {
                int roomId = mapa[x][y];
                if (roomId != -1 && rooms.find(roomId) != rooms.end())
                {
                    int diffX = static_cast<int>(x) - currentX;
                    int diffY = static_cast<int>(y) - currentY;

                    if (std::abs(diffX) <= 2 && std::abs(diffY) <= 2)
                    {
                        Room *roomToDraw = rooms[roomId];
                        SDL_Rect miniRoom = {
                            uiCenterX + (diffX * miniMapTileSize) - (miniMapTileSize / 2),
                            uiCenterY + (diffY * miniMapTileSize) - (miniMapTileSize / 2),
                            miniMapTileSize, miniMapTileSize};

                        RoomType type = roomToDraw->getType();
                        if (type == RoomType::START)
                            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                        else if (type == RoomType::BOSS)
                            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                        else
                            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);

                        SDL_RenderFillRect(renderer, &miniRoom);

                        // Pintar Puertas
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                        int doorLength = 8, doorThickness = 3;

                        if (roomToDraw->getTopRoom() != nullptr)
                        {
                            SDL_Rect r = {miniRoom.x + (miniMapTileSize / 2) - (doorLength / 2), miniRoom.y, doorLength, doorThickness};
                            SDL_RenderFillRect(renderer, &r);
                        }
                        if (roomToDraw->getBottomRoom() != nullptr)
                        {
                            SDL_Rect r = {miniRoom.x + (miniMapTileSize / 2) - (doorLength / 2), miniRoom.y + miniMapTileSize - doorThickness, doorLength, doorThickness};
                            SDL_RenderFillRect(renderer, &r);
                        }
                        if (roomToDraw->getLeftRoom() != nullptr)
                        {
                            SDL_Rect r = {miniRoom.x, miniRoom.y + (miniMapTileSize / 2) - (doorLength / 2), doorThickness, doorLength};
                            SDL_RenderFillRect(renderer, &r);
                        }
                        if (roomToDraw->getRightRoom() != nullptr)
                        {
                            SDL_Rect r = {miniRoom.x + miniMapTileSize - doorThickness, miniRoom.y + (miniMapTileSize / 2) - (doorLength / 2), doorThickness, doorLength};
                            SDL_RenderFillRect(renderer, &r);
                        }

                        if (roomToDraw == currentRoom)
                        {
                            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                            SDL_RenderDrawRect(renderer, &miniRoom);

                            Grid *grid = currentRoom->getGrid();
                            float pctX = (player->getX() + (player->getDestRect().w / 2.0f)) / (grid->getCols() * grid->getTileSize());
                            float pctY = (player->getY() + (player->getDestRect().h / 2.0f)) / (grid->getRows() * grid->getTileSize());

                            int dotSize = 4;
                            SDL_Rect playerDot = {
                                miniRoom.x + static_cast<int>(pctX * miniRoom.w) - (dotSize / 2),
                                miniRoom.y + static_cast<int>(pctY * miniRoom.h) - (dotSize / 2),
                                dotSize, dotSize};

                            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                            SDL_RenderFillRect(renderer, &playerDot);
                        }
                        else
                        {
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                            SDL_RenderDrawRect(renderer, &miniRoom);
                        }
                    }
                }
            }
        }
    }
}

// ============================================================================
// LIMPIEZA DE MEMORIA
// ============================================================================

void Game::clean()
{
    delete player;

    for (auto e : enemies)
        delete e;
    enemies.clear();

    for (auto const &pair : rooms)
        delete pair.second;
    rooms.clear();

    currentRoom = nullptr;
    baseRoom = nullptr;

    for (auto p : projectiles)
        delete p;
    projectiles.clear();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}