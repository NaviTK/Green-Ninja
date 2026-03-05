#include "Player.hpp"
#include "Grid.hpp"
#include "TextureManager.hpp"
#include <iostream>
#include <string>

// 1. DEFINICIÓN OBLIGATORIA de la variable estática
SDL_Texture *Player::playerTexture = nullptr;

void Player::InicializarStadisticas(float posx, float posy)
{
    lastShot = 0;           // contador para volver a disparar
    oriented = SOUTH;       // Inicialmente mirando hacia abajo
    x = posx;               // posicion horizontal del jugador
    y = posy;               // posicion vertical del jugador
    health = 100;           // vida del jugador
    dmg = 10.0f;            // daño base del jugador
    moveSpeed = 200.0f;     // velocidad de movimiento
    attackSpeed = 5.0f;     // velocidad de ataque (projectiles por segundo)
    projectileSpeed = 5.0f; // velocidad de los proyectiles disparados
    projectileSize = 1.0f;  // tamaño de los proyectiles (1.0f es tamaño normal)
    range = 20.0f;          // alcance de los proyectiles (cuanto mayor, más lejos llegan)
}

void Player::setShootCallback(std::function<void(float, float, int, int, float, float, float, float)> callback)
{
    onShootCallback = callback;
}

// 2. FUNCIÓN DE CARGA: Se debe llamar ANTES de crear al jugador o dentro del constructor de forma segura
void Player::LoadTexture(const std::string &path, SDL_Renderer *renderer)
{
    if (playerTexture == nullptr)
    {
        playerTexture = TextureManager::LoadTexture(path.c_str(), renderer);
    }
}

// 3. CONSTRUCTOR CORREGIDO
Player::Player(float x, float y, SDL_Renderer *renderer)
    : Entity(x, y, playerTexture), m_renderer(renderer)
{
    InicializarStadisticas(x, y);
    // ASIGNACIÓN MANUAL: Como Entity ya se inicializó con nullptr, se lo asignamos ahora
    this->texture = playerTexture;

    timerUp = timerDown = timerLeft = timerRight = 0;
    frameWidth = 16;
    frameHeight = 16;
    srcRect = {0, 0, frameWidth, frameHeight};
    float escala = 3.0f;
    destRect.w = static_cast<int>(frameWidth * escala);
    destRect.h = static_cast<int>(frameHeight * escala);
}

void Player::movementLogic(double deltaTime, Grid *grid)
{
    const Uint8 *keystates = SDL_GetKeyboardState(NULL);

    float oldX = x;
    float oldY = y;
    float moveX = 0;
    float moveY = 0;

    // 2. Calculamos el movimiento Vertical
    if (keystates[SDL_SCANCODE_UP])
    {
        if (!isAttacking)
            oriented = NORTH; // Solo gira si NO está atacando
        moveY -= moveSpeed * deltaTime;
    }
    else if (keystates[SDL_SCANCODE_DOWN])
    {
        if (!isAttacking)
            oriented = SOUTH;
        moveY += moveSpeed * deltaTime;
    }

    // 3. Calculamos el movimiento Horizontal
    if (keystates[SDL_SCANCODE_LEFT])
    {
        if (!isAttacking)
            oriented = WEST;
        moveX -= moveSpeed * deltaTime;
    }
    else if (keystates[SDL_SCANCODE_RIGHT])
    {
        if (!isAttacking)
            oriented = EAST;
        moveX += moveSpeed * deltaTime;
    }

    // 4. Intentamos movernos solo en el Eje X
    if (moveX != 0)
    {
        x += moveX;
        if (checkCollision(x, y, grid))
        {
            x = oldX; // Si chocamos en X, deshacemos el movimiento horizontal
        }
    }

    // 5. Intentamos movernos solo en el Eje Y
    if (moveY != 0)
    {
        y += moveY;
        if (checkCollision(x, y, grid))
        {
            y = oldY; // Si chocamos en Y, deshacemos el movimiento vertical
        }
    }
}

void Player::animationLogic(double deltaTime)
{
    // 1. ESTADO DE ATAQUE (Tiene prioridad absoluta)
    if (isAttacking)
    {
        attackTimer -= deltaTime; // Descontamos el tiempo
        if (attackTimer <= 0.0f)
        {
            isAttacking = false;
            currentState = IDLE1; // Se acabó el tiempo, volvemos a reposo
        }
        else
        {
            currentState = ATTACKING; // Mantenemos el frame de ataque
        }
    }
    // 2. ESTADO DE MOVIMIENTO (Solo si no está atacando)
    else
    {
        const Uint8 *keystates = SDL_GetKeyboardState(NULL);
        bool isMoving = keystates[SDL_SCANCODE_RIGHT] || keystates[SDL_SCANCODE_LEFT] ||
                        keystates[SDL_SCANCODE_UP] || keystates[SDL_SCANCODE_DOWN];

        if (isMoving)
        {
            animTimer += deltaTime; // Acumulamos tiempo
            if (animTimer >= TIME_PER_FRAME)
            {
                animTimer = 0.0f;                             // Reseteamos el contador
                currentState = animationFrames[currentState]; // Siguiente frame
            }
        }
        else
        {
            currentState = IDLE1; // Si no pulsamos nada, frame de reposo
            animTimer = 0.0f;
        }
    }

    // 3. ACTUALIZAR EL RECTÁNGULO DE ORIGEN (Sprite Sheet)
    srcRect.y = frameHeight * currentState;

    switch (oriented)
    {
    case SOUTH:
        srcRect.x = frameWidth * 0;
        break;
    case NORTH:
        srcRect.x = frameWidth * 1;
        break;
    case WEST:
        srcRect.x = frameWidth * 2;
        break;
    case EAST:
        srcRect.x = frameWidth * 3;
        break;
    }
}

