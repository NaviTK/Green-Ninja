#pragma once
#include "green-ninja/Entity.hpp"
#include "green-ninja/Player.hpp"
#include "green-ninja/Grid.hpp"
#include "green-ninja/Room.hpp"
#include "green-ninja/Projectile.hpp"
#include <vector>

typedef std::pair<int, int> MapCoord;

// Si no tienes este enum definido en Entity.hpp, descoméntalo aquí:
enum Direction
{
    SOUTH,
    NORTH,
    WEST,
    EAST
};

class Enemy : public Entity
{
protected:
    Player *target;
    Room *habitacion;

    // --- ESTADÍSTICAS BASE ---
    float health;
    int damage;
    int aggroRange;
    int attackRange;

    // --- FÍSICAS Y MOVIMIENTO ---
    float knockbackVX;
    float knockbackVY;
    float targetX;
    float targetY;
    float moveSpeed;
    bool isMoving;
    MapCoord currentPos;
    int oriented; // Orientación actual (NORTH, SOUTH, EAST, WEST)

    // --- ANIMACIÓN Y DAÑO ---
    float dmgTimer;
    bool isTakingDmg;
    int TakingDmgOffset;

    int currentFrame;
    int numFrames;
    double animTimer;
    double TIME_PER_FRAME;
    int frameWidth;
    int frameHeight;

    // Funciones de IA
    std::vector<MapCoord> calcularCaminoAStar(Grid *grid, MapCoord inicio, MapCoord destino);
    int calcularHeuristica(MapCoord a, MapCoord b);
    bool pos_ok(MapCoord nodo, const Grid &sala);

    // --- NUEVAS FUNCIONES COMPARTIDAS PARA TODOS LOS ENEMIGOS ---
    void aplicarRetroceso(Projectile *proj);
    void manejarRetroceso(double deltaTime, Grid *grid);
    void actualizarDireccionMirada(float oldX, float oldY);
    void animationLogic(double deltaTime);

public:
    Enemy(MapCoord startPos, Player *playerTarget, Room *hab);
    virtual ~Enemy() = default;

    // Estas dos siguen siendo = 0 porque cada enemigo ataca/piensa distinto
    virtual void update(double deltaTime, Grid *grid) override = 0;
    virtual void attack() = 0;

    // Estas ahora son genéricas y están implementadas en Enemy.cpp
    virtual void takeDamage(float amount);
    virtual void takeDamage(Projectile *proj);
    virtual bool isDead() const;
    virtual float getDamage() const;
    virtual MapCoord getCoord() const;
};