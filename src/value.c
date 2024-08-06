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