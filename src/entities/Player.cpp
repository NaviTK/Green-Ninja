#include "green-ninja/Player.hpp"
#include "green-ninja/Grid.hpp"
#include "green-ninja/Projectile.hpp"          // <-- NUEVO: Necesario para conocer la clase Projectile
#include "green-ninja/ProjectileModifiers.hpp" // <-- NUEVO: Necesario para instanciar los modificadores
#include <iostream>
#include <cmath>     // Necesario para std::abs y std::fmod
#include <algorithm> // Necesario para std::find

void Player::InicializarStadisticas(float posx, float posy)
{
    knockbackVX = 0.0f;
    knockbackVY = 0.0f;
    isTakingDmg = false;
    dmgTimer = 0.0f;
    invulnerabilityTime = 2.0f;
    TakingDmgOffset = 64;   // Offset en píxeles en la hoja de sprites
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

// --- MODIFICADO: Ahora devuelve un Projectile* ---
void Player::setShootCallback(std::function<Projectile *(float, float, int, int, float, float, float, float)> callback)
{
    onShootCallback = callback;
}

// --- NUEVO: Añadir un efecto a la lista del jugador ---
void Player::addEffect(ProjectileEffect effect)
{
    activeEffects.push_back(effect);
}

// --- NUEVO: Aplicar los efectos al proyectil ---
void Player::applyEffectsToProjectile(Projectile *p)
{
    if (!p)
        return; // Por seguridad

    for (ProjectileEffect effect : activeEffects)
    {
        switch (effect)
        {
        case ProjectileEffect::BLOOD_TEAR:
            p->addModifier(std::make_unique<BloodTearModifier>());
            break;
        case ProjectileEffect::WIGGLE_WORM:
            p->addModifier(std::make_unique<WiggleModifier>());
            break;
        }
    }
}

// --- CONSTRUCTOR ---
Player::Player(float x, float y, SDL_Renderer *renderer, SDL_Texture *texturaJugador)
    : Entity(x, y, texturaJugador), m_renderer(renderer)
{
    InicializarStadisticas(x, y);
    // Asignamos la textura a la variable de la clase padre Entity explícitamente por seguridad
    this->texture = texturaJugador;

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

    // 1. APLICAR INERCIA DEL KNOCKBACK
    if (std::abs(knockbackVX) > 0.1f || std::abs(knockbackVY) > 0.1f)
    {
        moveX += knockbackVX * deltaTime;
        moveY += knockbackVY * deltaTime;

        float friction = 10.0f;
        knockbackVX -= knockbackVX * friction * deltaTime;
        knockbackVY -= knockbackVY * friction * deltaTime;
    }

    // 2. Movimiento Vertical (Usando las variables dinámicas)
    if (keystates[this->keyUp])
    {
        if (!isAttacking) oriented = NORTH;
        moveY -= moveSpeed * deltaTime;
    }
    else if (keystates[this->keyDown])
    {
        if (!isAttacking) oriented = SOUTH;
        moveY += moveSpeed * deltaTime;
    }

    // 3. Movimiento Horizontal (Usando las variables dinámicas)
    if (keystates[this->keyLeft])
    {
        if (!isAttacking) oriented = WEST;
        moveX -= moveSpeed * deltaTime;
    }
    else if (keystates[this->keyRight])
    {
        if (!isAttacking) oriented = EAST;
        moveX += moveSpeed * deltaTime;
    }

    // 4. Eje X: Aplicar y colisionar
    if (moveX != 0)
    {
        x += moveX;
        if (checkCollision(x, y, grid))
        {
            x = oldX;
            knockbackVX = 0.0f;
        }
    }

    // 5. Eje Y: Aplicar y colisionar
    if (moveY != 0)
    {
        y += moveY;
        if (checkCollision(x, y, grid))
        {
            y = oldY;
            knockbackVY = 0.0f;
        }
    }
}

void Player::animationLogic(double deltaTime)
{
    // --- GESTIÓN DE INVULNERABILIDAD Y PARPADEO ---
    if (dmgTimer > 0.0f)
    {
        dmgTimer -= deltaTime;

        // Efecto retro de parpadeo: Alterna verdadero/falso cada 0.1 segundos
        if (std::fmod(dmgTimer, 0.2f) > 0.1f)
        {
            isTakingDmg = true;
        }
        else
        {
            isTakingDmg = false;
        }

        // Cuando se acaba el tiempo, nos aseguramos de apagar el efecto visual
        if (dmgTimer <= 0.0f)
        {
            isTakingDmg = false;
            dmgTimer = 0.0f;
        }
    }

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
        
        // Mucho más limpio: Comprobamos directamente las variables dinámicas
        bool isMoving = keystates[this->keyRight] || keystates[this->keyLeft] ||
                        keystates[this->keyUp] || keystates[this->keyDown];

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

    // --- APLICAR OFFSET DE DAÑO SÓLO DURANTE LOS "PICOS" DEL PARPADEO ---
    if (isTakingDmg)
    {
        srcRect.x += TakingDmgOffset;
    }
}

void Player::shootLogic()
{
    if (!m_renderer)
        return; // Programación defensiva
    // para testear modificadores de projectil
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
    if (currentKeyStates[SDL_SCANCODE_M])
    {
        addEffect(ProjectileEffect::BLOOD_TEAR);
        addEffect(ProjectileEffect::WIGGLE_WORM);
    }
    // fin test de modificadores de projectil
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

        // --- CAMBIO PRINCIPAL: Guardamos el puntero, lo creamos y aplicamos efectos ---
        Projectile *newProj = onShootCallback(spawnX, spawnY, mouseX, mouseY, projectileSpeed, projectileSize, range, dmg);

        applyEffectsToProjectile(newProj);
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
            if (grid->GetTileAt(j, i).hasType(TileType::WALL) || grid->GetTileAt(j, i).hasType(TileType::ROCK1) ||
                grid->GetTileAt(j, i).hasType(TileType::ROCK2) || grid->GetTileAt(j, i).hasType(TileType::FOSA))
                return true;
        }
    }

    return false;
}

