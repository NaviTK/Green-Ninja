#pragma once
#include "green-ninja/Entity.hpp"
#include <SDL.h>

class Grid; // Declaración adelantada para evitar dependencias circulares

class Projectile : public Entity
{
private:
    float dirX, dirY;         // Vector de dirección normalizado (Hacia dónde va)
    float speed;              // Velocidad del proyectil (píxeles por segundo)
    float distanceTraveled;   // Cuánto ha viajado (para compararlo con maxRange)
    float maxRange;           // Alcance máximo antes de desaparecer
    float knockback = 300.0f; // knockBack del projectil
    // --- NUEVOS ATRIBUTOS RECOMENDADOS ---
    double angle; // Ángulo en grados para rotar la textura (que la flecha mire a donde viaja)
    float damage; // Cuánto daño hará al impactar

public:
    SDL_Rect getDestRect() const { return destRect; }
    // Constructor (He añadido el daño por defecto al final)
    Projectile(float x, float y, float targetX, float targetY, SDL_Texture *tex, float spd, float rng, int dmg = 10);

    // Método para cambiar el tamaño del proyectil (usado por Player al disparar)
    void setSize(float sizeMultiplier);

    // Métodos heredados de Entity
    void update(double deltaTime, Grid *grid) override;

    // Sobrescribimos el render para poder dibujarlo rotado (usando el 'angle')
    void render(SDL_Renderer *renderer, const SDL_Rect &camera) override;

    // Métodos propios de lógica y colisión
    bool isExpired() const;
    SDL_Rect getCollider() const; // Para comprobar choques en Game.cpp
    float getDamage() const;      // Para restarle vida al enemigo
    float getKnockback() const;
};