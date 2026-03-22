#pragma once
#include "green-ninja/ProjectileModifier.hpp"
#include "green-ninja/Projectile.hpp" // Necesitan conocer los métodos del proyectil
#include <cmath>

// --- Modificador de Prueba: Lágrima de Sangre ---
class BloodTearModifier : public ProjectileModifier
{
public:
    void onInit(Projectile *p) override;
    // No necesitamos sobrescribir update() porque solo cambia cosas al crearse
};

// --- Modificador de Prueba: Trayectoria en Zig-Zag ---
class WiggleModifier : public ProjectileModifier
{
private:
    float timeAlive = 0.0f;
    float baseX, baseY;

public:
    void onInit(Projectile *p) override;
    void update(Projectile *p, double deltaTime) override;
};

// ¡Aquí irás añadiendo más declaraciones en el futuro! (Homing, Explosive, Piercing...)