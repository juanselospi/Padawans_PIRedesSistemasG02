// LOGICA PRINCIPAL

#include "filesystem.h"
#include "disk.h"

#include <iostream>
#include <cstring>

static Superblock sb;
static Inode inode_table[MAX_INODES];

// Bitmap simple en memoria
static unsigned char bitmap[BLOCK_SIZE];

// estructura de directorio en memoria (root)
static const int ROOT_INODE = 0;

// tamaño de punteros por bloque
static const int PTRS_PER_BLOCK = BLOCK_SIZE / sizeof(int);

// helper seguro para inode table write
// Usa buffer de BLOCK_SIZE para evitar overflow en disk_write
static void persist_inodes() {
    int inodes_per_block = BLOCK_SIZE / sizeof(Inode);
    char block_buf[BLOCK_SIZE];

    for (int i = 0; i < sb.total_inodes; i += inodes_per_block) {
        memset(block_buf, 0, BLOCK_SIZE);
        int count = std::min(inodes_per_block, (int)sb.total_inodes - i);
        memcpy(block_buf, &inode_table[i], count * sizeof(Inode));
        disk_write(2 + (i / inodes_per_block), block_buf);
    }
}

// helper: leer directorio root
static int read_root_dir(DirEntry* entries) {

    Inode &root = inode_table[ROOT_INODE];

    if (root.direct[0] == -1) {
        return 0;
    }

    int result = disk_read(root.direct[0], entries);

    if (result < 0) return 0;

    // Contar solo entradas válidas (con nombre no vacío)
    int max_entries = BLOCK_SIZE / sizeof(DirEntry);
    int count = 0;
    for (int i = 0; i < max_entries; i++) {
        if (entries[i].name[0] != '\0') {
            count++;
        }
    }
    return count;
}

// helper: escribir directorio root
static void write_root_dir(DirEntry* entries) {

    Inode &root = inode_table[ROOT_INODE];

    if (root.direct[0] == -1) {

        for (int b = sb.data_start; b < sb.total_blocks; b++) {
            if (bitmap[b] == 0) {
                bitmap[b] = 1;
                root.direct[0] = b;
                break;
            }
        }
    }

    if (root.direct[0] == -1) return;

    // Persistir inodo root correctamente (via helper con buffer seguro)
    persist_inodes();

    // escribir directorio
    disk_write(root.direct[0], entries);
}

// mkfs
int fs_mkfs(int total_blocks) {

    std::cout << "Creando filesystem...\n";

    sb.magic = FS_MAGIC;
    sb.total_blocks = total_blocks;

    sb.bitmap_start = 1;
    sb.inode_start = 2;
    // inodes_per_block = 512/52 = 9, ceil(64/9) = 8 bloques de inodos (bloques 2-9)
    // data_start debe ser 2 + 8 = 10 para no solapar con los bloques de inodos
    sb.data_start = 10;

    sb.total_inodes = MAX_INODES;
    sb.free_blocks = total_blocks - sb.data_start;

    memset(bitmap, 0, BLOCK_SIZE);

    // marcar metadata como ocupada
    for (int i = 0; i < sb.data_start; i++) {
        bitmap[i] = 1;
    }

    memset(inode_table, 0, sizeof(inode_table));

    // ROOT INODE bien inicializado
    inode_table[0].used = 1;
    inode_table[0].size = 0;

    for (int i = 0; i < DIRECT_POINTERS; i++) {
        inode_table[0].direct[i] = -1;
    }

    inode_table[0].indirect = -1;
    inode_table[0].double_indirect = -1;

    inode_table[0].created = time(nullptr);
    inode_table[0].modified = time(nullptr);

    // asignar bloque REAL al directorio root
    int root_block = -1;

    for (int b = sb.data_start; b < total_blocks; b++) {
        if (bitmap[b] == 0) {
            root_block = b;
            bitmap[b] = 1;
            break;
        }
    }

    if (root_block == -1) {
        std::cout << "Error: no hay bloques para root dir\n";
        return -1;
    }

    inode_table[0].direct[0] = root_block;

    // limpiar directorio (usar buffer de tamaño exacto del bloque)
    char empty[BLOCK_SIZE];
    memset(empty, 0, sizeof(empty));

    disk_write(root_block, empty);

    // escribir metadata en disco (siempre con buffer de BLOCK_SIZE)
    {
        char block_buf[BLOCK_SIZE];
        memset(block_buf, 0, BLOCK_SIZE);
        memcpy(block_buf, &sb, sizeof(Superblock));
        disk_write(0, block_buf);
    }
    disk_write(1, bitmap);
    persist_inodes();

    std::cout << "Filesystem creado correctamente.\n";
    return 0;
}



