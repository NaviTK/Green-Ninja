#pragma once
#include <vector>
#include <functional>
#include <SDL2/SDL.h>
#include "green-ninja/Entity.hpp"
#include "green-ninja/Projectile.hpp"

class SpatialManager {
private:
    // Los 4 Mares y el Grand Line (Fronteras)
    std::vector<Entity*> northWestBlue;
    std::vector<Entity*> northEastBlue;
    std::vector<Entity*> southWestBlue;
    std::vector<Entity*> southEastBlue;
    std::vector<Entity*> grandLine;

    float midX;
    float midY;

public:
    SpatialManager(float mapWidth, float mapHeight);
    void updateMapDimensions(float newWidth, float newHeight);
    void clear();
    void insert(Entity* entity);
    void checkAllCollisions();

    float getMidX() const { return midX; }
    float getMidY() const { return midY; }

    template<typename Func>
    void checkCollisionsFor(Projectile* p, Func onCollision) {
        SDL_Rect rect = p->getCollider();
        
        for (Entity* e : grandLine) onCollision(e);

        if (rect.x < midX) {
            if (rect.y < midY) { for (Entity* e : northWestBlue) onCollision(e); }
            else               { for (Entity* e : southWestBlue) onCollision(e); }
        } else {
            if (rect.y < midY) { for (Entity* e : northEastBlue) onCollision(e); }
            else               { for (Entity* e : southEastBlue) onCollision(e); }
        }
    }

    // --- NUEVA FUNCIÓN PARA DIBUJAR LOS COLORES DE DEBUG ---
    void renderDebugEnemies(SDL_Renderer* renderer, const SDL_Rect& camera);
};