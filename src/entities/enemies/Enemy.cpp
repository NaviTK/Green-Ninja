#include "green-ninja/Enemy.hpp"
#include <queue>
#include <map>
#include <algorithm>
#include <cmath>

// =========================================================================
// CONSTRUCTOR
// =========================================================================
Enemy::Enemy(MapCoord startPos, Player *playerTarget, Room *hab)
    : Entity(startPos.first * 48, startPos.second * 48, nullptr) // Asumiendo que tus tiles miden 64. Ajusta esto a tu tamaño real.
{
    habitacion = hab;
    target = playerTarget;
    // Estos valores por defecto los sobreescribirá cada hijo (ej: Mapache) en su propio constructor
    aggroRange = 10;
    attackRange = 1;
    damage = 1;
}

bool Enemy::pos_ok(MapCoord nodo, const Grid &sala)
{
    int rows = sala.getRows();
    int cols = sala.getCols();
    if (nodo.first < 0 || nodo.first >= cols)
        return false;
    if (nodo.second < 0 || nodo.second >= rows)
        return false;

    return true;
}

int Enemy::calcularHeuristica(MapCoord a, MapCoord b)
{
    int dx = std::abs(a.first - b.first);
    int dy = std::abs(a.second - b.second);

    // Moverse en recto cuesta 10, en diagonal cuesta 14
    int min_d = std::min(dx, dy);
    int max_d = std::max(dx, dy);
    return 14 * min_d + 10 * (max_d - min_d);
}

// =========================================================================
// ALGORITMO DE BÚSQUEDA DE RUTAS (BFS) Incompleto
// =========================================================================
/*std::vector<MapCoord> Enemy::calcularCaminoBFS(Grid *grid, MapCoord inicio, MapCoord destino)
{
    std::map<MapCoord, MapCoord> come_from;                              // mapa con <donde_estoy/de_donde_vine>
    std::vector<MapCoord> camino;                                        // camino final que construiremos al encontrar al player
    std::queue<MapCoord> nodosPorVisitar;                                // Lista de Nodos por niveles
    Grid *sala = habitacion->getGrid();                                  // grid de la habitacion (con info de cada Tile)
    Mapa visited(sala->getRows(), std::vector<int>(sala->getCols(), 0)); // mapa con los nodos ya visitados
    nodosPorVisitar.push(inicio);                                        // Añadimos Inicio como primer Nodo
    bool finished = false;                                               // condicion de salida del bucle, podriamos hacer break tambien
    while (!nodosPorVisitar.empty() && !finished)
    { // iteramos mientras queden posiciones que visitar y no hayamos llegado a player
        MapCoord nodoActual = nodosPorVisitar.front();
        nodosPorVisitar.pop();
        if (pos_ok(nodoActual, *sala) && !visited[nodoActual.second][nodoActual.first])
        { // si ya hemos pasado por aqui nos salimos
            visited[nodoActual.second][nodoActual.first] = 1;
            if (nodoActual == destino)
            { // construimos el camino con el map come_from y salimos del bucle
                MapCoord nodoAux = nodoActual;
                while (nodoAux != inicio)
                {
                    camino.push_back(nodoAux);
                    nodoAux = come_from.at(nodoAux);
                }
                finished = true;
            }
            else if (sala->GetTileAt(nodoActual.first, nodoActual.second).isWalkable())
            {
            }
        }
    }
    return camino;
}*/
struct AStarNode
{
    int f_score;    // El coste total (g + h)
    MapCoord coord; // La coordenada X, Y

    // Sobrecargamos el operador > para que la priority_queue ponga el menor f_score arriba
    bool operator>(const AStarNode &other) const
    {
        return f_score > other.f_score;
    }
};

