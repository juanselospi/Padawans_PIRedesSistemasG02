// MANEJO DE ARCHIVO.BIN

#pragma once

#define BLOCK_SIZE 512

// crear disco de bloques de tamaño fijo
int disk_init(const char* path, int num_blocks);

// abre o crea el archivo que simula el disco
int disk_open(const char* path);

// cierra el disco
int disk_close();

// lee un bloque completo
int disk_read(int block_num, void* buffer);

// escribe un bloque completo
int disk_write(int block_num, const void* buffer);
