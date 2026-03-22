#pragma once
#include <SDL2/SDL.h>

class Projectile; // Declaración adelantada

class ProjectileModifier
{
public:
    virtual ~ProjectileModifier() = default;

    // Se ejecuta cada frame antes o después del movimiento básico
    virtual void update(Projectile *p, double deltaTime) {}

    // Se ejecuta al momento de crearse para aplicar efectos iniciales (como tamaño)
    virtual void onInit(Projectile *p) {}
};