#include "FileSystem.h"
#include "Filesystem/filesystem.h"
#include "Filesystem/disk.h"
#include <sstream>
#include <iostream>
#include <cstring>
#include <mutex>

// Mutex para proteger el acceso global al filesystem
static std::mutex fs_mutex;

FileSystem::FileSystem() {
    std::lock_guard<std::mutex> lock(fs_mutex);
    if (disk_open("Filesystem/lego_data.bin") == 0) {
        if (fs_mount() != 0) {
            std::cerr << "Error: No se pudo montar el filesystem lego_data.bin" << std::endl;
        }
    } else {
        std::cerr << "Error: No se pudo abrir el disco lego_data.bin" << std::endl;
    }
}

// Devuelve todos los nombres de figuras
std::vector<std::string> FileSystem::getFigureNames() const {
    std::lock_guard<std::mutex> lock(fs_mutex);
    
    std::cout << "[FileSystem] Solicitando lista de figuras (index.txt)" << std::endl;
    
    char buffer[1024];

    memset(buffer, 0, 1024);
    
    int bytes = fs_read("index.txt", buffer, 1024);
    if (bytes <= 0) {
        return {};
    }
    
    std::vector<std::string> names;
    std::stringstream ss(buffer);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty()) {
            // Eliminar posibles retornos de carro si vienen de windows
            if (line.back() == '\r') line.pop_back();
            names.push_back(line);
        }
    }
    return names;
}

// Devuelve las piezas de una figura y una parte específica
std::vector<Piece> FileSystem::getPieces(const std::string & figure, const std::string & part) const {
    std::lock_guard<std::mutex> lock(fs_mutex);

    std::cout << "[FileSystem] Solicitando piezas de Figura: " << figure << ", Parte: " << part << std::endl;

    std::string filename = figure + "_" + part + ".txt";

    char buffer[2048];
    memset(buffer, 0, 2048);
    
    int bytes = fs_read(filename.c_str(), buffer, 2048);
    if (bytes <= 0) {
        return {};
    }

    std::vector<Piece> pieces;
    std::stringstream ss(buffer);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        
        size_t pipe = line.find('|');
        if (pipe != std::string::npos) {
            try {
                int qty = std::stoi(line.substr(0, pipe));
                std::string desc = line.substr(pipe + 1);
                // Eliminar \r
                if (!desc.empty() && desc.back() == '\r') desc.pop_back();
                pieces.push_back({qty, desc});
            } catch (...) {
                continue;
            }
        }
    }
    
    return pieces;
}