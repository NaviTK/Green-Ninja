#include "Game.hpp"
#include "TextureManager.hpp"
#include "ConfigLoader.hpp"
#include "Projectile.hpp"
#include "RoomType.hpp"
#include "Mapache.hpp"
#include <string>
#include <iostream>

// --- NUEVO: FUNCIÓN MATEMÁTICA DE COLISIÓN (AABB) ---
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

            player->setShootCallback([this](float pX, float pY, int mX, int mY, float projectileSpeed, float projectileSize, float range, float damage)
                                     {
                float realSpeed = projectileSpeed * 100.0f;
                float realRange = range * 100.0f;

                Projectile* p = new Projectile(pX, pY, mX, mY, this->projectileTexture, realSpeed, realRange, damage);
                p->setSize(projectileSize);
                
                this->projectiles.push_back(p); });

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
                    // ¡AQUÍ ESTÁ EL CAMBIO PRINCIPAL!
                    // En lugar de pasar projectiles[i]->getDamage(),
                    // le pasamos el puntero al objeto completo.
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
    SDL_RenderClear(renderer);

    if (baseRoom)
        baseRoom->render(renderer, camara);

    for (auto e : enemies)
        e->render(renderer, camara);

    if (player)
        player->render(renderer, camara);

    for (auto p : projectiles)
        p->render(renderer, camara);

    SDL_RenderPresent(renderer);
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