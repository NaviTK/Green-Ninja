#include "Tile.hpp"

// Constructor por defecto
Tile::Tile() : type(TileType::FLOOR1) {}

// Constructor con tipo
Tile::Tile(TileType startType) : type(startType) {}

TileType Tile::getType() const
{
    return type;
}

void Tile::setType(TileType newType)
{
    type = newType;
}

bool Tile::hasType(TileType flag) const
{
    // ¡Adiós a los operadores de bits (&)! Ahora es una simple comparación.
    return type == flag;
}

void Tile::clearTo(TileType baseType)
{
    type = baseType;
}