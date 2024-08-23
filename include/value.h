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

#define ISINTTYPE(type) (type == TYPE_INT || type == TYPE_SHORT || type == TYPE_LONG || type == TYPE_BYTE || type == TYPE_UINT || type == TYPE_USHORT || type == TYPE_ULONG || type == TYPE_UBYTE)
#define ISFLOATTYPE(type) (type == TYPE_FLOAT || type == TYPE_DOUBLE)


typedef struct {
    DataType type;
    bool is_variable_type; // non strict type
    bool defined;
    union {
        char byteValue;
        unsigned char ubyteValue;
        short shortValue;
        unsigned short ushortValue;
        int intValue;
        unsigned int uintValue;
        long long longValue;
        unsigned long long ulongValue;
        float floatValue;
        double doubleValue;
        char boolValue;
        char charValue;
        char* stringValue;
        void* objectValue;
    } as;
} Value;

#define UNDEFINED_VALUE (Value){ D_NULL, .defined = false }

void destroyValue(Value* value);
void printValue(Value value, bool extensive);
void markDefined(Value* value);
Value hardCopyValue(Value value);
ErrorType castValue(Value* value, DataType type);
ErrorType incValue (Value* value, int amount);

char* valueToString (Value value, bool extensive);

typedef struct {
    size_t count;
    size_t capacity;
    Value* values;
} ValueArray;

typedef struct {
    size_t count;
    size_t capacity;
    Value* values;
    char** keys;
} ValueObject;

typedef struct {
    size_t count;
    size_t capacity;
    size_t* stack;
} StackFrames;

typedef struct {
    char** names;
    size_t count;
    size_t capacity;
} NameMap;

void init_ValueArray(ValueArray* array);
void write_ValueArray(ValueArray* array, Value value);
void free_ValueArray(ValueArray* array);
void destroyValueArray(ValueArray* array);

void init_ValueObject(ValueObject* object);
void write_ValueObject(ValueObject* object, char* key, Value value);
void free_ValueObject(ValueObject* object);
bool hasKey(ValueObject* object, char* key);
Value* getValueAtKey(ValueObject* object, char* key);
void removeFromKey(ValueObject* object, char* key);

void init_StackFrames(StackFrames* stack);
void write_StackFrames(StackFrames* stack, size_t frame);
void free_StackFrames(StackFrames* stack);

void init_NameMap(NameMap* map);
void write_NameMap(NameMap* map, char* name);
void free_NameMap(NameMap* map);
bool hasName(NameMap* map, char* name);
size_t getName(NameMap* map, char* name);
size_t addName(NameMap* map, char* name);

#endif // dosato_value_h