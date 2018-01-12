DeSiGNAR (Data Structures GeNeral librARy)

This is the beginning of a complete project which is expected to be
a general library to make different kinds of models and simulation.

The structure of this library is:

- include directory: Contains all the header files.
- src directory: Contains all the source files (implementations declared in
headers.
- samples directory: Contains some test and demos with the usage of the
differents developed abstractions.
- obj directory: In this directory will be created all the objects files when
you compile the library.
- lib directory: When you compile the library, in this directory will be
added the file libDesignar.a.
- bin directory: When you compile the samples, in this directory will be added
the binary files to execute.

In order to compile the library you must execute "make library".

In order to compile the samples you must execute "make samples".

TODO:
- Tarjan Algorithm for computing strong connected components to Digraph.
- Making all the documentation.
- Completing test-map.C.
- Making test for Graph and Digraph.