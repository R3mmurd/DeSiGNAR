![logo](logo.png)

# DeSiGNAR (Data Structures GeNeral librARy)

This is a library that implements important generic Data Structures and
algorithms.

The structure of this library is:

| Directory        | Description|
| :-------------: |:-------------|
| *include*     | Contains all the header files. |
| *src*      | Contains all the source files (implementations declared in headers.|
| *samples* | Contains some demos with the usage of the differents developed abstractions.|
| *tests* | Contains some tests of the differents developed abstractions.|
| *obj* |In this directory will be created all the objects files when you compile the library.|
| *lib* |When you compile the library, in this directory will be added the file **libDesignar.a.**|

## Getting started

- Build the static library

  ```shell
  $ make library
  ```

- Compile samples 

  ```shell
  $ make samples
  ```

  - Compile tests

  ```shell
  $ make tests
  ```

  - Compile all of the above

  ```shell
  $ make all
  ```
