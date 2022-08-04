# cJSON library wrapper for C++

cJSON library (https://github.com/DaveGamble/cJSON) is popular minimalist JSON library for C.
This project provides a C++ 11 interface for cJSON. It is a header-only library, which is easy to integrate in project. Implementation is based on cJSON version 1.7.15.

# Tests
This project includes a tests for C++ interface. It may be referred as usage examples.
To build project, just run `make`. Default target will produce a `test` binary.
Project tests have following dependencies:
- cJSON library version 1.7.15
- CppUtest library
