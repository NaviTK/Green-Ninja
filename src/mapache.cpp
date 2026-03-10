#include "Mapache.hpp"
#include <iostream>
#include <cmath>

// (Hemos borrado la definición estática y la función LoadTexture de aquí arriba)

// 1. CONSTRUCTOR ACTUALIZADO
Mapache::Mapache(MapCoord startPos, Player *playerTarget, Room *hab, SDL_Texture *texturaMapache)
    : Enemy(startPos, playerTarget, hab)
{
    health = 30.0f;
    target = playerTarget;
    attackCooldown = 1.5;
    timeSinceLastAttack = 0.0;
    aggroRange = 8;
    attackRange = 1;
    damage = 10;

    targetX = x; // Al nacer, su destino es el mismo lugar donde está
    targetY = y;
    moveSpeed = 100.0f;
    // --- CONFIGURACIÓN GRÁFICA ---
    // Recibimos la textura que nos ha pasado Game y se la guardamos al padre Entity
    this->texture = texturaMapache;

    // Ajusta estos valores al tamaño de tu sprite sheet del Mapache
    frameWidth = 16;
    frameHeight = 16;
    srcRect = {0, 0, frameWidth, frameHeight};

    float escala = 3.0f; // Misma escala que el jugador
    destRect.w = static_cast<int>(frameWidth * escala);
    destRect.h = static_cast<int>(frameHeight * escala);

    oriented = SOUTH;
    isMoving = false;
}

// 2. ANIMATION LOGIC
void Mapache::animationLogic(double deltaTime)
{
    // Si se mueve, alternamos frames
    if (isMoving)
    {
        animTimer += deltaTime;
        if (animTimer >= TIME_PER_FRAME)
        {
            animTimer = 0.0f;
            currentFrame = (currentFrame + 1) % numFrames; // Pasa del 0 al 1, 2, 3 y vuelve al 0
        }
    }
    else
    {
        currentFrame = 0; // Si está quieto, usamos el frame de reposo
    }

    // Calculamos qué parte de la imagen cortar.
    srcRect.x = frameWidth * currentFrame;

    switch (oriented)
    {
    case SOUTH:
        srcRect.y = frameHeight * 0;
        break;
    case NORTH:
        srcRect.y = frameHeight * 1;
        break;
    case WEST:
        srcRect.y = frameHeight * 2;
        break;
    case EAST:
        srcRect.y = frameHeight * 3;
        break;
    }
}

// 3. UPDATE
void Mapache::update(double deltaTime, Grid *grid)
{
    int tileSize = grid->getTileSize();
    timeSinceLastAttack += deltaTime;

    float oldX = x;
    float oldY = y;

    // 1. Calculamos la distancia hasta nuestro pixel de destino
    float dx = targetX - x;
    float dy = targetY - y;
    float distanceToTarget = std::sqrt(dx * dx + dy * dy);

    // Si estamos a más de 1 pixel de nuestro objetivo, seguimos caminando hacia él
    if (distanceToTarget > 1.0f)
    {
        x += (dx / distanceToTarget) * moveSpeed * deltaTime;
        y += (dy / distanceToTarget) * moveSpeed * deltaTime;
        isMoving = true;
    }
    else
    {
        // Hemos llegado a la casilla destino (o casi).
        // Nos "anclamos" exactamente a ella para evitar errores de decimales.
        x = targetX;
        y = targetY;
        isMoving = false;

        // --- AHORA QUE ESTAMOS QUIETOS, PENSAMOS EL SIGUIENTE PASO ---

        // 1. Calculamos el centro del mapache
        float mapacheCenterX = x + (destRect.w / 2.0f);
        float mapacheCenterY = y + (destRect.h / 2.0f);
        MapCoord miPos = {static_cast<int>(mapacheCenterX / tileSize), static_cast<int>(mapacheCenterY / tileSize)};

        // 2. Calculamos el centro del jugador
        float playerCenterX = target->getX() + (target->getDestRect().w / 2.0f);
        float playerCenterY = target->getY() + (target->getDestRect().h / 2.0f);
        MapCoord targetPos = {static_cast<int>(playerCenterX / tileSize), static_cast<int>(playerCenterY / tileSize)};

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
            // Solo calculamos A* cuando terminamos de caminar a la casilla anterior.
            std::vector<MapCoord> ruta = calcularCaminoAStar(grid, miPos, targetPos);

            if (!ruta.empty())
            {
                MapCoord siguientePaso = ruta[0];

                // Le damos sus nuevas coordenadas pixel objetivo
                targetX = siguientePaso.first * tileSize;
                targetY = siguientePaso.second * tileSize;
            }
        }
    }

    // --- LÓGICA DE DIRECCIÓN PARA LA ANIMACIÓN ---
    if (x > oldX)
        oriented = EAST;
    else if (x < oldX)
        oriented = WEST;
    else if (y > oldY)
        oriented = SOUTH;
    else if (y < oldY)
        oriented = NORTH;

    // --- ACTUALIZAR ANIMACIONES Y RECTÁNGULOS ---
    animationLogic(deltaTime);

    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);
}

// 4. ATTACK
void Mapache::attack()
{
    std::cout << " El mapache ataca ferozmente." << std::endl;
}