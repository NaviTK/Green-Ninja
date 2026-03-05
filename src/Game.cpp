#include "Game.hpp"
#include "TextureManager.hpp"
#include "ConfigLoader.hpp"
#include "Projectile.hpp"
#include "RoomType.hpp"
#include <string>
#include <iostream>

Game::Game() : window(nullptr), renderer(nullptr), isRunning(false), player(nullptr), baseRoom(nullptr) {}

Game::~Game() {}

void Game::Inicialize(int width, int height)
{
    windowWidht = width;
    windowHeight = height;
    camara = {0, 0, windowWidht, windowHeight};
    mapa = Mapa(2 * depth + 1, std::vector<int>(2 * depth + 1, -1)); // Inicializamos el mapa con -1 (sin habitación)
}

void Game::LoadAllTextures(SDL_Renderer *renderer)
{
    // Cargamos las rutas desde el archivo
    auto spritePaths = ConfigLoader::LoadSprites("../assets/sprites.txt");

    // 1. Cargar texturas estáticas/globales
    Player::LoadTexture(spritePaths["player"], renderer);
    projectileTexture = TextureManager::LoadTexture(spritePaths["projectile"].c_str(), renderer);

    // 2. Cargar texturas para todas las habitaciones generadas
    // Recorremos el std::map y cargamos las texturas de la grilla de cada Room
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
            // 1. Generamos el nivel primero para saber cuánto mide
            baseRoom = Room::GenerarNivel(depth, rooms, mapa);

            // Calculamos el tamaño real de la habitación en píxeles
            int roomPixelWidth = baseRoom->getGrid()->getCols() * baseRoom->getGrid()->getTileSize();
            int roomPixelHeight = baseRoom->getGrid()->getRows() * baseRoom->getGrid()->getTileSize();

            // Le decimos a SDL que nuestra resolución interna es exactamente el tamaño de la sala.
            SDL_RenderSetLogicalSize(renderer, roomPixelWidth, roomPixelHeight);
            // ---------------------------------
            LoadAllTextures(renderer);
            // 3. CREAMOS AL JUGADOR PRIMERO
            // Así nos aseguramos de que el renderer existe y se lo pasamos al constructor
            int tileSize = baseRoom->getGrid()->getTileSize();
            int colCentro = baseRoom->getGrid()->getCols() / 2;
            int filaCentro = baseRoom->getGrid()->getRows() / 2;

            float spawnX = colCentro * tileSize;
            float spawnY = filaCentro * tileSize;

            player = new Player(spawnX, spawnY, renderer);

            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            isRunning = true;

            // 5. CONECTAMOS EL BOTÓN DE DISPARO
            player->setShootCallback([this](float pX, float pY, int mX, int mY, float projectileSpeed, float projectileSize, float range, float damage)
                                     {
                // IMPORTANTE: Al usar LogicalSize, las coordenadas del ratón ya se escalan solas.
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
    // Solo actualizamos si tenemos una habitación válida
    if (player && baseRoom)
    {
        checkRoomTransition();
        // 1. Actualizamos la posición y lógica del jugador contra la grilla de la sala ACTUAL
        player->update(deltaTime, baseRoom->getGrid());

        // 2. Actualizamos la lógica de la sala actual (enemigos, puertas, etc.)
        baseRoom->update(deltaTime);

        // NOTA: Como usamos SDL_RenderSetLogicalSize, la cámara ya no necesita perseguir al jugador.
        // SDL ajusta y centra toda la habitación automáticamente en la pantalla.
        camara.x = 0;
        camara.y = 0;
    }

    // 3. Actualizamos y limpiamos los proyectiles
    for (int i = 0; i < (int)projectiles.size(); i++)
    {
        // Los proyectiles chocan contra las paredes de la habitación actual
        projectiles[i]->update(deltaTime, baseRoom->getGrid());

        if (projectiles[i]->isExpired())
        {
            delete projectiles[i];
            projectiles.erase(projectiles.begin() + i);
            i--;
        }
    }
}

void Game::render()
{
    SDL_RenderClear(renderer);

    // 1. Dibujamos la habitación en la que estamos (suelo y paredes)
    if (baseRoom)
    {
        baseRoom->render(renderer, camara);
    }

    // 2. El Jugador
    if (player)
    {
        player->render(renderer, camara);
    }

    // 3. Los proyectiles
    for (auto p : projectiles)
    {
        p->render(renderer, camara);
    }

    SDL_RenderPresent(renderer);
}

void Game::clean()
{
    delete player;

    // Limpiamos TODAS las habitaciones del mapa
    for (auto const &pair : rooms)
    {
        delete pair.second; // Borra el objeto Room de la memoria
    }
    rooms.clear();      // Vacia el std::map
    baseRoom = nullptr; // Ya fue borrado en el bucle anterior

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

    // 1. Calculamos el centro del jugador
    float playerCenterX = player->getX() + (player->getDestRect().w / 2.0f);
    float playerCenterY = player->getY() + (player->getDestRect().h / 2.0f);

    Grid *grid = baseRoom->getGrid();
    int tileSize = grid->getTileSize();

    // 2. Sacamos en qué casilla (columna y fila) está parado el jugador
    int playerCol = static_cast<int>(playerCenterX) / tileSize;
    int playerRow = static_cast<int>(playerCenterY) / tileSize;

    // PROTECCIÓN EXTRA: Si el jugador se sale ligeramente de la pantalla, abortamos
    // para no leer posiciones raras en la matriz y evitar un crash.
    if (playerCol < 0 || playerCol >= grid->getCols() ||
        playerRow < 0 || playerRow >= grid->getRows())
    {
        return;
    }

    if (weAreInADoor(playerCol, playerRow, grid))
    {
        // Variables para la nueva sala
        Room *nextRoom = nullptr;
        float newPlayerX = player->getX();
        float newPlayerY = player->getY();

        // 4. Averiguamos qué puerta es y preparamos el salto
        if (playerRow == 0) // Puerta NORTE (arriba)
        {
            // ¡CAMBIO AQUÍ! Usamos baseRoom en lugar de currentRoom
            nextRoom = baseRoom->getTopRoom();
            newPlayerY = (grid->getRows() - 2) * tileSize;
        }
        else if (playerRow == grid->getRows() - 1) // Puerta SUR (abajo)
        {
            nextRoom = baseRoom->getBottomRoom();
            newPlayerY = 1 * tileSize;
        }
        else if (playerCol == 0) // Puerta OESTE (izquierda)
        {
            nextRoom = baseRoom->getLeftRoom();
            newPlayerX = (grid->getCols() - 2) * tileSize;
        }
        else if (playerCol == grid->getCols() - 1) // Puerta ESTE (derecha)
        {
            nextRoom = baseRoom->getRightRoom();
            newPlayerX = 1 * tileSize;
        }

        // 5. ¡Hacemos el cambio!
        // Asegurarnos de que nextRoom no sea nulo antes de teletransportarnos
        if (nextRoom != nullptr)
        {
            baseRoom = nextRoom; // Cambiamos la sala que se dibuja

            // Movemos al jugador a su nueva posición
            player->setX(newPlayerX);
            player->setY(newPlayerY);

            // Borrar los proyectiles al cambiar de sala
            for (auto p : projectiles)
                delete p;
            projectiles.clear();
        }
        else
        {
            // (Opcional) Chivato por si cruzas una puerta que no lleva a ningún sitio
            std::cout << "¡Ups! Esta puerta no lleva a ninguna sala (nextRoom es nullptr)" << std::endl;
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