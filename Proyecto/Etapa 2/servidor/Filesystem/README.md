# Filesystem

- `disk.h/cc`: Capa de abstracción del disco. Maneja la lectura y escritura de bloques de 512 bytes.
- `filesystem.h/cc`: Lógica principal del FS (Superbloque, Inodos, Bitmaps, Directorios).
- `fs_cli.cc`: Herramienta de línea de comandos para interactuar con el FS.
- `main.cc`: Pruebas automatizadas básicas.
- `seed.cc`: Script para inicializar el disco con datos de prueba (figuras predefinidas).

## Guía de uso con la Terminal (`fs_cli`)

Desde la carpeta del servidor:

```bash
make
```

### Comandos Disponibles

#### Listar archivos

Muestra todos los archivos guardados:

```bash
./fs_cli ls
```

#### Ingresar una nueva figura (Subir archivo)

Toma un archivo y lo ingresa al filesystem:

```bash
# Ejemplo:
./fs_cli put mi_figura_local.txt figura_en_fs.txt
```

#### Leer una figura (Ver contenido)

Muestra el contenido de un archivo guardado en el FS.

```bash
./fs_cli cat figura_en_fs.txt
```

#### Eliminar una figura

Borra el archivo y libera los bloques ocupados.

```bash
./fs_cli rm figura_en_fs.txt
```

#### Formatear el disco

Crea un sistema de archivos limpio con un número específico de bloques.

```bash
# Ejemplo para 500 bloques:
./fs_cli format 500
```

---

## Inicialización Rápida (Seed)

Se puede usar el `seed` para llenar el disco con datos de prueba. Compilar y ejecutar:

```bash
g++ seed.cc filesystem.cc disk.cc -o seed
./seed
```

Esto genera el archivo `lego_data.bin`.
