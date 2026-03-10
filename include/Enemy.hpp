#pragma once
#include "Entity.hpp"
#include "Player.hpp" // Necesitamos saber dónde está el jugador
#include "Grid.hpp"
#include "Room.hpp"
#include <vector>

typedef std::pair<int, int> MapCoord;

class Enemy : public Entity
{
protected:
    Player *target;   // Puntero al jugador para saber a quién perseguir
    int aggroRange;   // Distancia a la que detecta al jugador
    int attackRange;  // Distancia a la que puede atacar
    Room *habitacion; // puntero a la habitacion donde estamos
    int damage;

    // Función protegida para que la usen las clases hijas
    // Devuelve el camino de casillas hacia el objetivo
    std::vector<MapCoord> calcularCaminoAStar(Grid *grid, MapCoord inicio, MapCoord destino);
    int calcularHeuristica(MapCoord a, MapCoord b);
    bool pos_ok(MapCoord nodo, const Grid &sala);

public:
    Enemy(MapCoord startPos, Player *playerTarget, Room *hab);
    virtual ~Enemy() = default;

    // Obligamos a los hijos a implementar cómo se mueven y atacan
    virtual void update(double deltaTime, Grid *grid) override = 0;
    virtual void attack() = 0;
    virtual void takeDamage(float amount) = 0;
    virtual bool isDead() const = 0;
};