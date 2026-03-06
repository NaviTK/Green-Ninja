#pragma once
#include "Grid.hpp"
#include <list>
#include <map>     // Necesario para std::map
#include <utility> // Necesario para std::pair
#include "RoomType.hpp"
#include <cstdlib>   // Para rand() y srand()
#include <ctime>     // Para time()
#include <algorithm> // Para std::shuffle
#include <random>    // Para generación de números aleatorios moderna

typedef std::pair<int, int> MapCoord;
typedef std::vector<std::vector<int>> Mapa;

enum class direction
{
    NORTH,
    SOUTH,
    EAST,
    WEST
};

class Room
{
private:
    // campos staticos
    static int nextId;                                              // Para asignar IDs únicos a cada habitación
    static const std::map<RoomType, std::pair<int, int>> roomSizes; // Mapa estático que asigna un tamaño específico a cada tipo de habitación (en filas y columnas)

    // propiedades
    int id;                         // id unico de la habitacion
    Grid *roomGrid;                 // grid unico de la habitacion
    MapCoord coordenadasQuadricula; // posicion en el mapa de quadricula

    // Conexiones con otras habitaciones (nullptr si hay pared y no hay salida)
    Room *topRoom;
    Room *bottomRoom;
    Room *leftRoom;
    Room *rightRoom;

    RoomType type;  // El tipo de habitación (START, NORMAL, BIG, etc.)
    bool isCleared; // Para saber si ya matamos a todos y podemos abrir las puertas

    std::list<std::pair<int, int>> getDoors(); // Devuelve la lista de coordenadas de las puertas de esta sala (para que Game sepa dónde dibujarlas y dónde detectar colisiones)

    Room *expandirMapa(int currentDepth, int maxDepth, std::map<int, Room *> &roomsMap, Mapa &mapa, direction direccion, MapCoord lastCoord);
    // funcion para conseguir siguiente posicion en la quadricula durante la recursion
    static MapCoord AñadirAlExpandir(MapCoord posicionActual, direction direccionActual);
    // comprueba si la posicion de la cuadricula es correcta
    bool pos_ok(MapCoord posicionCuadricula, Mapa &cuadricula);

public:
    static direction getOpposite(direction dir);
    void setNeighborByDirection(direction dir, Room *neighbor);
    // -----------------------------------------------------------------
    // CREADORA 1: La que usa Game (Genera el nivel y devuelve el START)
    // -----------------------------------------------------------------
    static Room *GenerarNivel(int maxDepth, std::map<int, Room *> &roomsMap, Mapa &mapa);

    // -----------------------------------------------------------------
    // CREADORA 2: El constructor real (Inicializa una sala individual)
    // Fíjate que por defecto crea una sala NORMAL. El tamaño lo sacará del mapa 'roomSizes'
    // -----------------------------------------------------------------
    Room(RoomType roomType, MapCoord mapPosition);
    ~Room();

    // Utilidad estática para resetear el contador de IDs al cambiar de piso
    static void resetContadorIDs() { nextId = 0; }

    // Le decimos a la habitación quiénes son sus vecinos de golpe
    void setNeighbors(Room *top, Room *bottom, Room *left, Room *right);

    // Rompe las paredes del Grid para crear las puertas
    void buildDoors();

    void update(double deltaTime);
    void render(SDL_Renderer *renderer, const SDL_Rect &camera);

    // Getters útiles
    Grid *getGrid() const { return roomGrid; }
    int getId() const { return id; }
    RoomType getType() const { return type; }
    bool IsCleared() const { return isCleared; }

    // Getters de vecinos para cuando el jugador cruce una puerta
    Room *getTopRoom() const { return topRoom; }
    Room *getBottomRoom() const { return bottomRoom; }
    Room *getLeftRoom() const { return leftRoom; }
    Room *getRightRoom() const { return rightRoom; }

    // Setters individuales para conectar habitaciones paso a paso
    void setTopRoom(Room *room) { topRoom = room; }
    void setBottomRoom(Room *room) { bottomRoom = room; }
    void setLeftRoom(Room *room) { leftRoom = room; }
    void setRightRoom(Room *room) { rightRoom = room; }
};