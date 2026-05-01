#pragma once
#include <string>
#include <vector>
#include <utility> // Para std::pair
#include "green-ninja/Player.hpp"

class ConfigManager {
private:
    // Función de utilidad para limpiar espacios y saltos de línea molestos
    std::string trim(const std::string& str);

public:
    // El método principal al que llamará el main
    void loadAndApplyConfig(const std::string& filename, Player* player);
};