#include <cstdint>
#pragma once
/**
 * @brief Define los tipos de terreno disponibles en el juego.
 * Ahora es un enum class secuencial. Permite hasta 256 tipos de casillas.
 */
enum class TileType : uint8_t
{
    NONE = 0,
    FLOOR1,     // 1
    FLOOR2,     // 2
    FLOOR3,     // 3
    WALL,       // 4
    RIGHTDOOR,  // 5
    LEFTDOOR,   // 6
    TOPDOOR,    // 7
    BOTTOMDOOR, // 8
    WATER,
    OIL,
    FUEGO,
    // ¡Puedes seguir añadiendo decenas de estados hacia abajo sin miedo!
};