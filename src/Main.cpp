#include "green-ninja/Game.hpp"

Game *game = nullptr;

int main(int argc, char *argv[])
{
    game = new Game();
    double deltaTime = 0;
    game->init("Elemental Grid Tactics", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, false);
    Uint64 lastTime = SDL_GetPerformanceCounter();
    while (game->running())
    {
        Uint64 currentTime = SDL_GetPerformanceCounter();
        // Calculamos la diferencia en segundos
        deltaTime = (double)((currentTime - lastTime) * 1000 / (double)SDL_GetPerformanceFrequency()) * 0.001;
        lastTime = currentTime;
        game->handleEvents();
        game->update(deltaTime);
        game->render();
    }

    game->clean();
    delete game;

    return 0;
}