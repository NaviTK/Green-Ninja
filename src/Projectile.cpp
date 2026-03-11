#include "Projectile.hpp"
#include "Grid.hpp"
#include <cmath>

// En algunos compiladores de Windows, M_PI no viene por defecto a menos que lo definas
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Añadimos el parámetro 'dmg' al final
Projectile::Projectile(float x, float y, float targetX, float targetY, SDL_Texture *tex, float spd, float rng, int dmg)
    : Entity(x, y, tex), speed(spd), maxRange(rng), distanceTraveled(0), damage(dmg)
{
    float deltaX = targetX - x;
    float deltaY = targetY - y;
    float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

    if (distance > 0)
    {
        dirX = deltaX / distance;
        dirY = deltaY / distance;

        // atan2 nos da el ángulo en radianes. Lo multiplicamos para pasarlo a grados.
        angle = std::atan2(deltaY, deltaX) * (180.0 / M_PI);
    }
    else
    {
        dirX = 0;
        dirY = -1;     // Dispara hacia arriba por defecto
        angle = -90.0; // Ángulo mirando hacia arriba
    }

    srcRect = {0, 0, 32, 32};
    destRect.w = 16;
    destRect.h = 16;

    // Inicializamos la posición visual desde el primer frame
    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);
}

void Projectile::update(double deltaTime, Grid *grid)
{
    // 1. Mover la posición lógica
    x += dirX * speed * deltaTime;
    y += dirY * speed * deltaTime;

    // 2. Actualizar el rectángulo de dibujo
    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);

    distanceTraveled += speed * deltaTime;
    angle += 360.0 * deltaTime; // Rota 360 grados por segundo (ajusta esto para cambiar la velocidad de rotación)
}

// --- NUEVO: Sobrescribimos el render para poder rotar la textura ---
void Projectile::render(SDL_Renderer *renderer, const SDL_Rect &camera)
{
    SDL_Rect renderRect = {
        destRect.x - camera.x,
        destRect.y - camera.y,
        destRect.w,
        destRect.h};

    TextureManager::DrawRotated(texture, srcRect, renderRect, angle, renderer);
}

bool Projectile::isExpired() const
{
    return distanceTraveled >= maxRange;
}

// --- NUEVO: Para el sistema de colisiones en Game.cpp ---
SDL_Rect Projectile::getCollider() const
{
    // El rectángulo visual nos sirve perfectamente como "Hitbox"
    return destRect;
}

// --- NUEVO: Para hacer daño al enemigo ---
float Projectile::getDamage() const
{
    return damage;
}

void Projectile::setSize(float sizeMultiplier)
{
    // Modificamos el destRect desde dentro de la propia clase, donde sí tenemos permiso
    destRect.w = static_cast<int>(16 * sizeMultiplier);
    destRect.h = static_cast<int>(16 * sizeMultiplier);
}

float Projectile::getKnockback() const
{
    return knockback;
}