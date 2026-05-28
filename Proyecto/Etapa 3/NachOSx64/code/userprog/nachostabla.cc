#include "nachostabla.h"
#include "system.h"
#include <unistd.h>
#include <stdio.h>

//  Las posiciones 0, 1 y 2 se marcan como usadas (stdin/stdout/stderr).
NachosOpenFilesTable::NachosOpenFilesTable() {
    openFiles = new int[ MAX_OPEN_FILES ];
    openFilesMap = new BitMap( MAX_OPEN_FILES );
    usage = 0;

    // Inicializar todo a -1 (libre)
    for ( int i = 0; i < MAX_OPEN_FILES; i++ ) {
        openFiles[ i ] = -1;
    }

    // Reservar los descriptores 0 (ConsoleInput), 1 (ConsoleOutput),
    openFilesMap->Mark( 0 ); // ConsoleInput
    openFilesMap->Mark( 1 ); // ConsoleOutput
    openFilesMap->Mark( 2 ); // ConsoleError
}


NachosOpenFilesTable::~NachosOpenFilesTable() {
    // Cerrar archivos abiertos (evitar los 3 primeros: consola)
    for ( int i = 3; i < MAX_OPEN_FILES; i++ ) {
        if ( openFilesMap->Test( i ) ) {
            close( openFiles[ i ] );
        }
    }
    delete [] openFiles;
    delete openFilesMap;
}


//  Registra un descriptor Unix y retorna el descriptor NachOS asignado.
int NachosOpenFilesTable::Open( int UnixHandle ) {
    int NachosHandle = openFilesMap->Find();   // Encuentra primer slot libre y lo marca
    if ( NachosHandle == -1 ) {
        // Tabla llena
        return -1;
    }
    openFiles[ NachosHandle ] = UnixHandle;
    return NachosHandle;
}


//  Libera el slot NachOS y retorna el descriptor Unix correspondiente.
int NachosOpenFilesTable::Close( int NachosHandle ) {
    if (NachosHandle < 0 || NachosHandle >= MAX_OPEN_FILES) {
        return -1;
    }
    if (!openFilesMap->Test( NachosHandle )) {
        return -1;   // No estaba abierto
    }
    int UnixHandle = openFiles[ NachosHandle ];
    openFilesMap->Clear( NachosHandle );
    openFiles[ NachosHandle ] = -1;
    return UnixHandle;
}

//  Retorna true si el descriptor NachOS está en uso.
bool NachosOpenFilesTable::isOpened( int NachosHandle ) {
    if ( NachosHandle < 0 || NachosHandle >= MAX_OPEN_FILES ) {
        return false;
    }
    return openFilesMap->Test( NachosHandle );
}

//  Retorna el descriptor Unix asociado al descriptor NachOS.
int NachosOpenFilesTable::getUnixHandle( int NachosHandle ) {
    if ( !isOpened( NachosHandle ) ) {
        return -1;
    }
    return openFiles[ NachosHandle ];
}


//  Un hilo adicional va a usar esta tabla; incrementar el contador.
void NachosOpenFilesTable::addThread() {
    usage++;
}


//  Un hilo dejó de usar esta tabla.  Si es el último, el destructor
//  cerrará todo.
void NachosOpenFilesTable::delThread() {
    usage--;
}


//  Imprime el estado actual de la tabla.
void NachosOpenFilesTable::Print() {
    printf( "NachosOpenFilesTable: usage = %d\n", usage );
    for ( int i = 0; i < MAX_OPEN_FILES; i++ ) {
        if ( openFilesMap->Test( i ) ) {
            printf( "  [%d] -> Unix fd %d\n", i, openFiles[ i] );
        }
    }
}
