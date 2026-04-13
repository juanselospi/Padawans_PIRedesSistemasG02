#include <pthread.h>
#include <iostream>
#include <queue>
#include <string>
#include <unistd.h>
#include <vector>
#include <map>

using namespace std;

typedef struct {
    string tipo; // que tipo de request
    string id; // ID del cliente original
    string figura; // la figura que el usuario pide
    vector<string> figuras; // si el usuario pide la lista
    int segmento; // mitad 1 o mitad 2
    vector<pair<string, int>> piezas; // piezas y cantidades individuales          
} Mensaje;


// CADA PIEZA TIENE SU PROPIA METADATA (se puede ampliar)
struct MetadataPieza {

    string tipo; // ej 1x1 2x2
    string fechaCreacion;
};


struct Pieza {

    string nombre;
    int cantidad;

    MetadataPieza metaPieza;
};


// LA FIGURA COMO TAL TIENE SU PROPIA METADATA
struct MetadataFigura {

    int totalPiezas;
    string fechaCreacion;
    string ultimaModificacion;
};


// CADA FIGURA TIENE PIEZAS ASOCIADAS
struct Figura {

    string nombre;

    vector<Pieza> segmento1;
    vector<Pieza> segmento2;

    MetadataFigura meta;
};


map<string, Figura> fileSystem;

map<string, vector<string>> servidores; // mapa de servidores y las figuras que tienen

queue<Mensaje> buzon_ci; // buzon cliente - intermediario
queue<Mensaje> buzon_is; // buzon intermediario - servidor

// buzon entre intermediarios
queue<Mensaje> buzon_ii; 
pthread_mutex_t mutex_ii;

pthread_mutex_t mutex_ci; // buzon cliente - intermediario
pthread_mutex_t mutex_is; // buzon intermediario - servidor

bool terminado = false;


// simulador de cliente IPC
void* cliente(void* arg) {

    string cliente = "B74200"; // id generica, en otra etapa se genera encriptada y se negocia con el intermeidiario para que no hayan 2 iguales
    string peticion;
    int segmento;

    cout << "Para ver LEGOS disponibles digite: LEGOS" << endl << "Para pedir un KIT digite el nombre" << endl;

    getline(cin, peticion);

    Mensaje m;

    bool confirmada = false;


    // peticion = "Death Star II";

    while(confirmada == false){

        // verifico la peticion del usuario, aunque sea incorrecta (se maneja mas adelante)
        if(peticion == "LEGOS") {

            m.tipo = "LIST_FIGURES";
            confirmada = true;
            break;
        } else {

            m.tipo = "GET_FIGURE";
            m.figura= peticion;

            cout << "que mitad desea, 1 o 2?" << endl;

            cin >> segmento;

            if(segmento != 1 && segmento != 2) {

                cout << "peticion invalida" << endl;
                cout << "segmento 1 por defecto" << endl;
                m.segmento = 1;
                confirmada = true;
            } else {
                m.segmento = segmento;
                confirmada = true;
            }
            break;
        }

    }


    m.id = cliente;

    pthread_mutex_lock(&mutex_ci);

    buzon_ci.push(m); // meto el mensaje al buzon

    pthread_mutex_unlock(&mutex_ci);

    cout << "El cliente " << m.id << " pide " << peticion << endl;

    // se espera la respuesta del intermediario
    while(true) {

        pthread_mutex_lock(&mutex_ci);

        if(!buzon_ci.empty()) {

            Mensaje respuesta = buzon_ci.front();

            if(respuesta.tipo == "RETURN_FIGURE" && respuesta.id == cliente) { // VERIFICAR RETURN_FIGURE CON ARIADNA

                buzon_ci.pop();

                pthread_mutex_unlock(&mutex_ci);

                // imprimo el contenido de la respeusta que recive le cliente
                cout << "FIGURA: " << respuesta.figura << endl;
                cout << "SEGMENTO: " << respuesta.segmento << endl;
                cout << "PIEZAS:" << endl;
                for (const pair<string, int>& p : respuesta.piezas) {
                    cout << "- " << p.first << " x" << p.second << endl;
                }

                cout << "TOTAL DE PIEZAS:" << endl;
                int total = 0;
                for (const auto& p : respuesta.piezas) {
                    total += p.second;
                }

                cout << total << endl;


                terminado = true;

                break;

            } else if (respuesta.tipo == "FIGURES_LIST") {

                buzon_ci.pop();

                pthread_mutex_unlock(&mutex_ci);

                cout << "Lista de LEGOS:" << endl;

                for (const string& fs : respuesta.figuras) {
                    cout << "- " << fs << endl;
                }

                terminado = true;

                break;

            } else if (respuesta.tipo == "FIGURE_NOT_FOUND") {

                buzon_ci.pop();

                pthread_mutex_unlock(&mutex_ci);

                cout << "Figura no encontrada" << endl;

                terminado = true;

                break;
            }
        }

        pthread_mutex_unlock(&mutex_ci);

        usleep(100000);
    }

    pthread_exit(NULL);
}