std::vector<MapCoord> Enemy::calcularCaminoAStar(Grid *grid, MapCoord inicio, MapCoord destino)
{
    std::map<MapCoord, MapCoord> come_from;
    std::vector<MapCoord> camino;
    Grid *sala = habitacion->getGrid();

    int rows = sala->getRows();
    int cols = sala->getCols();

    // Matriz de nodos visitados (cerrados)
    Mapa visited(rows, std::vector<int>(cols, 0));

    // Matriz de Costes Reales (g_score). Inicializado a "Infinito" (INT_MAX)
    std::vector<std::vector<int>> g_score(rows, std::vector<int>(cols, INT_MAX));
    g_score[inicio.second][inicio.first] = 0; // El coste de donde empezamos es 0

    // Cola de prioridad que ordena automáticamente los AStarNode
    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> nodosPorVisitar;

    // Añadimos el inicio a la cola
    nodosPorVisitar.push({calcularHeuristica(inicio, destino), inicio});

    bool finished = false;

    while (!nodosPorVisitar.empty() && !finished)
    {
        MapCoord nodoActual = nodosPorVisitar.top().coord; // En priority_queue usamos .top() en vez de .front()
        nodosPorVisitar.pop();

        // Si ya evaluamos esta casilla con un camino mejor antes, la saltamos
        if (visited[nodoActual.second][nodoActual.first])
            continue;

        // La marcamos como visitada
        visited[nodoActual.second][nodoActual.first] = 1;

        // 1. CONDICIÓN DE VICTORIA
        if (nodoActual == destino)
        {
            MapCoord nodoAux = nodoActual;
            while (nodoAux != inicio)
            {
                camino.push_back(nodoAux);
                nodoAux = come_from.at(nodoAux);
            }
            std::reverse(camino.begin(), camino.end());

            finished = true;
        }
        // 2. EXPLORAR VECINOS
        else if (sala->GetTileAt(nodoActual.first, nodoActual.second).isWalkable())
        {
            // Las 8 direcciones: {X, Y, Coste (10 recto, 14 diagonal)}
            struct DirCosto
            {
                int dx, dy, costo;
            };
            std::vector<DirCosto> direcciones = {
                {0, -1, 10}, {0, 1, 10}, {-1, 0, 10}, {1, 0, 10}, // 4 Rectas
                {-1, -1, 14},
                {1, -1, 14},
                {-1, 1, 14},
                {1, 1, 14} // 4 Diagonales
            };

            for (const auto &dir : direcciones)
            {
                MapCoord vecino = {nodoActual.first + dir.dx, nodoActual.second + dir.dy};

                // Si está fuera de los límites o es pared, lo ignoramos
                if (!pos_ok(vecino, *sala) || !sala->GetTileAt(vecino.first, vecino.second).isWalkable())
                    continue;

                // --- NUEVO: PREVENIR CORTES DE ESQUINA EN DIAGONALES ---
                if (dir.dx != 0 && dir.dy != 0)
                {
                    bool adyacenteX_ok = sala->GetTileAt(nodoActual.first + dir.dx, nodoActual.second).isWalkable();
                    bool adyacenteY_ok = sala->GetTileAt(nodoActual.first, nodoActual.second + dir.dy).isWalkable();

                    if (!adyacenteX_ok || !adyacenteY_ok)
                    {
                        continue;
                    }
                }
                // --------------------------------------------------------

                // Calculamos cuánto costaría llegar a este vecino pasando por el nodoActual
                int tentative_g = g_score[nodoActual.second][nodoActual.first] + dir.costo;

                // Si descubrimos un camino más corto (o es la primera vez que lo vemos)
                if (tentative_g < g_score[vecino.second][vecino.first])
                {
                    come_from[vecino] = nodoActual;                     // Rastro de migas
                    g_score[vecino.second][vecino.first] = tentative_g; // Actualizamos su récord de coste real

                    int f_score = tentative_g + calcularHeuristica(vecino, destino); // Coste real + Heurística
                    nodosPorVisitar.push({f_score, vecino});                         // Lo metemos en la cola
                }
            }
        }
    }
    return camino;
}

// =========================================================================
// SISTEMA DE DAÑO Y VIDA
// =========================================================================

void Enemy::takeDamage(float amount)
{
    health -= amount;
    dmgTimer = 0.25f;
}

void Enemy::takeDamage(Projectile *proj)
{
    health -= proj->getDamage();
    dmgTimer = 0.25f;
    aplicarRetroceso(proj);
}

bool Enemy::isDead() const
{
    return health <= 0.0f;
}

float Enemy::getDamage() const
{
    return damage; // Ojo, en tu Mapache la variable se llamaba "dmg", aquí en Enemy la llamaste "damage"
}

MapCoord Enemy::getCoord() const
{
    return currentPos;
}

// =========================================================================
// FÍSICAS Y COLISIONES UNIVERSALES
// =========================================================================

