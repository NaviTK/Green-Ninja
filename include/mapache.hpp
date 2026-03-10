#pragma once
#include "Enemy.hpp"
#include <SDL2/SDL.h> // Necesario para SDL_Texture

class Mapache : public Enemy
{
private:
    float health;
    double attackCooldown;
    double timeSinceLastAttack;
    float targetX;
    float targetY;
    float moveSpeed;
    // --- VARIABLES DE ANIMACIÓN ---
    int frameWidth;
    int frameHeight;
    float animTimer = 0.0f;
    const float TIME_PER_FRAME = 0.15f; // Velocidad de la animación
    int currentFrame = 0;
    int numFrames = 4; // Suponiendo que tu mapache tiene 4 frames por animación

    enum Direction
    {
        SOUTH,
        NORTH,
        WEST,
        EAST
    } oriented;
    bool isMoving;

public:
    // --- CONSTRUCTOR ACTUALIZADO (Añadido SDL_Texture*) ---
    Mapache(MapCoord startPos, Player *playerTarget, Room *hab, SDL_Texture *texturaMapache);
    virtual ~Mapache() = default;

    // --- FUNCIONES ---
    // (Hemos borrado LoadTexture de aquí)
    void animationLogic(double deltaTime);

    void update(double deltaTime, Grid *grid) override;
    void attack() override;

    void takeDamage(float amount) override
    {
        health -= amount;
        // Opcional: Aquí podrías añadir un parpadeo en rojo para el mapache
    }
    bool isDead() const override
    {
        return health <= 0.0f;
    }
};