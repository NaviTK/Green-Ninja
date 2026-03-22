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
// --- FUNCIÓN MATEMÁTICA DE COLISIÓN (AABB) ---
static bool checkCollisionAABB(const SDL_Rect &a, const SDL_Rect &b)
{
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

Game::Game() : window(nullptr), renderer(nullptr), isRunning(false), player(nullptr), baseRoom(nullptr) {}

Game::~Game()
{
    // destruimos texturas
    if (playerTexture)
        SDL_DestroyTexture(playerTexture);
    if (mapacheTexture)
        SDL_DestroyTexture(mapacheTexture);
    if (projectileTexture)
        SDL_DestroyTexture(projectileTexture);
}

void Game::Inicialize(int width, int height)
{
    windowWidht = width;
    windowHeight = height;
    camara = {0, 0, windowWidht, windowHeight};
    mapa = Mapa(2 * depth + 1, std::vector<int>(2 * depth + 1, -1));
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
            Room *currentRoom = pair.second;
            if (currentRoom && currentRoom->getGrid())
            {
                currentRoom->getGrid()->loadTextures(renderer, spritePaths);
            }
        }
    }
    else
    {
        std::cerr << "Error: No hay habitaciones generadas en LoadAllTextures" << std::endl;
    }
}

bool Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen)
{
    Inicialize(width, height);
    int flags = 0;
    if (fullscreen)
        flags = SDL_WINDOW_FULLSCREEN;

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        if (renderer)
        {
            baseRoom = Room::GenerarNivel(depth, rooms, mapa);

            int roomPixelWidth = baseRoom->getGrid()->getCols() * baseRoom->getGrid()->getTileSize();
            int roomPixelHeight = baseRoom->getGrid()->getRows() * baseRoom->getGrid()->getTileSize();

            SDL_RenderSetLogicalSize(renderer, roomPixelWidth, roomPixelHeight);

            LoadAllTextures(renderer);

            int tileSize = baseRoom->getGrid()->getTileSize();
            int colCentro = baseRoom->getGrid()->getCols() / 2;
            int filaCentro = baseRoom->getGrid()->getRows() / 2;

            float spawnX = colCentro * tileSize;
            float spawnY = filaCentro * tileSize;

            player = new Player(spawnX, spawnY, renderer, playerTexture);

            MapCoord mapachePos = {colCentro, filaCentro};
            Mapache *mapacheDePrueba = new Mapache(mapachePos, player, baseRoom, mapacheTexture);
            enemies.push_back(mapacheDePrueba);

            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            isRunning = true;

            // --- AQUÍ ESTÁ EL CALLBACK MODIFICADO ---
            // Ahora devuelve un Projectile* en lugar de void
            player->setShootCallback([this](float pX, float pY, int mX, int mY, float projectileSpeed, float projectileSize, float range, float damage) -> Projectile *
                                     {
                float realSpeed = projectileSpeed * 100.0f;
                float realRange = range * 100.0f;

                Projectile* p = new Projectile(pX, pY, mX, mY, this->projectileTexture, realSpeed, realRange, damage);
                p->setSize(projectileSize);
                
                // Lo guardamos en el Game para que se renderice y actualice físicamente
                this->projectiles.push_back(p); 

                // LO DEVOLVEMOS para que el Player pueda inyectarle sus modificadores actuales
                return p; });

            // prueba de efectos
            // player->addEffect(ProjectileEffect::BLOOD_TEAR);
            // player->addEffect(ProjectileEffect::WIGGLE_WORM);

            return true;
        }
    }
    return false;
}

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

void Game::update(double deltaTime)
{
    if (player && baseRoom)
    {
        checkRoomTransition();
        player->update(deltaTime, baseRoom->getGrid());

        // --- 1. ACTUALIZAR ENEMIGOS Y DAÑO AL JUGADOR ---
        SDL_Rect playerRect = player->getDestRect();

        for (auto e : enemies)
        {
            e->update(deltaTime, baseRoom->getGrid());

            // Comprobar colisión Jugador vs Mapache
            SDL_Rect enemyRect = e->getDestRect();
            if (checkCollisionAABB(playerRect, enemyRect))
            {
                player->takeDamage(e->getDamage(), e->getCoord());
            }
        }

        baseRoom->update(deltaTime);
        camara.x = 0;
        camara.y = 0;
    }

    // --- 2. ACTUALIZAR PROYECTILES Y DAÑO A ENEMIGOS ---
    for (int i = 0; i < (int)projectiles.size(); i++)
    {
        projectiles[i]->update(deltaTime, baseRoom->getGrid());

        bool projectileDestroyed = false;

        // Si el proyectil agotó su rango o chocó con una pared
        if (projectiles[i]->isExpired())
        {
            projectileDestroyed = true;
        }
        else
        {
            // Comprobar colisión Proyectil vs Enemigos
            SDL_Rect projRect = projectiles[i]->getDestRect();

            for (int j = 0; j < (int)enemies.size(); j++)
            {
                SDL_Rect enemyRect = enemies[j]->getDestRect();

                if (checkCollisionAABB(projRect, enemyRect))
                {
                    // En lugar de pasar daño bruto, le pasamos el puntero al proyectil completo.
                    enemies[j]->takeDamage(projectiles[i]);

                    projectileDestroyed = true;

                    // ¿Murió el mapache?
                    if (enemies[j]->isDead())
                    {
                        std::cout << "¡Mapache eliminado!" << std::endl;
                        delete enemies[j];
                        enemies.erase(enemies.begin() + j);
                        // Como borramos el enemigo, decrementamos 'j' para no saltarnos el siguiente
                        j--;
                    }

                    // Un proyectil normal se destruye al golpear un solo enemigo
                    break;
                }
            }
        }

        // Limpiar el proyectil si ha sido destruido
        if (projectileDestroyed)
        {
            delete projectiles[i];
            projectiles.erase(projectiles.begin() + i);
            i--; // Ajustamos el índice del bucle de proyectiles
        }
    }
}

