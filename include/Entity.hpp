#pragma once
#include <SDL2/SDL.h>
#include "TextureManager.hpp"
#include <vector>
#include "Grid.hpp"

class Entity
{
protected:
    float x, y;
    SDL_Rect srcRect;
    SDL_Rect destRect;
    SDL_Texture *texture;

public:
    Entity(float x, float y, SDL_Texture *tex)
        : x(x), y(y), texture(tex)
    {
        // Inicializamos los rects para evitar basura en memoria
        srcRect = {0, 0, 0, 0};
        destRect = {0, 0, 0, 0};
    }

    virtual ~Entity() {}

    virtual void update(double deltaTime, Grid *grid) = 0;

    virtual void render(SDL_Renderer *ren, const SDL_Rect &camera)
    {
        // Actualizamos la posición del rectángulo de destino antes de dibujar
        destRect.x = static_cast<int>(x - camera.x);
        destRect.y = static_cast<int>(y - camera.y);

        // ORDEN (Textura, srcRect, destRect, Renderer)
        TextureManager::Draw(texture, srcRect, destRect, ren);
    }

    float getX() const { return x; }
    float getY() const { return y; }
    SDL_Rect getDestRect() const { return destRect; }
};