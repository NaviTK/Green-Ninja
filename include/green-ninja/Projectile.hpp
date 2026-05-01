#pragma once
#include "green-ninja/Entity.hpp"
#include "green-ninja/ProjectileModifier.hpp" // NUEVO
#include <SDL.h>
#include <vector>
#include <memory>

class Grid;

class Projectile : public Entity
{
private:
    bool alive;
    float dirX, dirY;
    float speed;
    float distanceTraveled;
    float maxRange;
    float knockback = 300.0f;
    double angle;
    float damage = 10;

    // --- NUEVO: Sistema de modificadores y color ---
    std::vector<std::unique_ptr<ProjectileModifier>> modifiers;
    SDL_Color color = {255, 255, 255, 255}; // Blanco por defecto (sin tinte)

public:
    Projectile();

    // --- NUEVOS MÉTODOS PARA MODIFICADORES ---
    void addModifier(std::unique_ptr<ProjectileModifier> mod);

    // Getters y Setters para que los modificadores alteren el proyectil
    float getDirX() const { return dirX; }
    float getDirY() const { return dirY; }
    void setDirection(float dx, float dy);
    void setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void modifySpeed(float multiplier) { speed *= multiplier; }
    void modifyDamage(float multiplier) { damage *= multiplier; }

    void setSize(float sizeMultiplier);

    void update(double deltaTime, Grid *grid) override;
    void render(SDL_Renderer *renderer, const SDL_Rect &camera) override;

    bool isAlive() const;
    void kill();
    void setAlive(float x, float y, float tX, float tY, float spd, float range, float dmg, SDL_Texture* tex);
    bool isExpired() const;
    SDL_Rect getCollider() const;
    float getDamage() const;
    float getKnockback() const;
    SDL_Rect getDestRect() const { return destRect; }
};