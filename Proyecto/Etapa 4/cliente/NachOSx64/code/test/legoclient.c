/*
 * legoclient.c
 *
 * Cliente NachOS para el servidor LegoServer.
 * Flujo principal:
 *   1. Obtiene la lista de figuras disponibles del servidor.
 *   2. Pide al usuario el nombre de la figura y la parte (1 o 2).
 *   3. Solicita las piezas de esa figura al servidor y las imprime.
 *   4. Repite hasta que el usuario decida salir.
 *
 * El servidor responde en texto plano cuando detecta "User-Agent: nachos".
 *
 * Compilar: make legoclient  (desde NachOSx64/code/test/)
 * Ejecutar: ./nachos -x ../test/legoclient  (desde userprog/)
 */

#include "syscall.h"

/* Buffers globales: el stack de NachOS es reducido, se evita el desbordamiento */
char buf[4096]; /* respuesta HTTP recibida del servidor */
char req[512]; /* peticion HTTP a enviar */
char path[256]; /* ruta de la URL */
char figure[64]; /* nombre de figura elegido por el usuario */
char part[4]; /* parte seleccionada: "1" o "2" */
char again[4]; /* respuesta del usuario al ciclo de repeticion */

/* Funciones de cadenas: la libc estandar no esta disponible en NachOS */

