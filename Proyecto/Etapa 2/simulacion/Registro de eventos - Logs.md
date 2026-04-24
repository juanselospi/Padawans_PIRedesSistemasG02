UNIVERSIDAD DE COSTA RICA

ESCUELA DE CIENCIAS DE LA COMPUTACIÓN E INFORMÁTICA

PROYECTO INTEGRADOR DE SISTEMAS OPERATIVOS Y  REDES DE COMUNICACIÓN

 

Proyecto del curso \- Etapa II

***Registro de eventos (Logs)***

**DOCENTE:**

Prof. Maeva Murcia Meléndez

Prof. Francisco Arroyo

**Estudiantes:**

Ariadna Ardila Gutiérrez C4C646

Jafet Araya C4C537

Juan González C4F588

Juan Loaiza B74200

Jeferson Marín C24549

Jason Montenegro C4H386

Iván Ríos C4I919

John Vargas C4K672

I Semestre \- 2026

### **Registro de eventos (logs)**

### Introducción

El manejo de bitácoras es una pieza fundamental de la comunicación entre intermediarios, para poder registrar el correcto o incorrecto funcionamiento del sistema planteado. Para esta causa se plantea un formato de log a registrar, siendo este un mensaje de texto plano que describe lo que ocurre durante la simulación. 

Los logs planteados tienen un formato estructurado de la siguiente manera:

***\[ENTIDAD\] \[FECHA\_HORA\] \[TIPO\_LOG\] \[MENSAJE\]***

La entidad corresponde al componente que genera o recibe el evento dentro del sistema. La fecha y hora indican el momento en que ocurre el suceso. El tipo de log puede ser ***SOLICITUD, ENRUTAMIENTO, RESPUESTA o ERROR***. Finalmente, el mensaje describe de forma concreta la acción realizada o el evento ocurrido, dependiendo del contexto de la comunicación entre componentes.

### Envío de solicitudes

	El envío de solicitudes se refiere a cómo los distintos componentes generan y envían requerimientos a otros componentes del sistema.

EStas solicitudes corresponden a las operaciones definidas por el protocolo de interacción, tales como ***LIST\_FIGURES, GET\_FIGURE o ASK\_FIGURE***, dependiendo del flujo de comunicación entre componentes.

Se registran únicamente los envíos relevantes que representan el inicio de una operación dentro del sistema. A continuación, se muestran ejemplos de logs de solicitudes para cada entidad.

* ***Cliente***

\[CLIENTE\] \[10/04/2026 09:00\] \[SOLICITUD\] LIST\_FIGURES  

\[CLIENTE\] \[10/04/2026 09:02\] \[SOLICITUD\] GET\_FIGURE figura=orchid segmento=1 

* ***Intermediario***

\[INTERM\] \[10/04/2026 09:02\] \[SOLICITUD\] ASK\_FIGURE figura=orchid segmento=1  

\[INTERM\] \[10/04/2026 09:03\] \[SOLICITUD\] GET\_FIGURE figura=orchid segmento=1 (reenvío a intermediarios)

### Gestión y enrutamiento

Esta sección registra las decisiones que toma el intermediario para manejar las solicitudes dentro del sistema, así como la información que utiliza para hacerlo. Estos logs permiten seguir cómo se procesan las solicitudes y cómo el intermediario determina a dónde enviarlas, según la información disponible.

Puede incluir acciones como la consulta o actualización de la tabla de rutas, la selección de servidores, el reenvío de solicitudes a otros intermediarios y la incorporación o eliminación de nodos, etc…

A continuación, se muestran ejemplos de logs de gestión y enrutamiento.

* ***Intermediario***

\[INTERM\] \[10/04/2026 08:55\] \[ENRUTAMIENTO\] Registrando nuevo intermediario

\[INTERM\] \[10/04/2026 08:56\] \[ENRUTAMIENTO\] Actualizando tabla de rutas

\[INTERM\] \[10/04/2026 09:02\] \[ENRUTAMIENTO\] Figura encontrada en tabla de rutas  

\[INTERM\] \[10/04/2026 09:02\] \[ENRUTAMIENTO\] Enviando solicitud a servidor 

\[INTERM\] \[10/04/2026 09:03\] \[ENRUTAMIENTO\] Figura no encontrada localmente  

\[INTERM\] \[10/04/2026 09:03\] \[ENRUTAMIENTO\] Reenviando GET\_FIGURE (interm-interm)  

\[INTERM\] \[10/04/2026 09:10\] \[ENRUTAMIENTO\] Eliminando servidor de tabla de rutas 

### Generación de respuestas

La generación de respuestas corresponde al registro de los mensajes producidos como resultado del procesamiento de una solicitud. Estos logs permiten verificar que las respuestas fueron enviadas correctamente al componente que realizó la solicitud inicialmente.

Este tipo de log puede incluir todas las respuestas definidas en el protocolo, tales como ***FIGURES\_LIST, RETURN\_FIGURE, FIGURE\_FOUND o FIGURE\_NOT\_FOUND***. A continuación, se muestran ejemplos de logs de respuestas para cada entidad.

* ***Cliente***

\[CLIENTE\] \[10/04/2026 16:08\] \[RESPUESTA\] RETURN\_FIGURE figura=orchid segmento=1

* ***Intermediario***

\[INTERM\] \[10/04/2026 09:45\] \[RESPUESTA\] FIGURE\_NOT\_FOUND figura=dog segmento=1

\[INTERM\] \[10/04/2026 09:01\] \[RESPUESTA\] FIGURES\_LIST

* ***Servidor***

\[SERVIDOR\] \[10/04/2026 09:45\] \[RESPUESTA\] FIGURE\_FOUND figura=orchid segmento=2

### Ocurrencia de errores

La ocurrencia de errores se refiere al manejo de comportamientos erróneos dentro del sistema, tales como fallos de conexión, inconsistencias en los datos o imposibilidad de completar una operación.

Este tipo de log puede contemplar errores asociados a fallos en la comunicación entre componentes, ausencia de respuestas, inconsistencias en la tabla de rutas, indisponibilidad de servidores o intermediarios, así como cualquier otra condición que impida completar correctamente una operación definida en el protocolo

A continuación, se muestran ejemplos de logs de error para cada entidad.

* ***Cliente***

\[CLIENTE\] \[10/04/2026 09:07\] \[ERROR\] Timeout esperando respuesta

* ***Intermediario***

\[INTERM\] \[10/04/2026 09:06\] \[ERROR\] Ningún intermediario respondió a la solicitud

\[INTERM\] \[10/04/2026 09:05\] \[ERROR\] Error de conexión con servidor

* ***Servidor***

\[SERVIDOR\] \[10/04/2026 09:06\] \[ERROR\] Error procesando solicitud ASK\_FIGURE 

