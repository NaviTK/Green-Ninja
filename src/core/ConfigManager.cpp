#include "green-ninja/ConfigManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

// Limpia los espacios en blanco, tabuladores y saltos de línea alrededor del texto
std::string ConfigManager::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return ""; // Si todo eran espacios, devuelve vacío
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void ConfigManager::loadAndApplyConfig(const std::string& filename, Player* player) {
    std::ifstream file(filename);
    std::string line;
    
    // Aquí preparamos nuestra lista de pares vacía
    std::vector<std::pair<std::string, SDL_Scancode>> parsedBindings;

    // Si el archivo no existe, no hacemos nada y el Player se queda con sus controles por defecto
    if (!file.is_open()) {
        std::cerr << "[ConfigManager] No se encontro " << filename << ". Usando controles por defecto." << std::endl;
        return; 
    } 

    while (std::getline(file, line)) {
        // DETALLE PRO: Ignorar líneas vacías o comentarios que empiecen por '#'
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream is_line(line);
        std::string key, value;
        
        // Separamos por el '='
        if (std::getline(is_line, key, '=') && std::getline(is_line, value)) {
            
            // Limpiamos la "basura invisible" (espacios extra)
            key = trim(key);
            value = trim(value);
            
            // Le pedimos a SDL que traduzca el texto (ej: "W") a su código interno
            SDL_Scancode scancode = SDL_GetScancodeFromName(value.c_str());
            
            if (scancode != SDL_SCANCODE_UNKNOWN) {
                // Empaquetamos la accion y la tecla, y la metemos a la lista
                parsedBindings.push_back({key, scancode});
            } else {
                std::cerr << "[ConfigManager] Error: Tecla invalida '" << value << "' para la accion '" << key << "'" << std::endl;
            }
        }
    }
    
    file.close();
    
    // Si logramos leer al menos una configuración, se la pasamos al jugador
    if (!parsedBindings.empty()) {
        player->setkeybinds(parsedBindings);
        std::cout << "[ConfigManager] Exito: Se enviaron " << parsedBindings.size() << " configuraciones de teclas al jugador." << std::endl;
    }
}