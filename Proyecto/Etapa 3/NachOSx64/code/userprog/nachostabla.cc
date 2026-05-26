#include "nachostabla.h"
#include "system.h"
#include <unistd.h>
#include <stdio.h>

// NachosOpenFilesTable::NachosOpenFilesTable
//  Constructor: inicializa el vector y el bitmap.
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
    // 2 (ConsoleError) para que no sean asignados a archivos de usuario.
    openFilesMap->Mark( 0 );   // ConsoleInput
    openFilesMap->Mark( 1 );   // ConsoleOutput
    openFilesMap->Mark( 2 );   // ConsoleError
}


// NachosOpenFilesTable::~NachosOpenFilesTable
//  Destructor: cierra los archivos Unix que aún estén abiertos y libera.

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


// NachosOpenFilesTable::Open
//  Registra un descriptor Unix y retorna el descriptor NachOS asignado.
//  Retorna -1 si la tabla está llena.

int NachosOpenFilesTable::Open( int UnixHandle ) {
    int NachosHandle = openFilesMap->Find();   // Encuentra primer slot libre y lo marca
    if ( NachosHandle == -1 ) {
        // Tabla llena
        return -1;
    }
    openFiles[ NachosHandle ] = UnixHandle;
    return NachosHandle;
}


// NachosOpenFilesTable::Close
//  Libera el slot NachOS y retorna el descriptor Unix correspondiente.
//  Retorna -1 si el descriptor no estaba abierto.

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


// NachosOpenFilesTable::isOpened
//  Retorna true si el descriptor NachOS está en uso.

bool NachosOpenFilesTable::isOpened( int NachosHandle ) {
    if ( NachosHandle < 0 || NachosHandle >= MAX_OPEN_FILES ) {
        return false;
    }
    return openFilesMap->Test( NachosHandle );
}


// NachosOpenFilesTable::getUnixHandle
//  Retorna el descriptor Unix asociado al descriptor NachOS.
//  Retorna -1 si no está abierto.

int NachosOpenFilesTable::getUnixHandle( int NachosHandle ) {
    if ( !isOpened( NachosHandle ) ) {
        return -1;
    }
    return openFiles[ NachosHandle ];
}


// NachosOpenFilesTable::addThread
//  Un hilo adicional va a usar esta tabla; incrementar el contador.

void NachosOpenFilesTable::addThread() {
    usage++;
}


// NachosOpenFilesTable::delThread
//  Un hilo dejó de usar esta tabla.  Si es el último, el destructor
//  cerrará todo.

void NachosOpenFilesTable::delThread() {
    usage--;
}


// NachosOpenFilesTable::Print
//  Imprime el estado actual de la tabla.

void NachosOpenFilesTable::Print() {
    printf( "NachosOpenFilesTable: usage = %d\n", usage );
    for ( int i = 0; i < MAX_OPEN_FILES; i++ ) {
        if ( openFilesMap->Test( i ) ) {
            printf( "  [%d] -> Unix fd %d\n", i, openFiles[ i] );
        }
    }
}