void Game::render()
{
    // --- NUEVO: Ponemos el "pincel" en color Negro antes de limpiar ---
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Limpiamos la pantalla (ahora se pintará de negro)
    SDL_RenderClear(renderer);

    if (baseRoom)
        baseRoom->render(renderer, camara);

    for (auto e : enemies)
        e->render(renderer, camara);

    if (player)
        player->render(renderer, camara);

    for (auto p : projectiles)
        p->render(renderer, camara);

    // Dibujamos la interfaz al final (que volverá a cambiar colores si lo necesita)
    renderHUD();

    SDL_RenderPresent(renderer);
}

void Game::renderHUD()
{
    if (!player)
        return;

    // VIDA E ÍTEMS(items aun no hay)
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

    // MINIMAPA LIMITADO (Distancia <= 2)
    int miniMapTileSize = 24;
    int mapMarginTop = 60;
    int mapMarginRight = 60;

    int logicalW, logicalH;
    SDL_RenderGetLogicalSize(renderer, &logicalW, &logicalH);

    // Encontrar la posición [x][y] de la habitación actual
    int currentX = -1, currentY = -1;
    for (size_t x = 0; x < mapa.size(); x++)
    {
        for (size_t y = 0; y < mapa[x].size(); y++)
        {
            int roomId = mapa[x][y];
            if (roomId != -1 && rooms.find(roomId) != rooms.end() && rooms[roomId] == baseRoom)
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

    // 3. Dibujar SOLO las habitaciones que están a distancia <= 2
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
                            miniMapTileSize,
                            miniMapTileSize};

                        // Colores por tipo de habitación
                        RoomType type = roomToDraw->getType();
                        if (type == RoomType::START)
                            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                        else if (type == RoomType::BOSS)
                            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                        else
                            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);

                        SDL_RenderFillRect(renderer, &miniRoom);

                        // Bordes y jugador
                        if (roomToDraw == baseRoom)
                        {
                            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                            SDL_RenderDrawRect(renderer, &miniRoom);

                            Grid *grid = baseRoom->getGrid();
                            float roomPixelW = grid->getCols() * grid->getTileSize();
                            float roomPixelH = grid->getRows() * grid->getTileSize();

                            float pCX = player->getX() + (player->getDestRect().w / 2.0f);
                            float pCY = player->getY() + (player->getDestRect().h / 2.0f);

                            float pctX = pCX / roomPixelW;
                            float pctY = pCY / roomPixelH;

                            int dotSize = 4;
                            SDL_Rect playerDot = {
                                miniRoom.x + static_cast<int>(pctX * miniRoom.w) - (dotSize / 2),
                                miniRoom.y + static_cast<int>(pctY * miniRoom.h) - (dotSize / 2),
                                dotSize,
                                dotSize};

                            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                            SDL_RenderFillRect(renderer, &playerDot);
                        }
                        else
                        {
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                            SDL_RenderDrawRect(renderer, &miniRoom);
                        }
                    } // Fin del if de distancia
                }
            }
        }
    }
}

void Game::clean()
{
    delete player;

    for (auto e : enemies)
        delete e;
    enemies.clear();

    for (auto const &pair : rooms)
        delete pair.second;
    rooms.clear();
    baseRoom = nullptr;

    for (auto p : projectiles)
        delete p;
    projectiles.clear();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::checkRoomTransition()
{
    if (!player || !baseRoom)
        return;

    float playerCenterX = player->getX() + (player->getDestRect().w / 2.0f);
    float playerCenterY = player->getY() + (player->getDestRect().h / 2.0f);

    Grid *grid = baseRoom->getGrid();
    int tileSize = grid->getTileSize();

    int playerCol = static_cast<int>(playerCenterX) / tileSize;
    int playerRow = static_cast<int>(playerCenterY) / tileSize;

    if (playerCol < 0 || playerCol >= grid->getCols() ||
        playerRow < 0 || playerRow >= grid->getRows())
    {
        return;
    }

    if (weAreInADoor(playerCol, playerRow, grid))
    {
        Room *nextRoom = nullptr;
        float newPlayerX = player->getX();
        float newPlayerY = player->getY();

        if (playerRow == 0)
        {
            nextRoom = baseRoom->getTopRoom();
            newPlayerY = (grid->getRows() - 2) * tileSize;
        }
        else if (playerRow == grid->getRows() - 1)
        {
            nextRoom = baseRoom->getBottomRoom();
            newPlayerY = 1 * tileSize;
        }
        else if (playerCol == 0)
        {
            nextRoom = baseRoom->getLeftRoom();
            newPlayerX = (grid->getCols() - 2) * tileSize;
        }
        else if (playerCol == grid->getCols() - 1)
        {
            nextRoom = baseRoom->getRightRoom();
            newPlayerX = 1 * tileSize;
        }

        if (nextRoom != nullptr)
        {
            baseRoom = nextRoom;
            player->setX(newPlayerX);
            player->setY(newPlayerY);

            for (auto p : projectiles)
                delete p;
            projectiles.clear();
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