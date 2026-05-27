/*
 * legoclient.c
 *
 * Cliente NachOS para el servidor LegoServer local (127.0.0.1:8080).
 * El servidor detecta clientes NachOS por el header "User-Agent: nachos"
 * y responde con texto plano en lugar de HTML:
 *
 *   GET /lego/index.php          →  una figura por línea
 *   GET /lego/list.php?figure=X&part=Y  →  "cantidad|descripcion\n" por pieza
 *
 * Compilar desde NachOSx64/code/test/:
 *   make legoclient
 *
 * Ejecutar (desde userprog/):
 *   ./nachos -x ../test/legoclient
 */

#include "syscall.h"

// Buffers globales (evita desbordamiento del stack de NachOS)
char buf[2048];     // respuesta HTTP completa                       
char req[512];      // petición HTTP a enviar                           
char path[128];     // path de la URL                                   
char figure[64];    // nombre de figura ingresado por el usuario        
char part[4];       // parte: "1" o "2"                                

// Funciones auxiliares de cadenas

int mystrlen(const char *s){
    int n = 0;
    while(s[n]) n++;
    return n;
}

// Concatena src al final de dst (dst debe tener espacio suficiente)
void mystrcat(char *dst, const char *src){
    while(*dst) dst++;
    while((*dst++ = *src++));
}

// Helpers de consola

void print(const char *s){
    Write(s, mystrlen(s), ConsoleOutput);
}

/*
 * Lee una línea de ConsoleInput en dst (máx maxlen-1 chars).
 * Retorna número de caracteres leídos, -1 en EOF real.
 * El '\n' no se incluye en dst.
 */
int readline(char *dst, int maxlen){
    int i = 0, n;
    do{
        n = Read(&dst[i], 1, ConsoleInput);
        if(n < 0) return -1;   // EOF real: stdin cerrado   
        if(n == 0) break;       // salto de línea recibido   
        i++;
    }
    while(i < maxlen - 1);
    dst[i] = '\0';
    return i;
}

/
 * doRequest: abre socket, envía GET, lee respuesta en buf[].
 * Retorna total de bytes recibidos.
 * *bodyStart se establece al índice donde comienza el cuerpo HTTP
 * (después de \r\n\r\n).
  */

int doRequest(const char *reqPath, int *bodyStart){
    int sock, n, total, i;

    sock = Socket(AF_INET_NachOS, SOCK_STREAM_NachOS);
    if(sock < 0){
        print("Error: no se pudo crear el socket\n");
        Exit(1);
    }

    Connect(sock, "127.0.0.1", 8080);

    // Construir petición GET con User-Agent: nachos
    req[0] = '\0';
    mystrcat(req, "GET ");
    mystrcat(req, reqPath);
    mystrcat(req, " HTTP/1.1\r\nHost: redes.ecci\r\nUser-Agent: nachos\r\n\r\n");

    Write(req, mystrlen(req), sock);

    // Leer respuesta completa hasta EOF (servidor cierra conexión)
    total = 0;
    while(total < (int)sizeof(buf) - 1){
        n = Read(buf + total, (int)sizeof(buf) - 1 - total, sock);
        if(n <= 0) break;
        total += n;
    }
    buf[total] = '\0';

    Close(sock);

    // Ubicar inicio del cuerpo HTTP (después de \r\n\r\n)
    *bodyStart = 0;
    for (i = 0; i < total - 3; i++){
        if(buf[i]   == '\r' && buf[i+1] == '\n' && buf[i+2] == '\r' && buf[i+3] == '\n'){
            *bodyStart = i + 4;
            break;
        }
    }

    return total;
}

/
 * main
  */

int main(){
    int total, bodyStart;

    // Obtener lista de figuras disponibles
    print("Conectando al servidor LegoServer (127.0.0.1:8080)...\n\n");

    total = doRequest("/lego/index.php", &bodyStart);

    if(total <= bodyStart){
        print("Error: no se recibieron figuras del servidor\n");
        Exit(1);
    }

    print("Figuras disponibles:\n");
    Write(buf + bodyStart, total - bodyStart, ConsoleOutput);

    // Pedir nombre de figura al usuario
    print("\nDigite el nombre de la figura: ");
    if(readline(figure, (int)sizeof(figure)) < 0){
        Exit(0);
    }
    if(figure[0] == '\0'){
        print("Nombre vacio, saliendo\n");
        Exit(0);
    }

    // Pedir parte (1 o 2)
    print("Seleccione la parte (1 o 2): ");
    if(readline(part, (int)sizeof(part)) < 0){
        Exit(0);
    }
    if(part[0] != '1' && part[0] != '2'){
        print("Parte invalida, use 1 o 2\n");
        Exit(1);
    }

    // Obtener lista de piezas
    path[0] = '\0';
    mystrcat(path, "/lego/list.php?figure=");
    mystrcat(path, figure);
    mystrcat(path, "&part=");
    mystrcat(path, part);

    total = doRequest(path, &bodyStart);

    print("\n=== Lista de Piezas ===\n");
    print("Cantidad | Descripcion\n");
    print("-----------------------\n");

    if(total > bodyStart){
        Write(buf + bodyStart, total - bodyStart, ConsoleOutput);
    }
    else{
        print("(sin resultados)\n");
    }

    print("=======================\n");

    Exit(0);
}
