#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <map>
#include "green-ninja/Tile.hpp"

// Definimos el alias para la matriz de Tiles
using TileMatrix = std::vector<std::vector<Tile>>;

class Grid
{
public:
    Grid(int rows, int cols);
    ~Grid();
    static void loadTextures(SDL_Renderer *renderer, std::map<std::string, std::string> &paths);
    void DrawGrid(SDL_Renderer *renderer, const SDL_Rect &camera);

    // getters/setters
    int getRows() const;
    int getCols() const;
    static int getTileSize();
    Tile &GetTileAt(int x, int y);
    bool isWalkable(int x, int y);

private:
    int rows;
    int cols;
    static const int TILE_SIZE = 48;

    // Nuestra matriz dinámica
    TileMatrix mapData;

    // ACTUALIZADO: Usamos el nuevo enum class directamente
    static std::map<TileType, SDL_Texture *> tileTextures;
};