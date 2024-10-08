#ifndef dosato_dynamic_library_loader_h
#define dosato_dynamic_library_loader_h

#include "common.h"
#include "error.h"
#include "value.h"
#include "virtual-machine.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

typedef Value (*DosatoFunction)(ValueArray, bool debug);
typedef void (*DosatoInitFunction)(VirtualMachine*);

typedef struct {
    char* name;
    DosatoFunction function;
} DosatoFunctionMap;

typedef struct {
    DosatoFunctionMap* functions;
    size_t count;
    size_t capacity;
} DosatoFunctionMapList;

void init_DosatoFunctionMapList(DosatoFunctionMapList* list);
void write_DosatoFunctionMapList(DosatoFunctionMapList* list, DosatoFunctionMap map);
void free_DosatoFunctionMapList(DosatoFunctionMapList* list);

typedef struct {
    char* name;
    void* handle;
    DosatoInitFunction init;
    DosatoFunctionMapList functions;
} DynamicLibrary;

void init_DynamicLibrary(DynamicLibrary* lib);
void free_DynamicLibrary(DynamicLibrary* lib);

DynamicLibrary loadLib (const char* path);


#endif // dosato_dynamic_library_loader_h