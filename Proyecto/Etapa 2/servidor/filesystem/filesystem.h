#pragma once

#include <ctime>
#include <cstring>

#define BLOCK_SIZE 512
#define MAX_INODES 64
#define MAX_FILENAME 28   
#define DIRECT_POINTERS 5

// número de punteros que caben en un bloque
#define POINTERS_PER_BLOCK (BLOCK_SIZE / sizeof(int))

// número mágico para validar el FS
#define FS_MAGIC 0x12345678

// superbloque
struct Superblock {
    int magic;
    int total_blocks;
    int free_blocks;

    int bitmap_start;
    int inode_start;
    int data_start;

    int total_inodes;
};

// inodo
struct Inode {
    int used;
    int size;

    int direct[DIRECT_POINTERS];
    int indirect;
    int double_indirect;

    time_t created;
    time_t modified;
};

struct DirEntry {
    char name[MAX_FILENAME];

    int inode_index;
};

// API del Filesystem
int fs_mkfs(int total_blocks);
int fs_mount(); 
int fs_create(const char* name);
int fs_write(int inode_index, const char* data, int size);
int fs_read(const char* name, char* buffer, int size);
int fs_ls();
int fs_delete(const char* name);

