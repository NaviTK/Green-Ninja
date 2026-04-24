#include <cstdint>
#pragma once
/**
 * @brief Define los tipos de terreno disponibles en el juego.
 * Ahora es un enum class secuencial. Permite hasta 256 tipos de casillas.
 */
enum class TileType : uint8_t
{
    NONE = 0,
    FLOOR1,           // 1
    FLOOR2,           // 2
    FLOOR3,           // 3
    WALL,             // 4
    RIGHTDOOR,        // 5
    LEFTDOOR,         // 6
    TOPDOOR,          // 7
    BOTTOMDOOR,       // 8
    CLOSEDRIGHTDOOR,  // 9
    CLOSEDLEFTDOOR,   // 10
    CLOSEDTOPDOOR,    // 11
    CLOSEDBOTTOMDOOR, // 12
    FOSA,             // 13
    ROCK1,            // 14
    ROCK2,            // 15
    FUEGO,
    AGUA
};