#include "green-ninja/ProjectileModifiers.hpp"

// --- Implementación Lágrima de Sangre ---
void BloodTearModifier::onInit(Projectile *p)
{
    p->setColor(255, 0, 0); // Rojo
    p->modifyDamage(1.5f);  // +50% daño
    p->setSize(1.5f);       // 50% más grande
}

// --- Implementación Zig-Zag ---
void WiggleModifier::onInit(Projectile *p)
{
    baseX = p->getDirX();
    baseY = p->getDirY();
}

void WiggleModifier::update(Projectile *p, double deltaTime)
{
    timeAlive += deltaTime;
    // Calcula una onda perpendicular a la dirección de movimiento
    float wave = std::sin(timeAlive * 15.0f) * 0.5f;

    // Aplica el desvío alterando la dirección actual
    p->setDirection(baseX + (baseY * wave), baseY - (baseX * wave));
}