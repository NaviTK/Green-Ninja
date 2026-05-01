#pragma once
#include <SDL2/SDL.h>
#include "green-ninja/TextureManager.hpp"
#include <vector>
#include "green-ninja/Grid.hpp"

class Entity
{
protected:
    float x, y;
    SDL_Rect srcRect;
    SDL_Rect destRect;
    SDL_Texture *texture;

    // --- NUEVAS VARIABLES DE HITBOX ---
    // Offset (desplazamiento) relativo a x e y
    float hitBoxOffsetX, hitBoxOffsetY;
    // Tamaño real de la caja de colisión
    float hitBoxWidth, hitBoxHeight;

public:
    Entity(float x, float y, SDL_Texture *tex)
        : x(x), y(y), texture(tex)
    {
        srcRect = {0, 0, 0, 0};
        destRect = {0, 0, 0, 0};

        // Por defecto, inicializamos la hitbox a 0.
        // Las clases hijas (Player, Enemy, etc.) deberán reescribir
        // estos valores en sus propios constructores según su sprite.
        hitBoxOffsetX = 0.0f;
        hitBoxOffsetY = 0.0f;
        hitBoxWidth = 0.0f;
        hitBoxHeight = 0.0f;
    }

    virtual ~Entity() {}

    virtual void update(double deltaTime, Grid *grid) = 0;

    virtual void render(SDL_Renderer *ren, const SDL_Rect &camera)
    {
        destRect.x = static_cast<int>(x - camera.x);
        destRect.y = static_cast<int>(y - camera.y);

        TextureManager::Draw(texture, srcRect, destRect, ren);
    }

    // Devuelve la Hitbox calculada en coordenadas exactas del MUNDO
    SDL_FRect getCollider() const 
    {
        return { 
            x + hitBoxOffsetX, 
            y + hitBoxOffsetY, 
            hitBoxWidth, 
            hitBoxHeight 
        };
    }

    float getX() const { return x; }
    float getY() const { return y; }
    SDL_Rect getDestRect() const { return destRect; }
};