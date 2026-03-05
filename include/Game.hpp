#ifndef GAME_HPP
#define GAME_HPP

#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include "Player.hpp"
#include "Projectile.hpp"
#include "Room.hpp"
#include <map>

typedef std::vector<std::vector<int>> Mapa; // matriz donde cada elemento es un ID que usando el map de rooms podemos acceder a la Room concreta.

class Game
{
public:
    Game();
    ~Game();
    bool init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen);
    void handleEvents();
    void update(double deltaTime);
    void render();
    void clean();
    void LoadAllTextures(SDL_Renderer *renderer);
    bool running() { return isRunning; }
    void checkRoomTransition();
    bool weAreInADoor(int col, int row, Grid *grid);
    static const int projectileLimit = 500; // Límite de proyectiles en pantalla para evitar saturación
private:
    void Inicialize(int width, int height);
    bool isRunning;
    // window and renderer
    SDL_Window *window;
    int windowWidht, windowHeight;
    SDL_Renderer *renderer;
    // camera
    SDL_Rect camara;
    // Entidades
    Player *player;
    std::vector<Projectile *> projectiles;
    // std::vector<Zombie *> zombies;
    // Textures
    std::map<std::string, std::string> spritePaths;
    SDL_Texture *projectileTexture = nullptr;
    // World
    std::map<int, Room *> rooms; // Mapa de habitaciones (id -> Room*)
    Room *currentRoom;           // La habitación actual en la que nos encontramos (inicialmente será la baseRoom)
    Room *baseRoom;              // La habitación raíz del mapa, desde la que se generarán todas las demás (START)
    int depth = 5;               // limite de profundidad de creacion de habitaciones(distancia a la que esta el BOSS)
    Mapa mapa;
};

#endif