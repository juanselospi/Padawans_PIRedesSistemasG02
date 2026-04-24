// OPEN / READ/ WRITE / LSEEK

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "disk.h"

static int disk_fd = -1;

// crear disco en bloques de tamaño fijo
int disk_init(const char* path, int num_blocks) {
    int fd = open(path, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("Error creando disco");
        return -1;
    }

    // expandir archivo al tamaño deseado
    off_t size = num_blocks * BLOCK_SIZE;
    if (lseek(fd, size - 1, SEEK_SET) == -1) {
        perror("Error en lseek");
        close(fd);
        return -1;
    }

    if (write(fd, "", 1) != 1) {
        perror("Error expandiendo archivo");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

// abrir disco existente
int disk_open(const char* path) {
    disk_fd = open(path, O_RDWR);
    if (disk_fd < 0) {
        perror("Error abriendo disco");
        return -1;
    }
    return 0;
}

// cerrar disco
int disk_close() {
    if (disk_fd >= 0) {
        close(disk_fd);
        disk_fd = -1;
    }
    return 0;
}

// leer bloque
int disk_read(int block_num, void* buffer) {
    if (disk_fd < 0) {
        fprintf(stderr, "Disco no abierto\n");
        return -1;
    }

    off_t offset = block_num * BLOCK_SIZE;

    if (lseek(disk_fd, offset, SEEK_SET) == -1) {
        perror("Error en lseek (read)");
        return -1;
    }

    int bytes = read(disk_fd, buffer, BLOCK_SIZE);
    if (bytes != BLOCK_SIZE) {
        perror("Error leyendo bloque");
        return -1;
    }

    return 0;
}

// escribir bloque
int disk_write(int block_num, const void* buffer) {
    if (disk_fd < 0) {
        fprintf(stderr, "Disco no abierto\n");
        return -1;
    }

    off_t offset = block_num * BLOCK_SIZE;

    if (lseek(disk_fd, offset, SEEK_SET) == -1) {
        perror("Error en lseek (write)");
        return -1;
    }

    int bytes = write(disk_fd, buffer, BLOCK_SIZE);
    if (bytes != BLOCK_SIZE) {
        perror("Error escribiendo bloque");
        return -1;
    }

    return 0;
}
