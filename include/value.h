#ifndef dosato_value_h
#define dosato_value_h

#include "common.h"

typedef enum {
    TYPE_INT,
    TYPE_STRING
} ValueType;

typedef struct {
    ValueType type;
    union {
        int intValue;
        char* stringValue;
    } as;
} Value;

typedef struct {
    size_t count;
    size_t capacity;
    Value* values;
} ValueArray;

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);

#endif // dosato_value_h