void Player::shootLogic()
{
    if (!m_renderer)
        return; // Programación defensiva

    int winMouseX, winMouseY; // Coordenadas físicas de la ventana
    uint32_t currentTick = SDL_GetTicks();
    Uint32 mouseState = SDL_GetMouseState(&winMouseX, &winMouseY);

    // Comprobamos si disparamos ANTES de hacer cálculos caros
    if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && (currentTick - lastShot > 1000 / attackSpeed))
    {
        // --- MAGIA DE CONVERSIÓN DE COORDENADAS ---
        float logicalMouseX, logicalMouseY;
        SDL_RenderWindowToLogical(m_renderer, winMouseX, winMouseY, &logicalMouseX, &logicalMouseY);

        int mouseX = static_cast<int>(logicalMouseX);
        int mouseY = static_cast<int>(logicalMouseY);
        // -----------------------------------------

        // Sacamos el centro exacto del jugador en el mundo (usando x e y reales, no destRect)
        float playerCenterX = x + (destRect.w / 2.0f);
        float playerCenterY = y + (destRect.h / 2.0f);

        // AHORA SÍ: Comparamos peras con peras (coordenadas lógicas)
        float dx = mouseX - playerCenterX;
        float dy = mouseY - playerCenterY;

        // Decidimos hacia dónde mira el jugador al disparar
        if (std::abs(dx) > std::abs(dy))
        {
            oriented = (dx > 0) ? EAST : WEST;
        }
        else
        {
            oriented = (dy > 0) ? SOUTH : NORTH;
        }

        // Le pasamos al proyectil las coordenadas del ratón ya traducidas
        shootProjectile(mouseX, mouseY);

        lastShot = currentTick;
        isAttacking = true;
        attackTimer = 0.5f; // Medio segundo de animación de ataque
        currentState = ATTACKING;
    }
}

void Player::shootProjectile(int mouseX, int mouseY)
{
    if (onShootCallback)
    {
        // Empezamos calculando el centro del jugador en el mundo real
        float spawnX = x + (destRect.w / 2.0f);
        float spawnY = y + (destRect.h / 2.0f);

        // ¿A cuántos píxeles del centro está la mano de tu personaje?
        float forwardOffset = 25.0f; // Distancia hacia adelante
        float sideOffset = 15.0f;    // Distancia hacia el lado (mano)

        // Desplazamos el punto de spawn dependiendo de hacia dónde esté mirando
        switch (oriented)
        {
        case NORTH:
            spawnY -= forwardOffset;
            spawnX -= sideOffset;
            break;
        case SOUTH:
            spawnY += forwardOffset;
            spawnX -= sideOffset; // Depende de qué mano quieras usar, puedes sumar o restar
            break;
        case EAST:
            spawnX += forwardOffset;
            spawnY += sideOffset;
            break;
        case WEST:
            spawnX -= forwardOffset;
            spawnY += sideOffset;
            break;
        }

        // ¡Disparamos desde la nueva posición de la mano!
        onShootCallback(spawnX, spawnY, mouseX, mouseY, projectileSpeed, projectileSize, range, dmg);
    }
    else
    {
        std::cout << "¡Disparo detectado, pero el callback no esta conectado a Game!" << std::endl;
    }
}

void Player::update(double deltaTime, Grid *grid)
{
    shootLogic();
    movementLogic(deltaTime, grid);
    animationLogic(deltaTime);

    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);
}

bool Player::checkCollision(float nextX, float nextY, Grid *grid)
{
    // --- MAGIA DE LA HITBOX ---
    float margenX = 12.0f;     // Estrecha los "hombros" 12 píxeles por cada lado
    float margenTop = 20.0f;   // Baja la cabeza
    float margenBottom = 2.0f; // Sube los pies

    float hitX = nextX + margenX;
    float hitY = nextY + margenTop;
    float hitW = destRect.w - (margenX * 2);
    float hitH = destRect.h - (margenTop + margenBottom);

    // Calculamos las casillas basándonos en la Hitbox reducida
    int leftTile = static_cast<int>(hitX) / grid->getTileSize();
    int rightTile = static_cast<int>(hitX + hitW - 1) / grid->getTileSize();
    int topTile = static_cast<int>(hitY) / grid->getTileSize();
    int bottomTile = static_cast<int>(hitY + hitH - 1) / grid->getTileSize();

    // 1. Comprobar límites del mapa
    if (leftTile < 0 || rightTile >= grid->getCols() ||
        topTile < 0 || bottomTile >= grid->getRows())
    {
        return true;
    }

    // 2. Comprobar si tocamos PARED
    for (int i = topTile; i <= bottomTile; i++)
    {
        for (int j = leftTile; j <= rightTile; j++)
        {
            if (grid->GetTileAt(j, i).hasType(TileType::WALL))
            {
                return true;
            }
        }
    }

    return false;
}