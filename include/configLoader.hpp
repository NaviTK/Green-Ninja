#pragma once
#include <string>
#include <map>
#include <fstream>
#include <iostream>

class ConfigLoader
{
public:
    static std::map<std::string, std::string> LoadSprites(const std::string &filePath)
    {
        std::map<std::string, std::string> spritePaths;
        std::ifstream file(filePath);
        std::string line;

        if (!file.is_open())
        {
            std::cerr << "Error: No se pudo abrir el archivo de configuracion: " << filePath << std::endl;
            return spritePaths;
        }

        while (std::getline(file, line))
        {
            // Ignorar líneas vacías o comentarios
            if (line.empty() || line[0] == '#')
                continue;

            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos)
            {
                std::string key = line.substr(0, delimiterPos);
                std::string path = line.substr(delimiterPos + 1);
                spritePaths[key] = path;
            }
        }

        file.close();
        return spritePaths;
    }
};