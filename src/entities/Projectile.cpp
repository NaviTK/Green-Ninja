#include "green-ninja/Projectile.hpp"
#include "green-ninja/Grid.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Projectile::Projectile()
    : Entity(x, y, nullptr), speed(0), maxRange(0), distanceTraveled(0), damage(10), alive(false)
{
    /*float x, float y, float targetX, float targetY,
    float deltaX = targetX - x;
    float deltaY = targetY - y;
    float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

    if (distance > 0)
    {
        dirX = deltaX / distance;
        dirY = deltaY / distance;
        angle = std::atan2(deltaY, deltaX) * (180.0 / M_PI);
    }
    else
    {
        dirX = 0;
        dirY = -1;
        angle = -90.0;
    }*/
    srcRect = {0, 0, 32, 32};
    destRect.w = 16;
    destRect.h = 16;
    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);
}

// Setter por si un moficador cambia el rumbo del projectil
void Projectile::setDirection(float dx, float dy)
{
    dirX = dx;
    dirY = dy;
    angle = std::atan2(dirY, dirX) * (180.0 / M_PI);
}

void Projectile::setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    color = {r, g, b, a};
}

void Projectile::addModifier(std::unique_ptr<ProjectileModifier> mod)
{
    mod->onInit(this); // Aplica efectos iniciales al añadirlo
    modifiers.push_back(std::move(mod));
}

void Projectile::update(double deltaTime, Grid *grid)
{
    // 1. Ejecutar modificadores (pueden cambiar dirX, dirY, etc.)
    for (auto &mod : modifiers)
    {
        mod->update(this, deltaTime);
    }

    // 2. Mover la posición lógica
    x += dirX * speed * deltaTime;
    y += dirY * speed * deltaTime;

    // 3. Actualizar el rectángulo de dibujo
    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);

    distanceTraveled += speed * deltaTime;
    // Quitamos el angle += 360 porque ahora el ángulo se ajusta a la dirección real de vuelo.
}

void Projectile::render(SDL_Renderer *renderer, const SDL_Rect &camera)
{
    SDL_Rect renderRect = {
        destRect.x - camera.x,
        destRect.y - camera.y,
        destRect.w,
        destRect.h};

    // Aplicar el color y luego dibujar
    SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(texture, color.a);

    TextureManager::DrawRotated(texture, srcRect, renderRect, angle, renderer);

    // Restaurar el color a blanco por si la textura se usa en otro lado
    SDL_SetTextureColorMod(texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(texture, 255);
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

bool Projectile::isAlive() const{
    return alive;
}

void Projectile::kill() {
    this->alive = false;
    
    // Opcional: Lo mandamos al "limbo" para que no interfiera 
    // visualmente ni en colisiones fantasma
    this->x = -1000.0f;
    this->y = -1000.0f;
    
    // Si tuviera efectos de partículas, aquí es donde 
    // dispararía la explosión antes de morir.
}

// Projectile.cpp
void Projectile::setAlive(float x, float y, float targetX, float targetY, float spd, float range, float dmg, SDL_Texture* tex) {
    float deltaX = targetX - x;
    float deltaY = targetY - y;
    float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

    if (distance > 0)
    {
        dirX = deltaX / distance;
        dirY = deltaY / distance;
        angle = std::atan2(deltaY, deltaX) * (180.0 / M_PI);
    }
    else
    {
        dirX = 0;
        dirY = -1;
        angle = -90.0;
    }
    this->x = x;
    this->y = y;
    this->speed = spd;
    this->maxRange = range;
    this->damage = dmg;
    this->texture = tex;
    
    this->distanceTraveled = 0.0f;
    this->alive = true;
}