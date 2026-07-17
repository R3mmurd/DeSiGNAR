![logo](logo.png)

<em>Read this in other languages: <a href="README.es.md">Español</a></em>

# DeSiGNAR (Data Structures GeNeral librARy)

A teaching-oriented C++17 library of generic data structures and algorithms:
arrays, lists, stacks/queues, several balanced-tree flavors (AVL, red-black,
treap, splay, randomized BST), hash tables (chained and open-addressing),
heaps, graphs and graph algorithms, geometry primitives, string algorithms,
and a handful of concurrency-aware containers.

## Directory structure

| Directory  | Description |
| :--------- | :----------- |
| `include`  | All header files (this is a header-heavy, template-based library — most of the actual code lives here). |
| `src`      | Implementation files for the pieces that aren't fully header-only. |
| `samples`  | Small demo programs showing how to use the various data structures. |
| `tests`    | The test suite, one executable per data structure/algorithm area. |
| `cmake`    | CMake helper modules (compiler/platform detection, package config template). |
| `obj`, `lib` | Build output for this repo's own in-tree build (see below); not present until you build. |

## Requirements

- A C++17 compiler: GCC ≥ 9, Clang ≥ 14, or MSVC (Visual Studio 2019+).
- CMake ≥ 3.20.
- Threads (used by the concurrency-aware containers): pthreads on Linux/macOS,
  native threading on Windows — CMake finds this automatically, nothing extra
  to install.

## Building from source

The same CMake invocation works on Linux, macOS, and Windows; only the build
step's syntax differs slightly per platform below.

### Linux / macOS

```shell
mkdir build && cd build
cmake ..
cmake --build . --parallel
ctest --output-on-failure   # run the test suite
```

By default this also builds `samples/` and `tests/`. If you only want the
library (e.g. you're embedding this via `add_subdirectory`), configure with:

```shell
cmake -DDESIGNAR_BUILD_SAMPLES=OFF -DDESIGNAR_BUILD_TESTS=OFF ..
```

To install into a standard prefix (so other projects can `find_package
(Designar)` — see below):

```shell
cmake --install . --prefix /usr/local   # or your prefix of choice
```

### Windows (MSVC)

From a "Developer Command Prompt for VS" (or any shell with `cmake` and MSVC
on `PATH`):

```bat
mkdir build && cd build
cmake ..
cmake --build . --config Debug
ctest -C Debug --output-on-failure
```

MSVC is a multi-configuration generator, so the `--config`/`-C` flag (Debug
here) is required where it isn't on Linux/macOS.

### Docker

No local toolchain needed — a `Dockerfile` at the repo root builds the
library and runs the full test suite inside a clean Ubuntu image:

```shell
docker build -t designar .
docker run --rm designar               # builds, then runs the test suite
docker run --rm -it --entrypoint bash designar   # interactive shell instead
```

### Useful CMake options

| Option | Default | Effect |
| :----- | :------ | :----- |
| `DESIGNAR_BUILD_SAMPLES` | `ON` | Build the demo programs in `samples/`. |
| `DESIGNAR_BUILD_TESTS` | `ON` | Build (and register with ctest) the test suite in `tests/`. |
| `DESIGNAR_WARNINGS_AS_ERRORS` | `OFF` | Treat compiler warnings as errors (what CI builds with). |
| `DESIGNAR_SANITIZE_ADDRESS_UB` | `OFF` | Build with AddressSanitizer + UndefinedBehaviorSanitizer. |
| `DESIGNAR_SANITIZE_THREAD` | `OFF` | Build with ThreadSanitizer (mutually exclusive with the option above). |

> **Note:** the test suite checks itself with `assert()`. Configuring with
> `CMAKE_BUILD_TYPE=Release` (or `RelWithDebInfo`/`MinSizeRel`) defines
> `NDEBUG`, which compiles every `assert()` out — this leaves several
> now-genuinely-unused local variables in the tests, which fails the build
> under `DESIGNAR_WARNINGS_AS_ERRORS=ON`. If you want an optimized build,
> either leave `DESIGNAR_BUILD_TESTS=ON` off, or don't combine `Release`
> with `DESIGNAR_WARNINGS_AS_ERRORS=ON`.

## Consuming Designar from another CMake project

Once installed (see `cmake --install` above, or via vcpkg/Conan below):

```cmake
find_package(Designar REQUIRED)
target_link_libraries(your_target PRIVATE Designar::Designar)
```

### vcpkg

A port lives in-tree at `vcpkg/ports/designar` (not yet in the curated
vcpkg registry, since this project has no tagged release yet):

```shell
vcpkg install designar --overlay-ports=/path/to/DeSiGNAR/vcpkg/ports
```

### Conan

A `conanfile.py` at the repo root builds a Conan 2.x package:

```shell
conan create .
```

### Building a release source archive

`cpack --config CPackSourceConfig.cmake` (run from a configured build
directory) produces `designar-<version>-src.tar.gz`/`.zip` containing only
what's needed to build the library itself (`include/`, `src/`, the CMake
files, `LICENSE`, `README.md`) — not this repo's tests, samples, Doxygen
config, or packaging recipes, which stay in version control for
contributors but have no reason to ship in a release archive.

## Documentation

API documentation is generated with Doxygen — in both English (`Doxyfile`)
and Spanish (`Doxyfile.es`) editions — and published automatically from
`main`; see the `publish_docs` job in
`.github/workflows/build_library.yml` for how, and enable GitHub Pages
("Settings → Pages → Source: GitHub Actions") once on a fork to get your own
copy. The two editions share the same generated API reference (this
library's Doxygen comments are English throughout); what's translated is
the getting-started/installation main page (this file, and its
<a href="README.es.md">Spanish counterpart</a>).

To build the docs locally:

```shell
doxygen Doxyfile      # output in en-doc/html/index.html
doxygen Doxyfile.es   # output in es-doc/html/index.html
```

## License

MIT — see `LICENSE`.
