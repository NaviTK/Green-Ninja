#pragma once
#include "Entity.hpp"
#include <string>
#include <map>
#include <functional> // <-- NUEVO: Necesario para std::function (Callbacks)

class Grid;

class Player : public Entity
{
private:
    // Atributos privados
    int timerUp, timerDown, timerLeft, timerRight;
    int frameWidth;
    int frameHeight;
    // Control de animaciones
    float animTimer = 0.0f;
    const float TIME_PER_FRAME = 0.15f; // Cambia de frame cada 0.15 segundos

    // Control del estado de ataque
    float attackTimer = 0.0f;
    bool isAttacking = false;
    // renderer
    SDL_Renderer *m_renderer;
    enum Direction
    {
        NORTH,
        SOUTH,
        WEST,
        EAST
    } oriented;

    // Definimos los estados usando desplazamientos de bits
    enum EntityState
    {
        IDLE1 = 0,
        IDLE2 = 1,
        IDLE3 = 2,
        IDLE4 = 3,
        ATTACKING = 4,
        RUNNING = 5,
        DYING = 6
    };

    std::map<EntityState, EntityState> animationFrames = {
        {EntityState::IDLE1, EntityState::IDLE2},
        {EntityState::IDLE2, EntityState::IDLE3},
        {EntityState::IDLE3, EntityState::IDLE4},
        {EntityState::IDLE4, EntityState::IDLE1},
        {EntityState::RUNNING, EntityState::RUNNING},
        {EntityState::ATTACKING, EntityState::ATTACKING},
        {EntityState::DYING, EntityState::DYING}};

    EntityState currentState = EntityState::IDLE1;

    // stadisticas
    float health;
    float dmg;
    float moveSpeed;
    float attackSpeed;
    float projectileSpeed;
    float projectileSize;
    float range;

    // projectiles
    uint32_t lastShot;

    // textura
    static SDL_Texture *playerTexture;

    // NUEVO: El "botón" de disparo.
    // Guardará la función que Game.cpp le pase para crear el proyectil.
    // Parámetros: (posicionJugadorX, posicionJugadorY, mouseX, mouseY, projectileSpeed, projectileSize, range, damage)
    std::function<void(float, float, int, int, float, float, float, float)> onShootCallback;

public:
    // Constructor
    Player(float x, float y, SDL_Renderer *renderer);

    // Destructor (opcional si no añades punteros nuevos)
    ~Player() override = default;

    // NUEVO: Método para que el Gestor (Game) enchufe la función de crear proyectiles
    void setShootCallback(std::function<void(float, float, int, int, float, float, float, float)> callback);

    // Métodos sobreescritos
    void update(double deltaTime, Grid *grid) override;
    // Usamos el render de la clase Entity por defecto

    // Método para inicializar las estadísticas del jugador
    void InicializarStadisticas(float x, float y); // Cambiado a float para mayor precisión

    // game logic
    void takeDamage(float amount);
    void gainHealth(float amount);
    void shootProjectile(int mouseX, int mouseY);
    void movementLogic(double deltaTime, Grid *grid);
    void animationLogic(double deltaTime);
    void shootLogic();
    bool checkCollision(float nextX, float nextY, Grid *grid);
    // setter/getters de posicion
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }
    int getX() { return x; };
    int getY() { return y; };
    // gestion de texturas
    static void LoadTexture(const std::string &path, SDL_Renderer *renderer);
    static void ReleaseResources();
};