#include "../include/common.h"

#include "../include/memory.h"
#include "../include/value.h"

DOSATO_LIST_FUNC_GEN(ValueArray, Value, values)

void destroyValueArray(ValueArray* array) {
    for (size_t i = 0; i < array->count; i++) {
        destroyValue(array->values[i]);
    }
    free_ValueArray(array);
}

void destroyValue(Value value) {
    if (value.type == TYPE_STRING) {
        free(value.as.stringValue);
    }
}

DOSATO_LIST_FUNC_GEN(StackFrames, size_t, stack)

void init_NameMap(NameMap* map) {
    map->names = NULL;
    map->count = 0;
    map->capacity = 0;
}

void write_NameMap(NameMap* map, char* name) {
    if (map->capacity < map->count + 1) {
        size_t oldCapacity = map->capacity;
        map->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        map->names = DOSATO_RESIZE_LIST(char*, map->names, oldCapacity, map->capacity);
    }
    map->names[map->count] = name;
    map->count++;
}

void free_NameMap(NameMap* map) {
    for (size_t i = 0; i < map->count; i++) {
        free(map->names[i]);
    }
    free(map->names);
    init_NameMap(map);
}

bool hasName(NameMap* map, char* name) {
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->names[i], name) == 0) {
            return true;
        }
    }
    return false;
}

size_t getName(NameMap* map, char* name) {
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->names[i], name) == 0) {
            return i;
        }
    }
    return 0;
}

size_t addName(NameMap* map, char* name) {
    if (hasName(map, name)) {
        return 0;
    }
    char* newName = malloc(strlen(name) + 1);
    strcpy(newName, name);
    write_NameMap(map, newName);
    return map->count - 1;
}