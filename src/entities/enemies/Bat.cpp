#include "green-ninja/Bat.hpp"
#include <iostream>
#include <cmath>

// 1. CONSTRUCTOR
Bat::Bat(MapCoord startPos, Player *playerTarget, Room *hab, SDL_Texture *texturaBat)
    : Enemy(startPos, playerTarget, hab)
{
    // Variables de Enemy (heredadas) - Ajustadas para un murciélago
    health = 50.0f;  // Más frágil que el mapache
    damage = 5;      // Hace un poco menos de daño
    aggroRange = 12; // Ve desde más lejos desde el aire
    attackRange = 0; // Necesita tocar al jugador

    currentPos = startPos;
    targetX = x;
    targetY = y;
    moveSpeed = 130.0f; // Vuela más rápido que el mapache
    oriented = SOUTH;
    isMoving = false;

    // Variables de Animación de Enemy
    this->texture = texturaBat;
    frameWidth = 16;
    frameHeight = 16;
    srcRect = {0, 0, frameWidth, frameHeight};
    numFrames = 4;         // Mantengo tus 4 frames
    TIME_PER_FRAME = 0.10; // Animación de aleteo más rápida
    animTimer = 0.0;
    currentFrame = 0;

    float escala = 3.0f;
    destRect.w = static_cast<int>(frameWidth * escala);
    destRect.h = static_cast<int>(frameHeight * escala);

    // Variables de daño de Enemy
    dmgTimer = 0.0f;
    isTakingDmg = false;
    TakingDmgOffset = 64;

    // Variables exclusivas del Murciélago
    attackCooldown = 1.0; // Ataca más seguido
    timeSinceLastAttack = 0.0;
}

// 2. UPDATE ORQUESTADOR
void Bat::update(double deltaTime, Grid *grid)
{
    timeSinceLastAttack += deltaTime;

    float oldX = x;
    float oldY = y;

    bool isKnockedBack = (std::abs(knockbackVX) > 15.0f || std::abs(knockbackVY) > 15.0f);

    if (isKnockedBack)
    {
        manejarRetroceso(deltaTime, grid);
    }
    else
    {
        actualizarComportamientoNormal(deltaTime, grid);
    }

    actualizarDireccionMirada(oldX, oldY);
    animationLogic(deltaTime);

    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);
}

// 3. ATAQUE
void Bat::attack()
{
    std::cout << " El murciélago te muerde desde el aire." << std::endl;
}

// 4. EL CEREBRO DEL MURCIÉLAGO (Vuelo directo)
void Bat::actualizarComportamientoNormal(double deltaTime, Grid *grid)
{
    knockbackVX = 0.0f;
    knockbackVY = 0.0f;

    // Calculamos el centro del murciélago y del jugador
    float batCenterX = x + (destRect.w / 2.0f);
    float batCenterY = y + (destRect.h / 2.0f);

    float playerCenterX = target->getX() + (target->getDestRect().w / 2.0f);
    float playerCenterY = target->getY() + (target->getDestRect().h / 2.0f);

    // Calculamos el vector de distancia directo (Ignorando el A* del grid)
    float dx = playerCenterX - batCenterX;
    float dy = playerCenterY - batCenterY;
    float distanceToPlayer = std::sqrt(dx * dx + dy * dy);

    int tileSize = grid->getTileSize();

    // Convertimos los rangos de casillas a píxeles para la persecución en tiempo real
    float pixelAggroRange = aggroRange * tileSize;
    float pixelAttackRange = (destRect.w / 2.0f) + 5.0f; // Margen de colisión para morder

    // Lógica de ataque o persecución
    if (distanceToPlayer <= pixelAttackRange)
    {
        isMoving = false;
        if (timeSinceLastAttack >= attackCooldown)
        {
            attack();
            timeSinceLastAttack = 0.0;
        }
    }
    else if (distanceToPlayer <= pixelAggroRange)
    {
        isMoving = true;
        // Normalizamos el vector y lo multiplicamos por la velocidad y el deltaTime
        x += (dx / distanceToPlayer) * moveSpeed * deltaTime;
        y += (dy / distanceToPlayer) * moveSpeed * deltaTime;
    }
    else
    {
        isMoving = false;
    }

    // Mantenemos currentPos actualizado por si otras funciones de la clase base (Enemy) lo necesitan
    int gridX = static_cast<int>(std::floor(batCenterX / tileSize));
    int gridY = static_cast<int>(std::floor(batCenterY / tileSize));

    // Límites por seguridad
    if (gridX < 0)
        gridX = 0;
    if (gridY < 0)
        gridY = 0;
    if (gridX >= grid->getCols())
        gridX = grid->getCols() - 1;
    if (gridY >= grid->getRows())
        gridY = grid->getRows() - 1;

    currentPos = {gridX, gridY};
}