void Enemy::aplicarRetroceso(Projectile *proj)
{
    float force = proj->getKnockback();

    SDL_Rect projRect = proj->getDestRect();
    float projCenterX = projRect.x + projRect.w / 2.0f;
    float projCenterY = projRect.y + projRect.h / 2.0f;

    float myCenterX = destRect.x + destRect.w / 2.0f;
    float myCenterY = destRect.y + destRect.h / 2.0f;

    float dirX = myCenterX - projCenterX;
    float dirY = myCenterY - projCenterY;
    float dist = std::hypot(dirX, dirY);

    if (dist > 0.0f)
    {
        dirX /= dist;
        dirY /= dist;
    }
    else
    {
        dirX = 1.0f;
        dirY = 0.0f;
    }

    knockbackVX = dirX * force;
    knockbackVY = dirY * force;
}

void Enemy::manejarRetroceso(double deltaTime, Grid *grid)
{
    int tileSize = grid->getTileSize();

    float marginX = 4.0f;
    float marginY = 4.0f;
    float w = destRect.w;
    float h = destRect.h;

    // MOVIMIENTO EN X
    float nextX = x + knockbackVX * deltaTime;

    int leftX = static_cast<int>(std::floor((nextX + marginX) / tileSize));
    int rightX = static_cast<int>(std::floor((nextX + w - marginX) / tileSize));
    int topY = static_cast<int>(std::floor((y + marginY) / tileSize));
    int bottomY = static_cast<int>(std::floor((y + h - marginY) / tileSize));

    if (leftX >= 0 && rightX < grid->getCols() && topY >= 0 && bottomY < grid->getRows() &&
        grid->isWalkable(leftX, topY) && grid->isWalkable(rightX, topY) &&
        grid->isWalkable(leftX, bottomY) && grid->isWalkable(rightX, bottomY))
    {
        x = nextX;
    }
    else
    {
        knockbackVX = 0.0f;
    }

    // MOVIMIENTO EN Y
    float nextY = y + knockbackVY * deltaTime;

    int currentLeftX = static_cast<int>(std::floor((x + marginX) / tileSize));
    int currentRightX = static_cast<int>(std::floor((x + w - marginX) / tileSize));
    int nextTopY = static_cast<int>(std::floor((nextY + marginY) / tileSize));
    int nextBottomY = static_cast<int>(std::floor((nextY + h - marginY) / tileSize));

    if (currentLeftX >= 0 && currentRightX < grid->getCols() && nextTopY >= 0 && nextBottomY < grid->getRows() &&
        grid->isWalkable(currentLeftX, nextTopY) && grid->isWalkable(currentRightX, nextTopY) &&
        grid->isWalkable(currentLeftX, nextBottomY) && grid->isWalkable(currentRightX, nextBottomY))
    {
        y = nextY;
    }
    else
    {
        knockbackVY = 0.0f;
    }

    float friction = 15.0f;
    knockbackVX -= knockbackVX * friction * deltaTime;
    knockbackVY -= knockbackVY * friction * deltaTime;

    targetX = x;
    targetY = y;
    isMoving = false;
}

// =========================================================================
// LÓGICA GRÁFICA UNIVERSAL
// =========================================================================

void Enemy::actualizarDireccionMirada(float oldX, float oldY)
{
    if (x > oldX)
        oriented = EAST;
    else if (x < oldX)
        oriented = WEST;
    else if (y > oldY)
        oriented = SOUTH;
    else if (y < oldY)
        oriented = NORTH;
}

void Enemy::animationLogic(double deltaTime)
{
    if (dmgTimer > 0.0f)
    {
        dmgTimer -= deltaTime;

        if (std::fmod(dmgTimer, 0.1f) > 0.05f)
        {
            isTakingDmg = true;
        }
        else
        {
            isTakingDmg = false;
        }

        if (dmgTimer <= 0.0f)
        {
            isTakingDmg = false;
            dmgTimer = 0.0f;
        }
    }

    if (isMoving)
    {
        animTimer += deltaTime;
        if (animTimer >= TIME_PER_FRAME)
        {
            animTimer = 0.0f;
            currentFrame = (currentFrame + 1) % numFrames;
        }
    }
    else
    {
        currentFrame = 0;
    }

    srcRect.x = frameWidth * currentFrame;

    switch (oriented)
    {
    case SOUTH:
        srcRect.y = frameHeight * 0;
        break;
    case NORTH:
        srcRect.y = frameHeight * 1;
        break;
    case WEST:
        srcRect.y = frameHeight * 2;
        break;
    case EAST:
        srcRect.y = frameHeight * 3;
        break;
    }

    if (isTakingDmg)
    {
        srcRect.x += TakingDmgOffset;
    }
}