// --- FUNCIÓN PARA RECIBIR DAÑO ---
void Player::takeDamage(float amount, MapCoord source)
{
    // COMPROBAMOS DIRECTAMENTE EL TEMPORIZADOR PARA LOS I-FRAMES
    if (dmgTimer > 0.0f)
        return; // Si el temporizador sigue vivo, ignoramos el golpe

    health -= amount;
    dmgTimer = 1.2f; // --- TIENES 1.2 SEGUNDOS DE INVULNERABILIDAD ---

    std::cout << "Jugador recibe " << amount << " de daño! Vida restante: " << health << std::endl;

    // --- CÁLCULO DEL KNOCKBACK ---
    int tileSize = 48; // Asumiendo tile 16 x escala 3

    // Centro del jugador
    float playerCenterX = x + (destRect.w / 2.0f);
    float playerCenterY = y + (destRect.h / 2.0f);

    // Centro del enemigo
    float enemyCenterX = (source.first * tileSize) + (tileSize / 2.0f);
    float enemyCenterY = (source.second * tileSize) + (tileSize / 2.0f);

    // Vector de dirección: Destino (Jugador) - Origen (Enemigo)
    float dirX = playerCenterX - enemyCenterX;
    float dirY = playerCenterY - enemyCenterY;

    // Calculamos la distancia (la magnitud del vector)
    float distance = std::sqrt(dirX * dirX + dirY * dirY);

    // Normalizamos el vector para que mida exactamente 1
    if (distance > 0.0f)
    {
        dirX /= distance;
        dirY /= distance;
    }
    else
    {
        // Por seguridad: si por un milagro están en el pixel exacto, empujamos a la derecha
        dirX = 1.0f;
        dirY = 0.0f;
    }

    // 2. Aplicamos la fuerza de empuje
    float knockbackForce = 500.0f;

    // Guardamos la velocidad de retroceso en el jugador
    knockbackVX = dirX * knockbackForce;
    knockbackVY = dirY * knockbackForce;
}

void Player::removeEffect(ProjectileEffect effect)
{
    // Buscamos el efecto en nuestra lista de efectos activos
    auto it = std::find(activeEffects.begin(), activeEffects.end(), effect);

    // Si lo encontramos, lo borramos
    if (it != activeEffects.end())
    {
        activeEffects.erase(it);
        std::cout << "Efecto eliminado del jugador." << std::endl;
    }
}

// --- NUEVO: Limpiar todos los efectos ---
void Player::clearAllEffects()
{
    activeEffects.clear();
    std::cout << "Todos los efectos han sido eliminados." << std::endl;
}

void Player::setkeybinds(const std::vector<std::pair<std::string,SDL_Scancode>>& bindings) {
    for (const auto& bind : bindings) {
        
        // bind.first es el texto (ej: "up"), bind.second es el SDL_Scancode
        if (bind.first == "up") {
            this->keyUp = bind.second;
        } 
        else if (bind.first == "down") {
            this->keyDown = bind.second;
        } 
        else if (bind.first == "left") {
            this->keyLeft = bind.second;
        } 
        else if (bind.first == "right") {
            this->keyRight = bind.second;
        }
        // En el futuro puedes añadir aquí: else if (bind.first == "shoot") ...
    }
}