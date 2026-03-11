#pragma once
#include "green-ninja/Enemy.hpp"
#include <vector>

class Mapache : public Enemy
{
private:
    // Variables exclusivas del Mapache (su cooldown de ataque)
    double attackCooldown;
    double timeSinceLastAttack;

    // Solo nos quedamos con la función que es el "cerebro" específico del Mapache
    void actualizarComportamientoNormal(double deltaTime, Grid *grid);

public:
    Mapache(MapCoord startPos, Player *playerTarget, Room *hab, SDL_Texture *texturaMapache);
    virtual ~Mapache() = default;

    // Obligados a implementar desde la clase padre
    void update(double deltaTime, Grid *grid) override;
    void attack() override;
};