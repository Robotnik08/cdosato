#include "../include/common.h"
#include "../include/memory.h"

#include "../include/debug.h"
#include "../include/dynamic_library_loader.h"
#include "../include/error.h"
#include "../include/value.h"

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
    lib->name = NULL;
    lib->handle = NULL;
    lib->init = NULL;
}

DynamicLibrary loadLib (const char* path) {
    DynamicLibrary lib;
    init_DynamicLibrary(&lib);
    lib.name = strdup(path);

    #ifdef _WIN32
    lib.handle = LoadLibraryA(path);
    if (!lib.handle) {
        printf("Error loading library (or library was not found): %s\n", path);
        exit(1);
    }
    
    lib.init = (DosatoFunctionEmpty)GetProcAddress(lib.handle, "init");
    if (!lib.init) {
        printf("Error, library must have init function: %s\n", path);
        exit(1);
    }

    // call the init function
    lib.init();

    DosatoFunctionMapList* functions = (DosatoFunctionMapList*)GetProcAddress(lib.handle, "functions");

    if (!functions) {
        printf("Error, library must have functions list: %s\n", path);
        exit(1);
    }

    lib.functions = *functions;

    #else
    lib.handle = dlopen(path, RTLD_LAZY);
    if (!lib.handle) {
        printf("Error loading library: %s\n", dlerror());
        exit(1);
    }

    lib.init = (DosatoFunctionEmpty)dlsym(lib.handle, "init");
    if (!lib.init) {
        printf("Error, library must have init function: %s\n", dlerror());
        exit(1);
    }
    
    // call the init function
    lib.init();

    DosatoFunctionMapList* functions = (DosatoFunctionMapList*)dlsym(lib.handle, "functions");

    if (!functions) {
        printf("Error, library must have functions list: %s\n", dlerror());
        exit(1);
    }

    lib.functions = *functions;
    #endif

    return lib;
}