// mount
int fs_mount() {

    std::cout << "Montando filesystem...\n";

    {
        char block_buf[BLOCK_SIZE];
        disk_read(0, block_buf);
        memcpy(&sb, block_buf, sizeof(Superblock));
    }
    disk_read(1, bitmap);

    {
        int inodes_per_block = BLOCK_SIZE / sizeof(Inode);
        char block_buf[BLOCK_SIZE];
        for (int i = 0; i < sb.total_inodes; i += inodes_per_block) {
            disk_read(2 + (i / inodes_per_block), block_buf);
            int count = std::min(inodes_per_block, (int)sb.total_inodes - i);
            memcpy(&inode_table[i], block_buf, count * sizeof(Inode));
        }
    }

    std::cout << "ROOT inode block: " << inode_table[0].direct[0] << "\n";
    std::cout << "Filesystem montado.\n";
    return 0;
}

// CREATE FILE
int fs_create(const char* name) {

    std::cout << "Creando archivo: " << name << "\n";

    DirEntry entries[BLOCK_SIZE / sizeof(DirEntry)];
    read_root_dir(entries);

    int max_entries = BLOCK_SIZE / sizeof(DirEntry);

    // buscar duplicado en todos los slots
    for (int i = 0; i < max_entries; i++) {
        if (entries[i].name[0] != '\0' && strcmp(entries[i].name, name) == 0) {
            std::cout << "Error: archivo ya existe\n";
            return -1;
        }
    }

    int inode_index = -1;

    for (int i = 0; i < MAX_INODES; i++) {
        if (!inode_table[i].used) {
            inode_index = i;
            break;
        }
    }

    if (inode_index == -1) {
        std::cout << "Error: no hay inodos libres\n";
        return -1;
    }

    Inode &inode = inode_table[inode_index];

    inode.used = 1;
    inode.size = 0;
    inode.indirect = -1;
    inode.double_indirect = -1;

    for (int i = 0; i < DIRECT_POINTERS; i++) {
        inode.direct[i] = -1;
    }

    inode.created = time(nullptr);
    inode.modified = inode.created;

    DirEntry e;
    memset(&e, 0, sizeof(e));   // limpia basura

    strncpy(e.name, name, sizeof(e.name) - 1);
    e.name[sizeof(e.name) - 1] = '\0';

    e.inode_index = inode_index;

    // Insertar en el primer slot vacío del directorio
    int slot = -1;
    for (int i = 0; i < max_entries; i++) {
        if (entries[i].name[0] == '\0') {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        std::cout << "Error: directorio lleno (sin slots)\n";
        return -1;
    }

    entries[slot] = e;

    write_root_dir(entries);

    std::cout << "Archivo creado con inode " << inode_index << "\n";

    return inode_index;
}

// RESOLVE
static int resolve(const char* name) {

    DirEntry entries[BLOCK_SIZE / sizeof(DirEntry)];
    read_root_dir(entries);

    int max_entries = BLOCK_SIZE / sizeof(DirEntry);
    for (int i = 0; i < max_entries; i++) {
        if (entries[i].name[0] != '\0' && strcmp(entries[i].name, name) == 0) {
            return entries[i].inode_index;
        }
    }

    return -1;
}

// FS_WRITE 
int fs_write(int inode_index, const char* data, int size) {

    if (inode_index < 0 || inode_index >= MAX_INODES) return -1;

    Inode &inode = inode_table[inode_index];

    if (!inode.used) return -1;

    int bytes_written = 0;

    for (int i = 0; i < DIRECT_POINTERS && bytes_written < size; i++) {

        int block = inode.direct[i];

        if (block == -1) {

            for (int b = sb.data_start; b < sb.total_blocks; b++) {
                if (bitmap[b] == 0) {
                    block = b;
                    bitmap[b] = 1;
                    inode.direct[i] = block;
                    break;
                }
            }
        }

        if (block == -1) return bytes_written;

        char buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);

        int to_copy = std::min(BLOCK_SIZE, size - bytes_written);
        memcpy(buffer, data + bytes_written, to_copy);

        disk_write(block, buffer);

        bytes_written += to_copy;
    }

    inode.size = size;
    inode.modified = time(nullptr);

    disk_write(1, bitmap);
    persist_inodes();

    return bytes_written;
}


