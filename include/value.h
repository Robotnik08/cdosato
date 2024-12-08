#ifndef dosato_value_h
#define dosato_value_h

#include "common.h"

typedef enum {
    D_NULL = -1,
    TYPE_EXCEPTION = -2,
    TYPE_HLT = -3,

    TYPE_INT = 0,
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
    TYPE_VAR,
    TYPE_FUNCTION
} DataType;

#define ISINTTYPE(type) (type == TYPE_INT || type == TYPE_SHORT || type == TYPE_LONG || type == TYPE_BYTE || type == TYPE_UINT || type == TYPE_USHORT || type == TYPE_ULONG || type == TYPE_UBYTE)
#define ISFLOATTYPE(type) (type == TYPE_FLOAT || type == TYPE_DOUBLE)


typedef struct {
    void* body;
    bool marked;
    DataType type;
} DosatoObject;

typedef struct {
    DataType type;
    bool is_variable_type; // non strict type
    bool defined;
    bool is_constant;
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
        DosatoObject* objectValue;
    } as;
} Value;

#define AS_STRING(value) ((char*)(value).as.objectValue->body)
#define AS_ARRAY(value) ((ValueArray*)(value).as.objectValue->body)
#define AS_OBJECT(value) ((ValueObject*)(value).as.objectValue->body)
#define AS_FUNCTION(value) ((Function*)(value).as.objectValue->body)

#define UNDEFINED_VALUE (Value){ D_NULL, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_EXCEPTION(e_code) (Value){ TYPE_EXCEPTION, .as.longValue = e_code, .is_variable_type = false, .defined = true, .is_constant = false }
#define BUILD_HLT(exit_code) (Value){ TYPE_HLT, .as.longValue = exit_code, .is_variable_type = false, .defined = true, .is_constant = false }
#define BUILD_NULL() (Value){ D_NULL, .defined = false, .is_variable_type = false, .is_constant = false }

#define BUILD_BYTE(value) (Value){ TYPE_BYTE, .as.byteValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_UBYTE(value) (Value){ TYPE_UBYTE, .as.ubyteValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_SHORT(value) (Value){ TYPE_SHORT, .as.shortValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_USHORT(value) (Value){ TYPE_USHORT, .as.ushortValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_INT(value) (Value){ TYPE_INT, .as.intValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_UINT(value) (Value){ TYPE_UINT, .as.uintValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_LONG(value) (Value){ TYPE_LONG, .as.longValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_ULONG(value) (Value){ TYPE_ULONG, .as.ulongValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_FLOAT(value) (Value){ TYPE_FLOAT, .as.floatValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_DOUBLE(value) (Value){ TYPE_DOUBLE, .as.doubleValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_BOOL(value) (Value){ TYPE_BOOL, .as.boolValue = value, .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_CHAR(value) (Value){ TYPE_CHAR, .as.charValue = value, .defined = false, .is_variable_type = false, .is_constant = false }

#define AS_BYTE(value) ((value).as.byteValue)
#define AS_UBYTE(value) ((value).as.ubyteValue)
#define AS_SHORT(value) ((value).as.shortValue)
#define AS_USHORT(value) ((value).as.ushortValue)
#define AS_INT(value) ((value).as.intValue)
#define AS_UINT(value) ((value).as.uintValue)
#define AS_LONG(value) ((value).as.longValue)
#define AS_ULONG(value) ((value).as.ulongValue)
#define AS_FLOAT(value) ((value).as.floatValue)
#define AS_DOUBLE(value) ((value).as.doubleValue)
#define AS_BOOL(value) ((value).as.boolValue)
#define AS_CHAR(value) ((value).as.charValue)


void destroyValue(Value* value);
void printValue(Value value, bool extensive);
void markDefined(Value* value);
Value hardCopyValue(Value value);
Value hardCopyValueSafe (Value value, DosatoObject*** pointers, int count);
ErrorType castValue(Value* value, DataType type);
bool valueEquals (Value* a, Value* b);
ErrorType incValue (Value* value, int amount);


char* valueToString (Value value, bool extensive);
char* valueToStringSafe (Value value, bool extensive, DosatoObject*** pointers, int count);
char* dataTypeToString (DataType type);

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
    uint8_t* error_jump_loc;
    size_t error_stack_count;
    size_t error_active_index_count;
    size_t error_stack_frame_count;
} ErrorJump;

typedef struct {
    size_t count;
    size_t capacity;
    ErrorJump* jumps;
} ErrorJumps;


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

ValueArray* buildArray(size_t count, ...);
ValueObject* buildObject(size_t count, ...);

void init_StackFrames(StackFrames* stack);
void write_StackFrames(StackFrames* stack, size_t frame);
void free_StackFrames(StackFrames* stack);

void init_ErrorJumps(ErrorJumps* jumps);
void write_ErrorJumps(ErrorJumps* jumps, ErrorJump jump);
void free_ErrorJumps(ErrorJumps* jumps);

void init_NameMap(NameMap* map);
void write_NameMap(NameMap* map, char* name);
void free_NameMap(NameMap* map);
bool hasName(NameMap* map, char* name);
size_t getName(NameMap* map, char* name);
size_t addName(NameMap* map, char* name);

#endif // dosato_value_h