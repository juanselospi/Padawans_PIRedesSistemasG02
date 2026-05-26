#ifndef NACHOSTABLA_H
#define NACHOSTABLA_H

#include "bitmap.h"

#define MAX_OPEN_FILES 128   // Máximo de archivos abiertos simultáneamente

// NachosOpenFilesTable
//
//  Maneja los archivos abiertos de un espacio de direcciones de usuario.
//  Los descriptores 0, 1 y 2 están reservados para stdin, stdout y stderr
//  (ConsoleInput, ConsoleOutput, ConsoleError) y no se almacenan aquí.
//

class NachosOpenFilesTable {
  public:
    NachosOpenFilesTable();           // Inicializar la tabla
    ~NachosOpenFilesTable();          // Liberar la tabla

    int  Open( int UnixHandle );      // Registrar un descriptor Unix; retorna el NachOS handle
    int  Close( int NachosHandle );   // Liberar un descriptor NachOS; retorna el Unix handle
    bool isOpened( int NachosHandle );
    int  getUnixHandle( int NachosHandle );

    void addThread();                 // Incrementar contador de hilos que usan esta tabla
    void delThread();                 // Decrementar contador; si llega a 0 cierra todo
    int  getUsage() { return usage; }

    void Print();                     // Imprimir contenido (debugging)

  private:
    int    * openFiles;               // Vector de descriptores Unix (-1 = libre)
    BitMap * openFilesMap;            // Bitmap para controlar slots libres
    int      usage;                   // Número de hilos que comparten esta tabla
};

#endif // NACHOSTABLA_H