int mystrlen(const char *s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

/* Agrega src al final de dst; dst debe tener espacio suficiente */
void mystrcat(char *dst, const char *src) {
    while (*dst) dst++;
    while ((*dst++ = *src++));
}

/* Copia hasta maxlen-1 caracteres de src en dst y agrega '\0' */
void mystrcpyn(char *dst, const char *src, int maxlen) {
    int i = 0;
    while (i < maxlen - 1 && src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/* Retorna c en minuscula (solo ASCII) */
char mytolower(char c) {
    if (c >= 'A' && c <= 'Z') return (char)(c + 32);
    return c;
}

/* Compara a y b sin distinguir mayusculas; retorna 0 si son iguales */
int mystricmp(const char *a, const char *b) {
    while (*a && *b) {
        if (mytolower(*a) != mytolower(*b)) return 1;
        a++; b++;
    }
    return (*a != '\0' || *b != '\0');
}

/* Escribe la cadena s en la consola */
void print(const char *s) {
    Write((char *)s, mystrlen(s), ConsoleOutput);
}

/* Lee una linea de consola en dst (max maxlen-1 chars, sin incluir '\n').
   Retorna el numero de caracteres leidos, o -1 si stdin esta cerrado. */
int readline(char *dst, int maxlen) {
    int i = 0, n;
    do {
        n = Read(&dst[i], 1, ConsoleInput);
        if (n < 0) return -1;   /* stdin cerrado (EOF) */
        if (n == 0) break;       /* fin de linea        */
        i++;
    } while (i < maxlen - 1);
    dst[i] = '\0';
    return i;
}

/* Comunicacion HTTP */

/* Abre un socket TCP, envia GET reqPath con User-Agent: nachos,
   almacena la respuesta en buf[] y cierra el socket.
   Retorna el total de bytes recibidos.
   *bodyStart queda apuntando al inicio del cuerpo HTTP (tras \r\n\r\n). */
int doRequest(const char *reqPath, int *bodyStart) {
    int sock, n, total, i;

    sock = Socket(AF_INET_NachOS, SOCK_STREAM_NachOS);
    if (sock < 0) {
        print("No se pudo crear el socket\n");
        Exit(1);
    }

    if (Connect(sock, "127.0.0.1", 8081) < 0) {
        print("No se pudo conectar al servidor intermediario\n");
        Close(sock);
        Exit(1);
    }

    /* Armar peticion GET */
    req[0] = '\0';
    mystrcat(req, "GET ");
    mystrcat(req, reqPath);
    mystrcat(req, " HTTP/1.1\r\nHost: redes.ecci\r\nUser-Agent: nachos\r\nConnection: close\r\n\r\n");

    Write(req, mystrlen(req), sock);

    /* Leer respuesta completa hasta que el servidor cierre la conexion */
    total = 0;
    while (total < (int)sizeof(buf) - 1) {
        n = Read(buf + total, (int)sizeof(buf) - 1 - total, sock);
        if (n <= 0) break;
        total += n;
    }
    buf[total] = '\0';

    Close(sock);

    /* Localizar el cuerpo HTTP buscando el separador \r\n\r\n */
    *bodyStart = 0;
    for (i = 0; i < total - 3; i++) {
        if (buf[i]   == '\r' && buf[i+1] == '\n' &&
            buf[i+2] == '\r' && buf[i+3] == '\n') {
            *bodyStart = i + 4;
            break;
        }
    }

    return total;
}

/* Logica del cliente */

/* Solicita al servidor la lista de figuras (GET /lego/index.php),
   la imprime en consola y retorna el indice de inicio del cuerpo en buf[].
   Actualiza *totalOut con los bytes totales recibidos. */
int listFigures(int *totalOut) {
    int bodyStart;
    int total = doRequest("/lego/index.php", &bodyStart);
    *totalOut = total;

    if (total <= bodyStart) {
        print("Error: el servidor no devolvio figuras disponibles\n");
        Exit(1);
    }

    print("Figuras disponibles:\n");
    Write(buf + bodyStart, total - bodyStart, ConsoleOutput);

    return bodyStart;
}

/* Pide al usuario el nombre de una figura y lo almacena en figure[].
   Repite la solicitud si la entrada esta vacia. */
void selectFigure(int bodyStart, int total) {
    int len;

    while (1) {
        print("\nDigite el nombre de la figura:\n");

        len = readline(figure, (int)sizeof(figure));
        if (len < 0) {
            Exit(0);
        }

        if (figure[0] == '\0') {
            print("Entrada vacia, intente de nuevo\n");
            print("Figuras disponibles:\n");
            Write(buf + bodyStart, total - bodyStart, ConsoleOutput);
            continue;
        }

        break;
    }
}

/* Pide al usuario la parte de la figura (1 o 2) y la almacena en part[].
   Repite la solicitud si la entrada es invalida o vacia. */
void selectPart() {
    int len;

    while (1) {
        print("Seleccione la parte de la figura: \n");
        print("1. Primera mitad\n");
        print("2. Segunda mitad\n");

        len = readline(part, (int)sizeof(part));
        if (len < 0) {
            Exit(0);
        }

        if (part[0] == '\0') {
            print("Entrada vacia, intente de nuevo\n");
            continue;
        }

        if (part[0] != '1' && part[0] != '2') {
            print("Opcion invalida, use 1 o 2\n");
            continue;
        }

        break;
    }
}

/* Recorre el cuerpo de la respuesta e imprime cada pieza en formato tabla.
   El servidor devuelve lineas con el formato "cantidad|descripcion\n". */
void printPieces(int bodyStart, int total) {
    int i;
    char qty[16];
    char desc[128];
    int qi, di;
    char c;
    int inQty;

    print("\n--- Lista de Piezas ---\n");
    print("CANTIDAD | DESCRIPCION\n");
    print("-----------------------\n");

    if (total <= bodyStart) {
        print("(sin resultados)\n");
        print("-----------------------\n\n");
        return;
    }

    i = bodyStart;
    while (i < total) {
        qi = 0;
        di = 0;
        inQty = 1;   /* 1 mientras se lee la cantidad, 0 al leer la descripcion */

        while (i < total) {
            c = buf[i++];
            if (c == '\n' || c == '\r') {
                if (c == '\r' && i < total && buf[i] == '\n') {
                    i++;   /* consumir el \n de una secuencia \r\n */
                }
                break;
            }
            if (c == '|') {
                inQty = 0;   /* separador: pasar a leer descripcion */
                continue;
            }
            if (inQty && qi < (int)sizeof(qty) - 1) {
                qty[qi++] = c;
            } else if (!inQty && di < (int)sizeof(desc) - 1) {
                desc[di++] = c;
            }
        }

        qty[qi]  = '\0';
        desc[di] = '\0';

        if (qi > 0 && di > 0) {   /* omitir lineas vacias o malformadas */
            print(qty);
            print(" | ");
            print(desc);
            print("\n");
        }
    }

    print("-----------------------\n\n");
}

int main() {
    int total, bodyStart;
    int doAgain;
    int agLen;

    print("Conectando al servidor intermediario\n\n");

    doAgain = 1;

    while (doAgain) {

        /* Obtener y mostrar figuras disponibles */
        bodyStart = listFigures(&total);

        /* Seleccionar figura y parte */
        selectFigure(bodyStart, total);
        selectPart();

        /* Construir ruta y consultar las piezas de la figura seleccionada */
        path[0] = '\0';
        mystrcat(path, "/lego/list.php?figure=");
        mystrcat(path, figure);
        mystrcat(path, "&part=");
        mystrcat(path, part);

        total = doRequest(path, &bodyStart);

        /* Mostrar la lista de piezas */
        printPieces(bodyStart, total);

        /* Preguntar si el usuario desea consultar otra figura */
        print("\xc2\xbf""Desea solicitar otra figura? (s/n): ");
        agLen = readline(again, (int)sizeof(again));
        if (agLen < 0) {
            doAgain = 0;
        } else if (again[0] == 's' || again[0] == 'S') {
            doAgain = 1;
            print("\n");
        } else {
            doAgain = 0;
        }
    }

    Exit(0);
}
