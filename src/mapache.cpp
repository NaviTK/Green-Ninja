#include "Mapache.hpp"
#include <iostream>
#include <cmath>

// 1. CONSTRUCTOR
Mapache::Mapache(MapCoord startPos, Player *playerTarget, Room *hab, SDL_Texture *texturaMapache)
    : Enemy(startPos, playerTarget, hab)
{
    // Variables de Enemy (heredadas)
    health = 100.0f;
    damage = 10;
    aggroRange = 8;
    attackRange = 0;

    currentPos = startPos;
    targetX = x;
    targetY = y;
    moveSpeed = 100.0f;
    oriented = SOUTH;
    isMoving = false;

    // Variables de Animación de Enemy
    this->texture = texturaMapache;
    frameWidth = 16;
    frameHeight = 16;
    srcRect = {0, 0, frameWidth, frameHeight};
    numFrames = 4;         // Suponiendo que tienes 4 frames. ¡Cámbialo si son otros!
    TIME_PER_FRAME = 0.15; // Velocidad de animación
    animTimer = 0.0;
    currentFrame = 0;

    float escala = 3.0f;
    destRect.w = static_cast<int>(frameWidth * escala);
    destRect.h = static_cast<int>(frameHeight * escala);

    // Variables de daño de Enemy
    dmgTimer = 0.0f;
    isTakingDmg = false;
    TakingDmgOffset = 64;

    // Variables exclusivas del Mapache
    attackCooldown = 1.5;
    timeSinceLastAttack = 0.0;
}

// 2. UPDATE ORQUESTADOR
void Mapache::update(double deltaTime, Grid *grid)
{
    timeSinceLastAttack += deltaTime;

    float oldX = x;
    float oldY = y;

    bool isKnockedBack = (std::abs(knockbackVX) > 15.0f || std::abs(knockbackVY) > 15.0f);

    if (isKnockedBack)
    {
        manejarRetroceso(deltaTime, grid); // Llama a la función de Enemy
    }
    else
    {
        actualizarComportamientoNormal(deltaTime, grid); // Llama a la propia
    }

    // Funciones heredadas de Enemy para pintar y orientar
    actualizarDireccionMirada(oldX, oldY);
    animationLogic(deltaTime);

    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);
}

// 3. ATAQUE
void Mapache::attack()
{
    std::cout << " El mapache ataca ferozmente." << std::endl;
}

// 4. EL CEREBRO DEL MAPACHE (IA de persecución)
void Mapache::actualizarComportamientoNormal(double deltaTime, Grid *grid)
{
    int tileSize = grid->getTileSize();

    knockbackVX = 0.0f;
    knockbackVY = 0.0f;

    float dx = targetX - x;
    float dy = targetY - y;
    float distanceToTarget = std::sqrt(dx * dx + dy * dy);

    // Nos movemos hacia la siguiente casilla
    if (distanceToTarget > 1.0f)
    {
        x += (dx / distanceToTarget) * moveSpeed * deltaTime;
        y += (dy / distanceToTarget) * moveSpeed * deltaTime;
        isMoving = true;
    }
    else // Buscamos la próxima casilla objetivo
    {
        x = targetX;
        y = targetY;
        isMoving = false;

        float mapacheCenterX = x + (destRect.w / 2.0f);
        float mapacheCenterY = y + (destRect.h / 2.0f);

        int gridX = static_cast<int>(std::floor(mapacheCenterX / tileSize));
        int gridY = static_cast<int>(std::floor(mapacheCenterY / tileSize));

        if (gridX < 0)
            gridX = 0;
        if (gridY < 0)
            gridY = 0;
        if (gridX >= grid->getCols())
            gridX = grid->getCols() - 1;
        if (gridY >= grid->getRows())
            gridY = grid->getRows() - 1;

        MapCoord miPos = {gridX, gridY};
        currentPos = miPos;

        float playerCenterX = target->getX() + (target->getDestRect().w / 2.0f);
        float playerCenterY = target->getY() + (target->getDestRect().h / 2.0f);

        int targetGridX = static_cast<int>(std::floor(playerCenterX / tileSize));
        int targetGridY = static_cast<int>(std::floor(playerCenterY / tileSize));

        if (targetGridX < 0)
            targetGridX = 0;
        if (targetGridY < 0)
            targetGridY = 0;
        if (targetGridX >= grid->getCols())
            targetGridX = grid->getCols() - 1;
        if (targetGridY >= grid->getRows())
            targetGridY = grid->getRows() - 1;

        MapCoord targetPos = {targetGridX, targetGridY};

        int distanciaEnCasillas = std::max(std::abs(miPos.first - targetPos.first), std::abs(miPos.second - targetPos.second));

        if (distanciaEnCasillas <= attackRange)
        {
            if (timeSinceLastAttack >= attackCooldown)
            {
                attack();
                timeSinceLastAttack = 0.0;
            }
        }
        else if (distanciaEnCasillas <= aggroRange)
        {
            // Usamos el A* de Enemy
            std::vector<MapCoord> ruta = calcularCaminoAStar(grid, miPos, targetPos);

            if (!ruta.empty())
            {
                MapCoord siguientePaso = ruta[0];
                targetX = siguientePaso.first * tileSize;
                targetY = siguientePaso.second * tileSize;
            }
            else
            {
                // Movimiento directo de desatasco
                float rDx = playerCenterX - mapacheCenterX;
                float rDy = playerCenterY - mapacheCenterY;
                float rDist = std::sqrt(rDx * rDx + rDy * rDy);

                if (rDist > 0.0f)
                {
                    targetX = x + (rDx / rDist) * 4.0f;
                    targetY = y + (rDy / rDist) * 4.0f;
                }
            }
        }
    }
}