#ifndef NACHOSTABLA_H
#define NACHOSTABLA_H

#include "bitmap.h"

#define MAX_OPEN_FILES 128   // Máximo de archivos abiertos simultáneamente

class NachosOpenFilesTable {
  public:
    NachosOpenFilesTable();
    ~NachosOpenFilesTable();

    // NachOS handle
    int  Open( int UnixHandle );// Registrar un descriptor Unix
    int  Close( int NachosHandle ); // Liberar un descriptor NachOS
    bool isOpened( int NachosHandle );
    int  getUnixHandle( int NachosHandle );

    void addThread(); // Incrementar contador de hilos que usan esta tabla
    void delThread(); // Decrementar contador; si llega a 0 cierra todo
    int  getUsage() { return usage; }

    void Print();

  private:
    int    * openFiles; // Vector de descriptores Unix (-1 = libre)
    BitMap * openFilesMap; // Bitmap para controlar slots libres
    int      usage; // Número de hilos que comparten esta tabla
};

#endif
