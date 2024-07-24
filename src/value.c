#include "../include/common.h"

#include "../include/memory.h"
#include "../include/value.h"

void initValueArray(ValueArray* array) {
    array->count = 0;
    array->capacity = 0;
    array->values = NULL;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        size_t oldCapacity = array->capacity;
        array->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        array->values = DOSATO_RESIZE_LIST(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    DOSATO_FREE_LIST(Value, array->values, array->capacity);
}