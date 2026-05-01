#include "green-ninja/SpatialManager.hpp"

SpatialManager::SpatialManager(float mapWidth, float mapHeight) {
    updateMapDimensions(mapWidth, mapHeight);
}

void SpatialManager::updateMapDimensions(float newWidth, float newHeight) {
    this->midX = newWidth / 2.0f;
    this->midY = newHeight / 2.0f;
    
    // Es buena idea limpiar las listas si cambiamos de dimensiones 
    // para evitar que queden punteros de la sala anterior.
    clear();
}

void SpatialManager::clear() {
    northWestBlue.clear();
    northEastBlue.clear();
    southWestBlue.clear();
    southEastBlue.clear();
    grandLine.clear();
}

void SpatialManager::insert(Entity* e) {
    // IMPORTANTE: Asegúrate de que este Rect NO tenga la cámara restada. 
    // Debe ser la posición real en el mapa.
    SDL_Rect r = e->getDestRect(); 

    int enemyRight = r.x + r.w;
    int enemyBottom = r.y + r.h;

    // --- LÓGICA DE GRAND LINE (AMARILLO) ---
    // Si el lado izquierdo está a la izquierda de midX Y el lado derecho está a la derecha...
    // O si el lado superior está arriba de midY Y el lado inferior está abajo...
    bool crossingVertical = (r.x < midX && enemyRight > midX);
    bool crossingHorizontal = (r.y < midY && enemyBottom > midY);

    if (crossingVertical || crossingHorizontal) {
        grandLine.push_back(e);
    } 
    // --- LÓGICA DE MARES (COLORES) ---
    else if (r.x + r.w/2 < midX) {
        if (r.y + r.h/2 < midY) northWestBlue.push_back(e);
        else southWestBlue.push_back(e);
    } else {
        if (r.y + r.h/2 < midY) northEastBlue.push_back(e);
        else southEastBlue.push_back(e);
    }
}

void SpatialManager::renderDebugEnemies(SDL_Renderer* renderer, const SDL_Rect& camera) {
        // Lambda interna para no repetir el código de dibujo por cada vector
        auto drawList = [&](std::vector<Entity*>& list, Uint8 r, Uint8 g, Uint8 b) {
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            for (Entity* entity : list) {
                SDL_Rect rect = entity->getDestRect();
                
                // Si getDestRect() devuelve coordenadas del MUNDO, 
                // hay que restarle la cámara para verlas bien en pantalla:
                rect.x -= camera.x;
                rect.y -= camera.y;

                SDL_RenderDrawRect(renderer, &rect);
            }
        };

        // Asignamos un color único a cada zona
        drawList(northWestBlue, 0, 255, 0);   // Verde
        drawList(northEastBlue, 0, 255, 255); // Cyan
        drawList(southWestBlue, 255, 0, 255); // Magenta
        drawList(southEastBlue, 0, 0, 255);   // Azul
        drawList(grandLine, 255, 255, 0);     // Amarillo (Zona de conflicto)
    }