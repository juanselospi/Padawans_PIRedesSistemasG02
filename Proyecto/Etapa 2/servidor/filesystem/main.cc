// PRUEBAS / DEMO

#include <iostream>
#include <cstring>

#include "disk.h"
#include "filesystem.h"

int fs_mkfs(int);
int fs_mount();
int fs_create(const char*);
int fs_write(int, const char*, int);
int fs_read(const char*, char*, int);
int fs_ls();
int fs_delete(const char*);

int main() {

    std::cout << "=== TEST FILESYSTEM ===\n";

    // crear disco si no existe
    disk_init("filesystem.bin", 500);

    // abrir disco
    disk_open("filesystem.bin");

    fs_mkfs(500);
    fs_mount();

    fs_create("file1.txt");

    const char* data = "Hola filesystem!";
    fs_write(1, data, strlen(data));

    char buffer[100];
    memset(buffer, 0, 100);

    fs_read("file1.txt", buffer, 100);

    std::cout << "Contenido: " << buffer << "\n";

    fs_ls();

    fs_delete("file1.txt");

    fs_ls();

    disk_close();

    return 0;
}