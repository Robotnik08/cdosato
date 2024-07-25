#ifndef dosato_value_h
#define dosato_value_h

#include "common.h"

typedef enum {
    D_NULL = -1,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_LONG,
    TYPE_BYTE,
    TYPE_VOID,
    TYPE_ARRAY,
    TYPE_UINT,
    TYPE_USHORT,
    TYPE_ULONG,
    TYPE_UBYTE,
    TYPE_OBJECT,
    TYPE_VAR
} DataType;

typedef struct {
    DataType type;
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

void init_ValueArray(ValueArray* array);
void write_ValueArray(ValueArray* array, Value value);
void free_ValueArray(ValueArray* array);

#endif // dosato_value_h