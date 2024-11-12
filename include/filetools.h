#ifndef dosato_filetools_h
#define dosato_filetools_h

#include "common.h"

long long int getFileSize(FILE *file);

char* readStdin(long long int* length);

#endif // dosato_filetools_h