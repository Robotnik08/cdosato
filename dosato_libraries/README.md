## Library API

### dosato.h

This is the main header files for the dosato api. Import this file to use the dosato api.

### test_library.c

This is a test example library that demonstrates how to use the dosato api. <br>

The Makefile is quite convoluted, but it should work using the following commands:
```bash
make LIB_NAME=<libname> HEADER_PATH=<directoryofdosato.h> DOSATO_LIB_PATH=<pathofdosatolib>
```