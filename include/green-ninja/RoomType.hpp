#pragma once

enum class RoomType
{
    NONE,   // Sin tipo asignado (útil para debugging)
    START,  // Donde naces, sin enemigos
    NORMAL, // Sala estándar con enemigos y rocas
    BIG,    // Sala más grande de lo normal
    GOLDEN, // Sala del tesoro (item gratis)
    BOSS    // Sala del jefe final
};