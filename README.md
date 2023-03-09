![logo](logo.png)

# DeSiGNAR (Data Structures GeNeral librARy)

This is a library that implements important generic Data Structures and
algorithms.

Currently the library requires C++20 in order to built, because 
all code was updated to new modules feature of std in order to improve
the build times in big projects. 

Since even some compilers need to add better support to the module feature, the current version is not yet error-free. As compilers improve their ability to handle modules, the library will also be updated with these changes.

The structure of this library is:

| Directory        | Description|
| :-------------: |:-------------|
| *src*      | Contains all the source files. |
| *samples* | Contains some demos with the usage of the differents developed abstractions.|
| *tests* | Contains some tests of the differents developed abstractions.|
| *obj* |In this directory will be created all the objects files when you compile the library.|
| *lib* |When you compile the library, in this directory will be added the file **libDesignar.a.**|


## Requirements
  - [Clang++ - Version 17 or newer](https://apt.llvm.org/)
  - [Xmake](https://xmake.io/#/guide/installation)

## Getting started

- This command is required in order to set clang as default compiler.
  ```shell
  $ xmake f --cxx=clang++ --ld=clang++
  ```


- Build the static library

  ```shell
  $ xmake library
  ```

- Compile samples 

  ```shell
  $ xmake -g samples
  ```

- Compile tests

  ```shell
  $ xmake -g tests
  ```

- Compile all of the above

  ```shell
  $ xmake
  ```
