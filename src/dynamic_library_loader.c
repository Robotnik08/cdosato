#include "../include/common.h"
#include "../include/memory.h"

#include "../include/debug.h"
#include "../include/dynamic_library_loader.h"
#include "../include/error.h"
#include "../include/value.h"
#include "../include/virtual-machine.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

DOSATO_LIST_FUNC_GEN(DosatoFunctionMapList, DosatoFunctionMap, functions)


void init_DynamicLibrary(DynamicLibrary* lib) {
    lib->name = NULL;
    lib->handle = NULL;
    lib->init = NULL;
    init_DosatoFunctionMapList(&lib->functions);
}

void free_DynamicLibrary(DynamicLibrary* lib) {
    free_DosatoFunctionMapList(&lib->functions);
    free(lib->name);
    lib->name = NULL;
    lib->handle = NULL;
    lib->init = NULL;
}

DynamicLibrary loadLib (const char* path) {
    DynamicLibrary lib;
    init_DynamicLibrary(&lib);
    lib.name = malloc(strlen(path) + 2);
    strcpy(lib.name, path);

    #ifdef _WIN32

    char* full_path = malloc(strlen(path) + 5);
    strcpy(full_path, path);
    strcat(full_path, ".dll");
    lib.handle = LoadLibraryA(full_path);
    if (!lib.handle) {
        printf("Error loading library '%s': Error code: %d\n", full_path, GetLastError());
        exit(1);
    }
    
    lib.init = (DosatoInitFunction)GetProcAddress(lib.handle, "init");
    if (!lib.init) {
        printf("Error, library must have init function: %s\n", full_path);
        exit(1);
    }

    // call the init function
    lib.init(main_vm);

    DosatoFunctionMapList* functions = (DosatoFunctionMapList*)GetProcAddress(lib.handle, "functions");

    if (!functions) {
        printf("Error, library must have functions list: %s\n", full_path);
        exit(1);
    }

    lib.functions = *functions;

    #else // unix
    
    char* full_path = malloc(strlen(path) + 4);
    strcpy(full_path, path);
    strcat(full_path, ".so");
    lib.handle = dlopen(full_path, RTLD_LAZY);

    if (!lib.handle) {
        printf("Error loading library '%s': %s\n", full_path, dlerror());
        exit(1);
    }

    lib.init = (DosatoInitFunction)dlsym(lib.handle, "init");

    if (!lib.init) {
        printf("Error, library must have init function: %s\n", full_path);
        exit(1);
    }

    // call the init function
    lib.init(main_vm);

    DosatoFunctionMapList* functions = (DosatoFunctionMapList*)dlsym(lib.handle, "functions");

    if (!functions) {
        printf("Error, library must have functions list: %s\n", full_path);
        exit(1);
    }
    
    lib.functions = *functions;

    #endif

    return lib;
}



