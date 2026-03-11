cmake_minimum_required(VERSION 3.14)
project(ElementalGridTactics VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. INDICAMOS DONDE ESTÁN LAS CABECERAS (.hpp)
# Esto permite que hagas #include "Game.hpp" sin poner la ruta de la carpeta
include_directories(include)

find_package(SDL2_image REQUIRED)
# BUSCAMOS SDL2 (Instalado previamente vía pacman)
find_package(SDL2 REQUIRED)

# LISTADO EXPLÍCITO DE ARCHIVOS FUENTE
set(SOURCES 
    src/main.cpp
    src/Game.cpp
    src/TextureManager.cpp
    src/Player.cpp
    src/Projectile.cpp
    src/Grid.cpp
    src/Tile.cpp
    src/Room.cpp
    src/Mapache.cpp
    src/Enemy.cpp
)

# CREAMOS EL EJECUTABLE
add_executable(${PROJECT_NAME} ${SOURCES})

# 5. ENLAZAMOS LAS LIBRERÍAS
# Conectamos tu código con el código binario de SDL2
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main SDL2::SDL2 SDL2_image::SDL2_image)

# 6. ORGANIZACIÓN DEL OUTPUT
# Mantiene el .exe en la raíz de /build para facilitar la carga de assets
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# 7. COPIAR DLL (Opcional pero muy útil en Windows)
# Copia la DLL de SDL2 al lado de tu .exe automáticamente
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()