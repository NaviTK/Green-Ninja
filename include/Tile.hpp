#pragma once
#include "TileType.hpp"
#include <SDL2/SDL.h>
#include <map>
#include <string>
// Asegúrate de incluir aquí el archivo donde definiste el nuevo enum class TileType

class Tile
{
private:
    // Ahora usamos directamente el tipo 'TileType' en lugar de un uint8_t genérico.
    // Esto hace que C++ nos avise si intentamos meterle un valor inválido.
    TileType type;

public:
    Tile();
    Tile(TileType startType);

    // Getters
    TileType getType() const;

    // Setters
    void setType(TileType newType);

    // Comprobación de estado
    bool hasType(TileType flag) const;

    bool isWalkable() const;
    // Limpieza
    void clearTo(TileType baseType);
};