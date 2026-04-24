#include "green-ninja/Grid.hpp"
#include "green-ninja/TextureManager.hpp"

// ¡IMPORTANTE! Definición de la variable estática para que el compilador reserve memoria
std::map<TileType, SDL_Texture *> Grid::tileTextures;

Grid::Grid(int r, int c) : rows(r), cols(c)
{
    // Redimensionamos la matriz: 'rows' vectores de tamaño 'cols'
    mapData.resize(rows, std::vector<Tile>(cols));
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            mapData[i][j].setType(TileType::FLOOR1);

            // Ejemplo: Bordes
            if (i == 0 || i == rows - 1 || j == 0 || j == cols - 1)
                mapData[i][j].setType(TileType::WALL);
        }
    }
}

Grid::~Grid()
{
}

void Grid::loadTextures(SDL_Renderer *renderer, std::map<std::string, std::string> &spritePaths)
{
    // TIPE OF FLOORS
    tileTextures[TileType::FLOOR1] = TextureManager::LoadTexture(spritePaths["floor1"].c_str(), renderer);
    tileTextures[TileType::FLOOR2] = TextureManager::LoadTexture(spritePaths["floor2"].c_str(), renderer);
    tileTextures[TileType::FLOOR3] = TextureManager::LoadTexture(spritePaths["floor3"].c_str(), renderer);
    // ELEMENTS OF THE ROOM
    tileTextures[TileType::WALL] = TextureManager::LoadTexture(spritePaths["wall"].c_str(), renderer);
    tileTextures[TileType::FOSA] = TextureManager::LoadTexture(spritePaths["fosa"].c_str(), renderer);
    tileTextures[TileType::ROCK1] = TextureManager::LoadTexture(spritePaths["rock1"].c_str(), renderer);
    tileTextures[TileType::ROCK2] = TextureManager::LoadTexture(spritePaths["rock2"].c_str(), renderer);
    // OPEN DOORS
    tileTextures[TileType::RIGHTDOOR] = TextureManager::LoadTexture(spritePaths["rightDoor"].c_str(), renderer);
    tileTextures[TileType::LEFTDOOR] = TextureManager::LoadTexture(spritePaths["leftDoor"].c_str(), renderer);
    tileTextures[TileType::TOPDOOR] = TextureManager::LoadTexture(spritePaths["topDoor"].c_str(), renderer);
    tileTextures[TileType::BOTTOMDOOR] = TextureManager::LoadTexture(spritePaths["bottomDoor"].c_str(), renderer);
    // CLOSED DOORS
    tileTextures[TileType::CLOSEDRIGHTDOOR] = TextureManager::LoadTexture(spritePaths["rightDoorClosed"].c_str(), renderer);
    tileTextures[TileType::CLOSEDLEFTDOOR] = TextureManager::LoadTexture(spritePaths["leftDoorClosed"].c_str(), renderer);
    tileTextures[TileType::CLOSEDTOPDOOR] = TextureManager::LoadTexture(spritePaths["topDoorClosed"].c_str(), renderer);
    tileTextures[TileType::CLOSEDBOTTOMDOOR] = TextureManager::LoadTexture(spritePaths["bottomDoorClosed"].c_str(), renderer);
}

void Grid::DrawGrid(SDL_Renderer *renderer, const SDL_Rect &camera)
{
    SDL_Rect src = {0, 0, 32, 32};
    SDL_Rect dest;
    dest.w = dest.h = TILE_SIZE;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            dest.x = (j * TILE_SIZE) - camera.x;
            dest.y = (i * TILE_SIZE) - camera.y;

            // Culling (si no se ve en pantalla, no lo dibujamos)
            if (dest.x + dest.w < 0 || dest.x > camera.w ||
                dest.y + dest.h < 0 || dest.y > camera.h)
            {
                continue;
            }

            TileType currentType = mapData[i][j].getType();

            // Buscamos si el estado actual de la casilla tiene una textura asignada en el mapa.
            if (tileTextures.find(currentType) != tileTextures.end())
            {
                if (currentType == TileType::ROCK1 || currentType == TileType::ROCK2)
                    TextureManager::Draw(tileTextures[Lvlfloor], src, dest, renderer);
                TextureManager::Draw(tileTextures[currentType], src, dest, renderer);
            }
        }
    }
}

Tile &Grid::GetTileAt(int x, int y)
{
    int col = x;
    int row = y;

    // Protección con el tamaño dinámico
    if (row < 0)
        row = 0;
    if (row >= rows)
        row = rows - 1;
    if (col < 0)
        col = 0;
    if (col >= cols)
        col = cols - 1;

    return mapData[row][col];
}

int Grid::getRows() const { return rows; }

int Grid::getCols() const { return cols; }

int Grid::getTileSize() { return TILE_SIZE; }

bool Grid::isWalkable(int x, int y)
{
    return mapData[y][x].isWalkable();
}