// FS_READ 
int fs_read(const char* name, char* buffer, int size) {

    int inode_index = resolve(name);

    if (inode_index == -1) {
        std::cout << "Error: archivo no encontrado\n";
        return -1;
    }

    Inode &inode = inode_table[inode_index];

    if (size > inode.size) size = inode.size;

    int bytes_read = 0;

    // DIRECT
    for (int i = 0; i < DIRECT_POINTERS && bytes_read < size; i++) {

        int block = inode.direct[i];
        if (block == -1) continue;

        char disk_buffer[BLOCK_SIZE];
        disk_read(block, disk_buffer);

        int to_copy = std::min(BLOCK_SIZE, size - bytes_read);
        memcpy(buffer + bytes_read, disk_buffer, to_copy);

        bytes_read += to_copy;
    }

    // INDIRECT
    if (inode.indirect != -1 && bytes_read < size) {

        int ptrs[PTRS_PER_BLOCK];
        disk_read(inode.indirect, ptrs);

        for (int i = 0; i < PTRS_PER_BLOCK && bytes_read < size; i++) {

            if (ptrs[i] == -1) continue;

            char disk_buffer[BLOCK_SIZE];
            disk_read(ptrs[i], disk_buffer);

            int to_copy = std::min(BLOCK_SIZE, size - bytes_read);
            memcpy(buffer + bytes_read, disk_buffer, to_copy);

            bytes_read += to_copy;
        }
    }

    // DOUBLE INDIRECT
    if (inode.double_indirect != -1 && bytes_read < size) {

        int level1[PTRS_PER_BLOCK];
        disk_read(inode.double_indirect, level1);

        for (int i = 0; i < PTRS_PER_BLOCK && bytes_read < size; i++) {

            if (level1[i] == -1) continue;

            int level2[PTRS_PER_BLOCK];
            disk_read(level1[i], level2);

            for (int j = 0; j < PTRS_PER_BLOCK && bytes_read < size; j++) {

                if (level2[j] == -1) continue;

                char disk_buffer[BLOCK_SIZE];
                disk_read(level2[j], disk_buffer);

                int to_copy = std::min(BLOCK_SIZE, size - bytes_read);
                memcpy(buffer + bytes_read, disk_buffer, to_copy);

                bytes_read += to_copy;
            }
        }
    }

    return bytes_read;
}

// LS
int fs_ls() {

    DirEntry entries[BLOCK_SIZE / sizeof(DirEntry)];
    read_root_dir(entries);

    int max_entries = BLOCK_SIZE / sizeof(DirEntry);
    int count = 0;

    std::cout << "Contenido del directorio:\n";

    for (int i = 0; i < max_entries; i++) {
        if (entries[i].name[0] != '\0') {
            std::cout << "- " << entries[i].name 
                << " (inode " << entries[i].inode_index << ")\n";
            count++;
        }
    }

    return count;
}

// DELETE 
int fs_delete(const char* name) {

    DirEntry entries[BLOCK_SIZE / sizeof(DirEntry)];
    read_root_dir(entries);

    int max_entries = BLOCK_SIZE / sizeof(DirEntry);
    int index = -1;

    for (int i = 0; i < max_entries; i++) {
        if (entries[i].name[0] != '\0' && strcmp(entries[i].name, name) == 0) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        std::cout << "Error: archivo no encontrado\n";
        return -1;
    }

    int inode_index = entries[index].inode_index;
    Inode &inode = inode_table[inode_index];

    // liberar inode
    inode.used = 0;
    inode.size = 0;

    for (int i = 0; i < DIRECT_POINTERS; i++) {
        if (inode.direct[i] != -1) bitmap[inode.direct[i]] = 0;
        inode.direct[i] = -1;
    }

    inode.indirect = inode.double_indirect = -1;

    // limpiar la entrada del directorio
    memset(&entries[index], 0, sizeof(DirEntry));

    write_root_dir(entries);

    disk_write(1, bitmap);
    persist_inodes();

    std::cout << "Archivo eliminado: " << name << "\n";
    return 0;
}
