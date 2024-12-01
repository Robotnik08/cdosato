#ifndef dosato_filetools_h
#define dosato_filetools_h

#include "common.h"

#include <sys/stat.h>
#include <dirent.h>

int directory_exists(const char *path);
void get_files_in_directory(const char *path, char ***files, int *count);
void free_files(char **files, int count);

long long int getFileSize(FILE *file);

char* readStdin(long long int* length);

#endif // dosato_filetools_h