// simulacro de intermediario
void* intermediario(void* arg) {

    while(!terminado) {

        pthread_mutex_lock(&mutex_ci);

        if(!buzon_ci.empty()) {

            Mensaje m = buzon_ci.front();

            if(m.tipo == "GET_FIGURE") {

                buzon_ci.pop();
                pthread_mutex_unlock(&mutex_ci);

                bool encontrado = false;

                for(auto& s : servidores) {
                    for(auto& f : s.second) {
                        if(f == m.figura) {
                            encontrado = true;
                            break;
                        }
                    }
                }

                if(!encontrado) {

                    cout << "Intermediario: consultando a otro intermediario..." << endl;

                    Mensaje consulta;
                    consulta.tipo = "ASK_INTERMEDIARY";
                    consulta.id = m.id;
                    consulta.figura = m.figura;
                    consulta.segmento = m.segmento;

                    pthread_mutex_lock(&mutex_ii);
                    buzon_ii.push(consulta);
                    pthread_mutex_unlock(&mutex_ii);

                    continue;
                }

                Mensaje peticion;
                peticion.tipo = "ASK_FIGURE";
                peticion.id = m.id;
                peticion.figura = m.figura;
                peticion.segmento = m.segmento;

                pthread_mutex_lock(&mutex_is);
                buzon_is.push(peticion);
                pthread_mutex_unlock(&mutex_is);

                continue;
            }
        }

        pthread_mutex_unlock(&mutex_ci);


        // escucha la respuesta del otro intermediario
        pthread_mutex_lock(&mutex_ii);

        if(!buzon_ii.empty()) {

            Mensaje m = buzon_ii.front();

            if(m.tipo == "INTERMEDIARY_FOUND") {

                buzon_ii.pop();
                pthread_mutex_unlock(&mutex_ii);

                Mensaje respuesta;
                respuesta.tipo = "RETURN_FIGURE";
                respuesta.id = m.id;
                respuesta.figura = m.figura;
                respuesta.segmento = m.segmento;

                // respuesta vacía de prueba
                respuesta.piezas.clear();

                pthread_mutex_lock(&mutex_ci);
                buzon_ci.push(respuesta);
                pthread_mutex_unlock(&mutex_ci);

                continue;

            } else if(m.tipo == "INTERMEDIARY_NOT_FOUND") {

                buzon_ii.pop();
                pthread_mutex_unlock(&mutex_ii);

                Mensaje respuesta;
                respuesta.tipo = "FIGURE_NOT_FOUND";
                respuesta.id = m.id;

                pthread_mutex_lock(&mutex_ci);
                buzon_ci.push(respuesta);
                pthread_mutex_unlock(&mutex_ci);

                continue;
            }
        }

        pthread_mutex_unlock(&mutex_ii);


        pthread_mutex_lock(&mutex_is);

        if(!buzon_is.empty()) {

            Mensaje m = buzon_is.front();

            if(m.tipo == "FIGURE_FOUND") {

                buzon_is.pop();
                pthread_mutex_unlock(&mutex_is);

                Mensaje respuesta;
                respuesta.tipo = "RETURN_FIGURE";
                respuesta.id = m.id;
                respuesta.figura = m.figura;
                respuesta.segmento = m.segmento;
                respuesta.piezas = m.piezas;

                pthread_mutex_lock(&mutex_ci);
                buzon_ci.push(respuesta);
                pthread_mutex_unlock(&mutex_ci);

                continue;
            }
        }

        pthread_mutex_unlock(&mutex_is);

        usleep(100000);
    }

    pthread_exit(NULL);
}


