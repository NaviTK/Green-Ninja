#include "green-ninja/Room.hpp"
#include <list>
#include <iostream>

// --- INICIALIZACIÓN DE VARIABLES ESTÁTICAS ---
int Room::nextId = 0;
Mapa Room::cuadriculaMapa;

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

Room::Room(RoomType roomType, MapCoord coordenadasEnLaCuadricula)
{
    isCleared = (roomType == RoomType::START || roomType == RoomType::GOLDEN);
    id = nextId;
    nextId++;
    type = roomType;
    topRoom = bottomRoom = leftRoom = rightRoom = nullptr;

    int rows = roomSizes.at(roomType).first;
    int cols = roomSizes.at(roomType).second;
    roomGrid = new Grid(rows, cols);
    coordenadasQuadricula = coordenadasEnLaCuadricula;
}

Room::~Room()
{
    if (roomGrid)
        delete roomGrid;
}

// =========================================================================
// GENERACIÓN PROCEDURAL
// =========================================================================

Room *Room::GenerarNivel(int maxDepth, std::map<int, Room *> &roomsMap, Mapa &cuadricula)
{
    cuadriculaMapa = cuadricula;
    // reseteamos los ids para que empiezen en 1(opcional)
    resetContadorIDs();
    // coordenadas de la sala inicial = {centroSala.x,centroSala.y}
    MapCoord startCoord = {maxDepth, maxDepth};
    // creamos sala inicial con coordenadas de la casilla central
    Room *startRoom = new Room(RoomType::START, startCoord);

    // asignamos posicion de la habitacion en la quadricula
    cuadricula[startCoord.first][startCoord.second] = startRoom->getId();
    // asignamos ID -> room*
    roomsMap[startRoom->getId()] = startRoom;
    // generamos hacia todas las direcciones con probabilidad: 30%
    std::vector<direction> directions(4);
    directions = {direction::WEST, direction::EAST, direction::NORTH, direction::SOUTH};
    for (auto direccionActual : directions)
    {
        Room *salaHija = startRoom->expandirMapa(1, maxDepth, roomsMap, cuadricula, direccionActual, AñadirAlExpandir(startCoord, direccionActual));

        if (salaHija)
        {
            startRoom->setNeighborByDirection(direccionActual, salaHija);
            salaHija->setNeighborByDirection(Room::getOpposite(direccionActual), startRoom);
        }
    }
    // construimos las puertas de TODAS las salas generadas, no hace falta generarlas durante la recursion
    for (auto const &[roomId, room] : roomsMap)
    {
        room->buildDoors();
    }
    // devolvemos la sala origen
    return startRoom;
}

Room *Room::expandirMapa(int currentDepth, int maxDepth, std::map<int, Room *> &roomsMap, Mapa &cuadricula, direction direccion, MapCoord coordenadasCuadricula)
{
    // condiciones de final de recursion
    // nos salimos del limite o de la cuadricula
    if (currentDepth >= maxDepth)
        return nullptr;
    if (!pos_ok(coordenadasCuadricula, cuadricula))
        return nullptr;
    if (cuadricula[coordenadasCuadricula.first][coordenadasCuadricula.second] != -1)
        return nullptr;
    // creamos sala nueva
    Room *nuevaSala = new Room(RoomType::NORMAL, coordenadasCuadricula);
    cuadricula[coordenadasCuadricula.first][coordenadasCuadricula.second] = nuevaSala->getId();
    roomsMap[nuevaSala->getId()] = nuevaSala;

    std::vector<direction> directions(4);
    directions = {direction::WEST, direction::EAST, direction::NORTH, direction::SOUTH};
    for (auto direccionActual : directions)
    {
        if (direccionActual != Room::getOpposite(direccion))
        {
            Room *salaHija = nuevaSala->expandirMapa(currentDepth + 1, maxDepth, roomsMap, cuadricula, direccionActual, AñadirAlExpandir(coordenadasCuadricula, direccionActual));

            if (salaHija)
            {
                nuevaSala->setNeighborByDirection(direccionActual, salaHija);
                salaHija->setNeighborByDirection(Room::getOpposite(direccionActual), nuevaSala);
            }
        }
    }

    return nuevaSala;
}

MapCoord Room::AñadirAlExpandir(MapCoord posicionActual, direction direccionActual)
{
    switch (direccionActual)
    {
    case direction::NORTH:
        return {posicionActual.first, posicionActual.second - 1};
    case direction::SOUTH:
        return {posicionActual.first, posicionActual.second + 1};
    case direction::EAST:
        return {posicionActual.first + 1, posicionActual.second};
    case direction::WEST:
        return {posicionActual.first - 1, posicionActual.second};
    default:
        return posicionActual;
    }
}

bool Room::pos_ok(MapCoord coordenadasCuadricula, Mapa &cuadricula)
{
    if (coordenadasCuadricula.first < 0 || coordenadasCuadricula.first >= cuadricula.size())
        return false;
    if (coordenadasCuadricula.second < 0 || coordenadasCuadricula.second >= cuadricula[0].size())
        return false;

    return true;
}

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