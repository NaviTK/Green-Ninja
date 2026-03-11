#ifndef TEXTUREMANAGER_HPP
#define TEXTUREMANAGER_HPP

#include <SDL2/SDL.h>

class TextureManager
{
public:
    // Carga una imagen y la devuelve como textura
    static SDL_Texture *LoadTexture(const char *fileName, SDL_Renderer *ren);

    // Función para dibujar una textura en una posición específica
    static void Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, SDL_Renderer *ren);

    // Función para dibujar una textura rotada (usada por Projectile)
    static void DrawRotated(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, double angle, SDL_Renderer *ren);
};

#endif