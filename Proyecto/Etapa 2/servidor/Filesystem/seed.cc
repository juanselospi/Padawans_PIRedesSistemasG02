#include <iostream>
#include <cstring>
#include "disk.h"
#include "filesystem.h"

static const char* DISK_FILE = "lego_data.bin";
static const int   DISK_BLOCKS = 500;

// Crea un archivo y lo llena con contenido
static void seedFile( const char* name, const char* content ) {
    int inode = fs_create( name );
    if ( inode < 0 ) {
        std::cerr << "  [ERROR] No se pudo crear: " << name << "\n";
        return;
    }
    int len = strlen( content );
    int written = fs_write( inode, content, len );
    if ( written != len ) {
        std::cerr << "  [WARN] Escritura incompleta en: " << name << "\n";
    } else {
        std::cout << "  [OK] " << name << " (" << written << " bytes)\n";
    }
}

// Función principal que inicializa el disco y crea los archivos
int main() {
    std::cout << "=== SEED: Inicializando disco Lego ===\n";
    disk_init( DISK_FILE, DISK_BLOCKS );
    disk_open( DISK_FILE );
    fs_mkfs( DISK_BLOCKS );
    fs_mount();

    //archivos a ingresar
    seedFile( "index.txt", "dragon\nhorse\nfish\ngiraffe\ndeath_star_ii\nmillennium_falcon\nimperial_destroyer\n" );
    seedFile( "dragon_1.txt", "1|brick 2x2 slope orange\n2|brick 2x3 yellow\n2|brick 2x2 yellow\n" );
    seedFile( "dragon_2.txt", "1|brick 1x4 black\n2|plate 2x2 red\n1|brick 2x6 blue\n" );
    seedFile( "horse_1.txt", "1|plate 2x4 brown\n1|brick 2x2 white\n2|brick 1x2 black\n" );
    seedFile( "horse_2.txt", "1|plate 1x4 brown\n2|brick 2x2 tan\n" );
    seedFile( "fish_1.txt", "2|plate 2x2 blue\n1|slope 2x2 white\n" );
    seedFile( "fish_2.txt", "1|plate 1x3 blue\n1|tile 1x2 black\n" );
    seedFile( "giraffe_1.txt", "4|brick 2x2 black\n4|brick 2x2 yellow\n6|brick 2x1 yellow\n6|brick 2x1 black\n1|brick 2x6 yellow\n" );
    seedFile( "giraffe_2.txt", "2|brick 2x4 yellow\n1|brick 2x3 yellow\n1|brick 1x4 yellow\n1|brick 2x3 red\n2|brick 1x1 eye yellow\n" );
    seedFile( "death_star_ii_1.txt", "1|brick 2x2 light gray\n2|brick 2x2 dark gray\n1|brick 2x3 light gray\n1|brick 2x3 dark gray\n2|brick 2x4 light gray\n1|brick 2x4 dark gray\n1|brick 2x6 light gray\n1|brick 2x6 dark gray\n1|plate 2x2 black\n" );
    seedFile( "death_star_ii_2.txt", "1|plate 2x3 dark gray\n1|plate 2x4 light gray\n1|plate 2x4 black\n1|tile 2x2 dark gray\n1|tile 2x3 light gray\n1|slope 2x2 dark gray\n1|slope 2x3 light gray\n1|round 2x2 light gray\n1|round 2x2 transparent green\n" );
    seedFile( "millennium_falcon_1.txt", "2|brick 2x2 light gray\n2|brick 2x3 light gray\n2|brick 2x4 light gray\n1|brick 2x2 dark gray\n1|brick 2x3 dark gray\n2|plate 2x4 light gray\n1|plate 2x2 dark gray\n1|round 2x2 light gray\n1|round 2x2 transparent blue\n1|tile 2x2 dark gray\n" );
    seedFile( "millennium_falcon_2.txt", "2|plate 2x3 light gray\n1|plate 2x4 dark gray\n1|tile 2x3 light gray\n1|tile 2x4 dark gray\n2|slope 2x2 light gray\n1|slope 2x3 dark gray\n2|wedge 2x2 light gray\n1|wedge 2x3 light gray\n1|plate 1x4 dark gray\n1|tile 1x4 black\n" );
    seedFile( "imperial_destroyer_1.txt", "2|brick 2x2 light gray\n1|brick 2x2 dark gray\n2|brick 2x3 light gray\n1|brick 2x3 dark gray\n2|brick 2x4 light gray\n1|brick 2x4 dark gray\n1|brick 2x6 light gray\n2|plate 2x4 light gray\n1|plate 2x3 light gray\n1|plate 2x2 dark gray\n1|tile 2x2 dark gray\n1|round 2x2 light gray\n" );
    seedFile( "imperial_destroyer_2.txt", "2|wedge 2x4 light gray\n2|wedge 2x3 light gray\n2|slope 2x3 light gray\n1|slope 2x2 dark gray\n1|tile 2x4 light gray\n1|tile 2x3 dark gray\n1|plate 1x6 light gray\n1|plate 1x4 dark gray\n1|round 2x2 light gray\n2|round 1x1 dark gray\n1|tile 1x4 black\n1|plate 2x2 black\n1|slope 1x2 light gray\n" );

    disk_close();

    std::cout << "Disco inicializado.\n";
    return 0;
}
