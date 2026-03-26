# Simulación de Comunicación Cliente–Intermediario–Servidor con Hilos

## Integrantes

Grupo: Los Padawan de CHiki

Juan L
Juan G
Jeferson M
Ariadna


## Descripción General

Este programa implementa una simulación de comunicación en un sistema distribuido utilizando **hilos (threads)** en C++. Se modelan tres componentes principales:

- Cliente
- Intermediario
- Servidor

La comunicación entre estos componentes se realiza mediante **colas compartidas (buzones)** protegidas con **mutex**, simulando un esquema tipo IPC.

---

## Arquitectura del Sistema

El sistema sigue el siguiente flujo:

Cliente → Intermediario → Servidor → Intermediario → Cliente


### Componentes:

### Cliente
- Realiza solicitudes al sistema.
- Puede:
  - Solicitar la lista de figuras (`LEGOS`)
  - Solicitar una figura específica y un segmento (1 o 2)
- Espera la respuesta del intermediario.

---

### Intermediario
- Actúa como un **middleware** o punto central de control.
- Se encarga de:
  - Validar si una figura existe antes de consultar al servidor.
  - Decidir si responde directamente o si debe contactar al servidor.
- Mantiene un **mapa de servidores** con las figuras disponibles.

---

### Servidor
- Simula un backend que contiene un “filesystem” de figuras.
- Responde a solicitudes de:
  - Lista de figuras
  - Detalles de una figura específica

---

## Estructuras de Datos

### `Mensaje`
Representa la unidad de comunicación entre los componentes.

Contiene:
- Tipo de mensaje (`GET_FIGURE`, `LIST_FIGURES`, etc.)
- ID del cliente
- Nombre de la figura
- Una lista de figuras
- Segmento solicitado
- Lista de piezas o figuras

---

### `Figura` y `Pieza`
- Una figura está compuesta por piezas.
- Cada figura tiene:
  - Dos segmentos (`segmento1`, `segmento2`)
  - Metadata (fechas, total de piezas)

---

### `fileSystem`
Tiene la forma:
map<string, Figura>


**Ejemplo de Uso**
# Para compilar

make

# Para correr

make run

# Output esperado

 I  ~/De/V/Proyecto PI Redes y Sistemas/P/Etapa 1/Simulacro Protocolo  main !1 ?1  make run                        ✔  7s  21:17:27 
./ProtocolSim.out
Para ver LEGOS disponibles digite: LEGOS
Para pedir un KIT digite el nombre
LEGOS
El cliente B74200 pide LEGOS
Intermediario recibio solicitud LIST_FIGURES del cliente B74200
Intermediario se comunica con el servidor para resolver ASK_FIGURE
Servidor procesa solicitud ASK_FIGURE de cliente B74200
Servidor respondio solicitud ASK_FIGURE de cliente B74200
Intermediario recibio respuesta del server
Intermediario resuelve FIGURE_FOUND para cliente B74200
Lista de LEGOS:
- Death Star II

# Figura seleccionada:
Death Star II
![](https://github.com/juanselospi/Team3_PIRedesSistemasG02/blob/main/Proyecto/Etapa%201/Recursos/Figura.png)
## Partes:
![](https://github.com/juanselospi/Team3_PIRedesSistemasG02/blob/main/Proyecto/Etapa%201/Recursos/partes_figura_lego1.png)
![](https://github.com/juanselospi/Team3_PIRedesSistemasG02/blob/main/Proyecto/Etapa%201/Recursos/6f46a9c6-1.png)
