cmake_minimum_required(VERSION 3.14)
project(ElementalGridTactics VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. INDICAMOS DÓNDE ESTÁN LAS CABECERAS (.hpp)
# Usamos CMAKE_SOURCE_DIR por seguridad.
# Al incluir la carpeta base "include", ahora en tus .cpp usarás: #include "green-ninja/Archivo.hpp"
include_directories(${CMAKE_SOURCE_DIR}/include)

# BUSCAMOS SDL2 Y SDL2_image (Instalados previamente vía pacman)
find_package(SDL2_image REQUIRED)
find_package(SDL2 REQUIRED)

# 2. LISTADO EXPLÍCITO DE ARCHIVOS FUENTE (¡ACTUALIZADO A LA NUEVA ESTRUCTURA!)
# Aquí es donde le decimos a CMake por qué carpetas debe navegar
set(SOURCES 
    src/Main.cpp
    src/core/Game.cpp
    src/core/TextureManager.cpp
    src/entities/Player.cpp
    src/entities/Projectile.cpp
    src/entities/ProjectileModifiers.cpp
    src/entities/enemies/Enemy.cpp
    src/entities/enemies/mapache.cpp
    src/world/Grid.cpp
    src/world/Room.cpp
    src/world/Tile.cpp
)

# 3. CREAMOS EL EJECUTABLE
add_executable(${PROJECT_NAME} ${SOURCES})

# 4. ENLAZAMOS LAS LIBRERÍAS
# Conectamos tu código con el código binario de SDL2
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main SDL2::SDL2 SDL2_image::SDL2_image)

# 5. ORGANIZACIÓN DEL OUTPUT
# Mantiene el .exe en la raíz de /build para facilitar la carga de assets
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# 6. COPIAR DLL (Opcional pero muy útil en Windows)
# Copia la DLL de SDL2 al lado de tu .exe automáticamente
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()