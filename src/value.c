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
    if (value.array_depth > 0) {
        ValueArray* array = value.as.objectValue;
        destroyValueArray(array);
        free(array);
        return;
    }

    if (value.type == TYPE_STRING) {
        free(value.as.stringValue);
    }
}

void printValue(Value value, bool extensive) {
    if (value.array_depth > 0) {
        printf("[");
        ValueArray* array = value.as.objectValue;
        for (size_t i = 0; i < array->count; i++) {
            printValue(array->values[i], true);
            if (i < array->count - 1) {
                printf(", ");
            }
        }
        printf("]");
        return;
    }

    switch (value.type) {
        case TYPE_UBYTE: {
            printf("%u", value.as.ubyteValue);
            break;
        }
        case TYPE_BYTE: {
            printf("%d", value.as.byteValue);
            break;
        }
        case TYPE_USHORT: {
            printf("%u", value.as.ushortValue);
            break;
        }
        case TYPE_SHORT: {
            printf("%d", value.as.shortValue);
            break;
        }
        case TYPE_UINT: {
            printf("%u", value.as.uintValue);
            break;
        }
        case TYPE_INT: {
            printf("%d", value.as.intValue);
            break;
        }
        case TYPE_ULONG: {
            printf("%llu", value.as.ulongValue);
            break;
        }
        case TYPE_LONG: {
            printf("%lld", value.as.longValue);
            break;
        }
        case TYPE_STRING: {
            if (extensive) {
                printf("\"%s\"", value.as.stringValue);
            } else {
                printf("%s", value.as.stringValue);
            }
            break;
        }
        case TYPE_CHAR: {
            if (extensive) {
                printf("'%c'", value.as.charValue);
            } else {
                printf("%c", value.as.charValue);
            }
            break;
        }
        case TYPE_FLOAT: {
            printf("%f", value.as.floatValue);
            break;
        }
        case TYPE_DOUBLE: {
            printf("%f", value.as.doubleValue);
            break;
        }
        case TYPE_BOOL: {
            printf("%s", value.as.boolValue ? "TRUE" : "FALSE");
            break;
        }
        default: {
            printf("Unknown type\n");
            break;
        }
    }
}

Value hardCopyValue(Value value) {
    Value copy = value;
    copy.defined = false;
    if (value.array_depth > 0) {
        ValueArray* array = value.as.objectValue;
        ValueArray* newArray = malloc(sizeof(ValueArray));
        init_ValueArray(newArray);
        for (size_t i = 0; i < array->count; i++) {
            write_ValueArray(newArray, hardCopyValue(array->values[i]));
        }
        copy.as.objectValue = newArray;
    } else if (value.type == TYPE_STRING) {
        char* newString = malloc(strlen(value.as.stringValue) + 1);
        strcpy(newString, value.as.stringValue);
        copy.as.stringValue = newString;
    }
    return copy;
}

void markDefined(Value* value) {
    value->defined = true;
    if (value->array_depth > 0) {
        ValueArray* array = value->as.objectValue;
        for (size_t i = 0; i < array->count; i++) {
            markDefined(&array->values[i]);
        }
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