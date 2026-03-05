#include "Room.hpp"
#include <list>
#include <iostream>

// --- INICIALIZACIÓN DE VARIABLES ESTÁTICAS ---
int Room::nextId = 0;

const std::map<RoomType, std::pair<int, int>> Room::roomSizes = {
    {RoomType::NONE, {0, 0}},
    {RoomType::START, {9, 15}},
    {RoomType::NORMAL, {9, 15}},
    {RoomType::BIG, {18, 30}},
    {RoomType::GOLDEN, {9, 15}},
    {RoomType::BOSS, {9, 15}}};

// Función estática para obtener la dirección opuesta
direction Room::getOpposite(direction dir)
{
    switch (dir)
    {
    case direction::NORTH:
        return direction::SOUTH;
    case direction::SOUTH:
        return direction::NORTH;
    case direction::EAST:
        return direction::WEST;
    case direction::WEST:
        return direction::EAST;
    default:
        return direction::NORTH;
    }
}

// Método de ayuda para conectar vecinos dinámicamente
void Room::setNeighborByDirection(direction dir, Room *neighbor)
{
    if (!neighbor)
        return;
    switch (dir)
    {
    case direction::NORTH:
        topRoom = neighbor;
        break;
    case direction::SOUTH:
        bottomRoom = neighbor;
        break;
    case direction::EAST:
        rightRoom = neighbor;
        break;
    case direction::WEST:
        leftRoom = neighbor;
        break;
    }
}

// =========================================================================
// CONSTRUCTOR Y DESTRUCTOR
// =========================================================================

Room::Room(RoomType roomType, MapCoord mapPosition)
{
    isCleared = (roomType == RoomType::START || roomType == RoomType::GOLDEN);
    id = nextId++;
    type = roomType;
    topRoom = bottomRoom = leftRoom = rightRoom = nullptr;

    int rows = roomSizes.at(roomType).first;
    int cols = roomSizes.at(roomType).second;
    roomGrid = new Grid(rows, cols);
    mapCoord = mapPosition;
}

Room::~Room()
{
    if (roomGrid)
        delete roomGrid;
}

// =========================================================================
// GENERACIÓN PROCEDURAL
// =========================================================================

Room *Room::GenerarNivel(int maxDepth, std::map<int, Room *> &roomsMap, Mapa &mapa)
{
    resetContadorIDs();

    MapCoord startCoord = {maxDepth, maxDepth};
    Room *startRoom = new Room(RoomType::START, startCoord);

    mapa[startCoord.first][startCoord.second] = startRoom->getId();
    roomsMap[startRoom->getId()] = startRoom;

    // Elegimos una dirección de expansión (ej. WEST)
    direction dirExpansion = direction::WEST;
    Room *salaHija = startRoom->expandirMapa(0, maxDepth, roomsMap, mapa, dirExpansion, startCoord);

    // Conectamos usando la lógica de opuestos
    if (salaHija)
    {
        startRoom->setNeighborByDirection(dirExpansion, salaHija);
        salaHija->setNeighborByDirection(Room::getOpposite(dirExpansion), startRoom);
    }

    startRoom->buildDoors();
    return startRoom;
}

Room *Room::expandirMapa(int currentDepth, int maxDepth, std::map<int, Room *> &roomsMap, Mapa &mapa, direction direccion, MapCoord lastCoord)
{
    if (currentDepth >= maxDepth)
        return nullptr;

    MapCoord newCoord = lastCoord;
    switch (direccion)
    {
    case direction::NORTH:
        newCoord.second -= 1;
        break;
    case direction::SOUTH:
        newCoord.second += 1;
        break;
    case direction::EAST:
        newCoord.first += 1;
        break;
    case direction::WEST:
        newCoord.first -= 1;
        break;
    }

    Room *nuevaSala = new Room(RoomType::NORMAL, newCoord);
    roomsMap[nuevaSala->getId()] = nuevaSala;
    mapa[newCoord.first][newCoord.second] = nuevaSala->getId();

    // Seguimos expandiendo en la misma dirección
    Room *salaSiguiente = nuevaSala->expandirMapa(currentDepth + 1, maxDepth, roomsMap, mapa, direccion, newCoord);

    // Conexión dinámica entre esta sala y la siguiente
    if (salaSiguiente)
    {
        nuevaSala->setNeighborByDirection(direccion, salaSiguiente);
        salaSiguiente->setNeighborByDirection(Room::getOpposite(direccion), nuevaSala);
    }

    // Nota: El padre de 'nuevaSala' se conectará a ella en la llamada recursiva anterior o en GenerarNivel
    nuevaSala->buildDoors();
    return nuevaSala;
}
// =========================================================================
// CONEXIONES Y PUERTAS
// =========================================================================

void Room::setNeighbors(Room *top, Room *bottom, Room *left, Room *right)
{
    if (top != nullptr)
        topRoom = top;
    if (bottom != nullptr)
        bottomRoom = bottom;
    if (left != nullptr)
        leftRoom = left;
    if (right != nullptr)
        rightRoom = right;
}

std::list<std::pair<int, int>> Room::getDoors()
{
    std::list<std::pair<int, int>> doors;

    if (roomGrid == nullptr)
        return doors;

    int rows = roomGrid->getRows();
    int cols = roomGrid->getCols();

    if (type == RoomType::BIG)
    {
        // Añadimos comprobaciones para que las salas grandes no abran puertas a la nada
        if (topRoom != nullptr)
        {
            doors.push_back({cols / 4, 0});
            doors.push_back({cols / 4 + cols / 2, 0});
        }
        if (rightRoom != nullptr)
        {
            doors.push_back({cols - 1, rows / 4});
            doors.push_back({cols - 1, rows / 4 + rows / 2});
        }
        if (bottomRoom != nullptr)
        {
            doors.push_back({cols / 4 + cols / 2, rows - 1});
            doors.push_back({cols / 4, rows - 1});
        }
        if (leftRoom != nullptr)
        {
            doors.push_back({0, rows / 4 + rows / 2});
            doors.push_back({0, rows / 4});
        }
    }
    else
    {
        if (topRoom != nullptr)
            doors.push_back({cols / 2, 0});
        if (rightRoom != nullptr)
            doors.push_back({cols - 1, rows / 2});
        if (bottomRoom != nullptr)
            doors.push_back({cols / 2, rows - 1});
        if (leftRoom != nullptr)
            doors.push_back({0, rows / 2});
    }

    return doors;
}

void Room::buildDoors()
{
    if (roomGrid == nullptr)
        return;

    int rows = roomGrid->getRows();
    int cols = roomGrid->getCols();
    std::list<std::pair<int, int>> doors = getDoors();

    for (auto &door : doors)
    {
        int x = door.first;
        int y = door.second;
        Tile &tile = roomGrid->GetTileAt(x, y);

        if (y == 0)
            tile.setType(TileType::TOPDOOR);
        if (y == rows - 1)
            tile.setType(TileType::BOTTOMDOOR);
        if (x == 0)
            tile.setType(TileType::LEFTDOOR);
        if (x == cols - 1)
            tile.setType(TileType::RIGHTDOOR);
    }
}

// =========================================================================
// BUCLE PRINCIPAL (UPDATE & RENDER)
// =========================================================================

void Room::update(double deltaTime)
{
}

void Room::render(SDL_Renderer *renderer, const SDL_Rect &camera)
{
    if (roomGrid)
    {
        roomGrid->DrawGrid(renderer, camera);
    }
}