#include "green-ninja/TextureManager.hpp"
#include <SDL2/SDL_image.h> // Necesario para cargar PNGs

SDL_Texture *TextureManager::LoadTexture(const char *fileName, SDL_Renderer *ren)
{
    // IMG_LoadTexture detecta automáticamente si es PNG, JPG o BMP
    SDL_Texture *tex = IMG_LoadTexture(ren, fileName);

    if (tex == nullptr)
    {
        // Usamos IMG_GetError() para saber exactamente por qué falló el PNG
        SDL_Log("No se pudo cargar el asset: %s | Error: %s", fileName, IMG_GetError());
    }

    return tex;
}

void TextureManager::Draw(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, SDL_Renderer *ren)
{
    // SDL_RenderCopy se encarga de "recortar" el src y "pegarlo" en el dest
    SDL_RenderCopy(ren, tex, &src, &dest);
}
void TextureManager::DrawRotated(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest, double angle, SDL_Renderer *ren)
{
    // Envolvemos la función avanzada de SDL dentro de nuestro Manager
    SDL_RenderCopyEx(ren, tex, &src, &dest, angle, NULL, SDL_FLIP_NONE);
}