// simulacro segundo intermediario
void* intermediario2(void* arg) {

    while(!terminado) {

        pthread_mutex_lock(&mutex_ii);

        if(!buzon_ii.empty()) {

            Mensaje m = buzon_ii.front();

            if(m.tipo == "ASK_INTERMEDIARY") {

                buzon_ii.pop();
                pthread_mutex_unlock(&mutex_ii);

                cout << "Intermediario2 consultado: " << m.figura << endl;

                Mensaje respuesta;
                respuesta.id = m.id;
                respuesta.figura = m.figura;
                respuesta.segmento = m.segmento;


                if(m.figura == "Millenium Falcon") {

                    respuesta.tipo = "INTERMEDIARY_FOUND";

                } else {

                    respuesta.tipo = "INTERMEDIARY_NOT_FOUND";
                }

                pthread_mutex_lock(&mutex_ii);
                buzon_ii.push(respuesta);
                pthread_mutex_unlock(&mutex_ii);

                continue;
            }
        }

        pthread_mutex_unlock(&mutex_ii);

        usleep(100000);
    }

    pthread_exit(NULL);
}


// simulacion basica de un server
void* servidor(void* arg) {

    while(!terminado) {

        pthread_mutex_lock(&mutex_is);

        if(!buzon_is.empty()) {

            Mensaje m = buzon_is.front();

            if(m.tipo == "ASK_FIGURE") {

                buzon_is.pop();

                pthread_mutex_unlock(&mutex_is);

                cout << "Servidor procesa solicitud " << m.tipo << " de cliente " << m.id << endl;

                Mensaje respuesta;
                respuesta.id = m.id;
                respuesta.figura = m.figura;
                respuesta.segmento = m.segmento;


                if(m.figura.empty()) {

                    respuesta.tipo = "FIGURE_NOT_FOUND";

                } else if(m.figura == "ALL") {

                    respuesta.tipo = "FIGURE_FOUND";

                    for(auto& entry : fileSystem) {
                        respuesta.figuras.push_back(entry.first);
                    }

                } else if(fileSystem.find(m.figura) != fileSystem.end()) {

                    respuesta.tipo = "FIGURE_FOUND";

                    Figura figura = fileSystem[m.figura];

                    vector<Pieza> bloque;

                    if(m.segmento == 1) {

                        bloque = figura.segmento1;

                    } else {

                        bloque = figura.segmento2;
                    }

                    for(auto& b : bloque) {

                        respuesta.piezas.push_back({b.nombre, b.cantidad});
                    }

                } else {

                    respuesta.tipo = "FIGURE_NOT_FOUND";
                }


                pthread_mutex_lock(&mutex_is);

                buzon_is.push(respuesta);

                pthread_mutex_unlock(&mutex_is);

                cout << "Servidor respondio solicitud " << m.tipo << " de cliente " << m.id << endl;

                continue;
            }
        }

        pthread_mutex_unlock(&mutex_is);

        usleep(100000);
    }

    pthread_exit(NULL);
}


int main() {

    // POR AHORA SE INICIALIZA LA FIGURA EN MANUALMENTE, MAS ADELANTE SERA ALMANCENADA EN DISCO
    Figura deathStar;

    deathStar.nombre = "Death Star II";
    deathStar.meta = {16, "2026-03-20", "2026-03-20"};


    // MOCKUPS
    // segmento 1
    deathStar.segmento1.push_back({"brick 2x2 yellow", 1, {"bloque", "2026-03-20"}});
    deathStar.segmento1.push_back({"brick 2x4 white", 2, {"bloque", "2026-03-20"}});


    // segmento 2
    deathStar.segmento2.push_back({"brick 2x3 yellow", 5, {"bloque", "2026-03-20"}});
    deathStar.segmento2.push_back({"brick 1x1 yellow eye", 4, {"decorativo", "2026-03-20"}});


    // guardar en filesystem
    fileSystem["Death Star II"] = deathStar;

    // inicializar servidores
    servidores["server1"].push_back("Death Star II");

    pthread_t hilos[4];

    pthread_mutex_init(&mutex_ci, NULL);
    pthread_mutex_init(&mutex_is, NULL);
    pthread_mutex_init(&mutex_ii, NULL); 

    pthread_create(&hilos[0], NULL, servidor, NULL);
    pthread_create(&hilos[1], NULL, intermediario, NULL);
    pthread_create(&hilos[2], NULL, intermediario2, NULL);
    pthread_create(&hilos[3], NULL, cliente, NULL);

    // join de hilos
    for(int i = 0; i < 4; i++) {

        pthread_join(hilos[i], NULL);
    }

    pthread_mutex_destroy(&mutex_ci);
    pthread_mutex_destroy(&mutex_is);
    pthread_mutex_destroy(&mutex_ii);

    return 0;
}

