#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include "disk.h"
#include "filesystem.h"

// Imprime el uso del comando
void print_usage() {
    std::cout << "Uso: ./fs_cli <comando> [argumentos]\n";
    std::cout << "Comandos:\n";
    std::cout << "  ls                       - Listar archivos\n";
    std::cout << "  cat <nombre>             - Mostrar contenido de un archivo\n";
    std::cout << "  rm <nombre>              - Borrar un archivo\n";
    std::cout << "  put <host_file> <fs_name> - Copiar archivo desde el sistema host al FS\n";
    std::cout << "  format <bloques>         - Formatear el disco con N bloques\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        
        return 1;
    }

    const char* disk_file = "lego_data.bin";
    std::string command = argv[1];

    if (command == "format") {
        if (argc < 3) {
            std::cout << "Error: especifica el número de bloques.\n";
            return 1;
        }

        int blocks = std::stoi(argv[2]);
        disk_init(disk_file, blocks);
        disk_open(disk_file);
        fs_mkfs(blocks);
        disk_close();

        return 0;
    }

    // Para los demás comandos, se abre el disco
    if (disk_open(disk_file) < 0) {
        std::cerr << "Error: No se pudo abrir el disco " << disk_file << ". ¿Ya lo formateaste?\n";
        return 1;
    }

    if (fs_mount() < 0) {
        std::cerr << "Error: No se pudo montar el filesystem.\n";
        disk_close();
        return 1;
    }

    if (command == "ls") {
        fs_ls();
    } 
    else if (command == "cat") {
        if (argc < 3) {
            std::cout << "Error: especifica el nombre del archivo.\n";
        } else {
            char buffer[1024 * 10]; // 10 kB
            memset(buffer, 0, sizeof(buffer));
            int read = fs_read(argv[2], buffer, sizeof(buffer) - 1);
            if (read >= 0) {
                std::cout << "--- Contenido de " << argv[2] << " ---\n";
                std::cout << buffer << "\n";
                std::cout << "---------------------------\n";
            }
        }
    } 
    else if (command == "rm") {
        if (argc < 3) {
            std::cout << "Error: especifica el nombre del archivo.\n";
        } else {
            fs_delete(argv[2]);
        }
    } 
    else if (command == "put") {
        if (argc < 4) {
            std::cout << "Error: uso put <host_file> <fs_name>\n";
        } else {
            std::ifstream file(argv[2], std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                std::cerr << "Error: No se pudo abrir el archivo host: " << argv[2] << "\n";
            } else {
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);

                std::vector<char> buffer(size);
                if (file.read(buffer.data(), size)) {
                    int inode = fs_create(argv[3]);
                    if (inode >= 0) {
                        fs_write(inode, buffer.data(), size);
                        std::cout << "Archivo '" << argv[3] << "' guardado correctamente (" << size << " bytes).\n";
                    }
                }
            }
        }
    } 
    else {
        std::cout << "Comando desconocido: " << command << "\n";
        print_usage();
    }

    disk_close();
    return 0;
}
