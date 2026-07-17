![logo](logo.png)

<em>Read this in other languages: <a href="README.md">English</a></em>

# DeSiGNAR (Data Structures GeNeral librARy)

Una biblioteca de C++17 con fines didácticos que implementa estructuras de
datos y algoritmos genéricos: arreglos, listas, pilas/colas, varios tipos de
árboles balanceados (AVL, rojo-negro, treap, splay, BST aleatorizado), tablas
hash (encadenadas y de direccionamiento abierto), heaps, grafos y algoritmos
sobre grafos, primitivas geométricas, algoritmos de cadenas, y algunos
contenedores conscientes de concurrencia.

## Estructura de directorios

| Directorio  | Descripción |
| :---------- | :----------- |
| `include`  | Todos los archivos de cabecera (esta es una biblioteca basada en plantillas con mucho código de cabecera — la mayor parte de la implementación vive aquí). |
| `src`      | Archivos de implementación de las partes que no son completamente header-only. |
| `samples` | Pequeños programas de demostración que muestran cómo usar las distintas estructuras de datos. |
| `tests`    | La batería de pruebas, un ejecutable por cada área de estructura de datos/algoritmo. |
| `cmake`    | Módulos auxiliares de CMake (detección de compilador/plataforma, plantilla de configuración de paquete). |
| `obj`, `lib` | Salida de compilación de este repositorio (ver más abajo); no existen hasta que compiles. |

## Requisitos

- Un compilador C++17: GCC ≥ 9, Clang ≥ 14, o MSVC (Visual Studio 2019+).
- CMake ≥ 3.20.
- Threads (usado por los contenedores conscientes de concurrencia): pthreads
  en Linux/macOS, threading nativo en Windows — CMake lo encuentra
  automáticamente, no hay que instalar nada extra.

## Compilando desde el código fuente

La misma invocación de CMake funciona en Linux, macOS y Windows; solo
cambia ligeramente la sintaxis del paso de compilación según la
plataforma, como se detalla abajo.

### Linux / macOS

```shell
mkdir build && cd build
cmake ..
cmake --build . --parallel
ctest --output-on-failure   # ejecuta la batería de pruebas
```

Por defecto esto también compila `samples/` y `tests/`. Si solo quieres la
biblioteca (por ejemplo, si la estás embebiendo vía `add_subdirectory`),
configura con:

```shell
cmake -DDESIGNAR_BUILD_SAMPLES=OFF -DDESIGNAR_BUILD_TESTS=OFF ..
```

Para instalar en un prefijo estándar (de modo que otros proyectos puedan
usar `find_package(Designar)` — ver más abajo):

```shell
cmake --install . --prefix /usr/local   # o el prefijo que prefieras
```

### Windows (MSVC)

Desde un "Developer Command Prompt for VS" (o cualquier shell con `cmake`
y MSVC en el `PATH`):

```bat
mkdir build && cd build
cmake ..
cmake --build . --config Debug
ctest -C Debug --output-on-failure
```

MSVC es un generador multi-configuración, por lo que la bandera
`--config`/`-C` (aquí, Debug) es necesaria, a diferencia de Linux/macOS.

### Docker

Sin necesidad de un toolchain local — un `Dockerfile` en la raíz del
repositorio compila la biblioteca y ejecuta toda la batería de pruebas
dentro de una imagen limpia de Ubuntu:

```shell
docker build -t designar .
docker run --rm designar               # compila y luego corre las pruebas
docker run --rm -it --entrypoint bash designar   # o una shell interactiva
```

### Opciones útiles de CMake

| Opción | Por defecto | Efecto |
| :----- | :---------- | :----- |
| `DESIGNAR_BUILD_SAMPLES` | `ON` | Compila los programas de demostración en `samples/`. |
| `DESIGNAR_BUILD_TESTS` | `ON` | Compila (y registra con ctest) la batería de pruebas en `tests/`. |
| `DESIGNAR_WARNINGS_AS_ERRORS` | `OFF` | Trata los warnings del compilador como errores (lo que usa CI). |
| `DESIGNAR_SANITIZE_ADDRESS_UB` | `OFF` | Compila con AddressSanitizer + UndefinedBehaviorSanitizer. |
| `DESIGNAR_SANITIZE_THREAD` | `OFF` | Compila con ThreadSanitizer (mutuamente excluyente con la opción anterior). |

> **Nota:** la batería de pruebas se verifica a sí misma con `assert()`.
> Configurar con `CMAKE_BUILD_TYPE=Release` (o `RelWithDebInfo`/`MinSizeRel`)
> define `NDEBUG`, lo cual elimina todo `assert()` — esto deja varias
> variables locales genuinamente sin uso en las pruebas, lo que rompe la
> compilación bajo `DESIGNAR_WARNINGS_AS_ERRORS=ON`. Si quieres una
> compilación optimizada, deja `DESIGNAR_BUILD_TESTS=OFF`, o no combines
> `Release` con `DESIGNAR_WARNINGS_AS_ERRORS=ON`.

## Consumiendo Designar desde otro proyecto CMake

Una vez instalada (ver `cmake --install` arriba, o vía vcpkg/Conan más abajo):

```cmake
find_package(Designar REQUIRED)
target_link_libraries(your_target PRIVATE Designar::Designar)
```

### vcpkg

Hay un port disponible en el propio repositorio, en `vcpkg/ports/designar`
(todavía no está en el registro curado de vcpkg, ya que este proyecto aún
no tiene un release etiquetado):

```shell
vcpkg install designar --overlay-ports=/ruta/a/DeSiGNAR/vcpkg/ports
```

### Conan

Un `conanfile.py` en la raíz del repositorio genera un paquete Conan 2.x:

```shell
conan create .
```

### Generando un archivo de release del código fuente

`cpack --config CPackSourceConfig.cmake` (ejecutado desde un directorio de
compilación ya configurado) produce `designar-<version>-src.tar.gz`/`.zip`
que contiene únicamente lo necesario para compilar la biblioteca
(`include/`, `src/`, los archivos de CMake, `LICENSE`, `README.md`) — no
las pruebas, los samples, la configuración de Doxygen ni las recetas de
empaquetado de este repositorio, que permanecen en el control de versiones
para los contribuidores pero no tienen razón de estar en un archivo de
release.

## Documentación

La documentación de la API se genera con Doxygen (ver el `Doxyfile` en la
raíz del repositorio) y se publica automáticamente desde `main` — ver el
job `publish_docs` en `.github/workflows/build_library.yml` para más
detalles, y habilita GitHub Pages ("Settings → Pages → Source: GitHub
Actions") en tu fork para tener tu propia copia.

Para generar la documentación localmente:

```shell
doxygen Doxyfile      # salida en en-doc/html/index.html (inglés)
doxygen Doxyfile.es   # salida en es-doc/html/index.html (español)
```

Nota: los comentarios Doxygen del código fuente (comentarios `/** ... */`
sobre clases y funciones) están en inglés — es la convención establecida
en todo el código de esta biblioteca y la que siguen la mayoría de las
bibliotecas de C++. Lo que sí está disponible en ambos idiomas es esta guía
de introducción/instalación (la página principal generada por Doxygen).

## Licencia

MIT — ver `LICENSE`.
