#ifndef __DOSATO_H__
#define __DOSATO_H__

// This is the main header file for the Dosato library API.

#ifdef __cplusplus
extern "C" {
#endif

#define DOSATO_VERSION "0.2.0"

/// Include the common header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

/// error.h
/// Error enum.

typedef enum {
    E_NULL,
    // token errors
    E_INVALID_NUMBER_LITERAL,
    E_INVALID_CHAR_LITERAL,
    E_UNCLOSED_STRING_LITERAL,

    // parser errors
    E_EXPECTED_MASTER,
    E_MISSING_SEPERATOR,
    E_MISSING_CLOSING_PARENTHESIS,
    E_MISSING_OPENING_PARENTHESIS,
    E_UNEXPECTED_TOKEN,
    E_EXPECTED_IDENTIFIER,
    E_EXPECTED_TYPE_INDENTIFIER,
    E_EXPECTED_ASSIGNMENT_OPERATOR,
    E_EXPECTED_ASSIGNMENT_OPERATOR_PURE,
    E_EXPECTED_BRACKET_ROUND,
    E_EXPECTED_BRACKET_CURLY,
    E_EXPECTED_BRACKET_SQUARE,
    E_EMPTY_EXPRESSION,
    E_NON_UNARY_OPERATOR,
    E_INCOMPLETE_TERNARY_OPERATOR,
    E_NON_BINARY_OPERATOR,
    E_EXPECTED_HASH_OPERATOR,
    E_INVALID_EXPRESSION,
    E_EXPECTED_COLON_OPERATOR,
    E_INVALID_TYPE,
    E_MUST_BE_GLOBAL,
    E_CANT_RETURN_OUTSIDE_FUNCTION,
    E_MASTER_CANT_HAVE_EXTENSIONS,
    E_ELSE_WITHOUT_IF,
    E_EXPECTED_THEN,
    E_BREAK_OUTSIDE_LOOP,
    E_CONTINUE_OUTSIDE_LOOP,
    E_EXPECTED_STRING,
    E_TOO_MANY_INCLUDES,

    // runtime errors
    E_UNDEFINED_VARIABLE,
    E_ALREADY_DEFINED_VARIABLE,
    E_NOT_AN_ARRAY,
    E_NOT_AN_OBJECT,
    E_INDEX_OUT_OF_BOUNDS,
    E_INVALID_KEY_TYPE,
    E_KEY_ALREADY_DEFINED,
    E_KEY_NOT_FOUND,
    E_CANT_CONVERT_TO_INT,
    E_CANT_CONVERT_TO_FLOAT,
    E_CANT_CONVERT_TO_OBJECT,
    E_CANT_CONVERT_TO_ARRAY,
    E_CANT_PERFORM_OPERATION,
    E_CANT_PERFORM_BINARY_OPERATION,
    E_CANT_PERFORM_UNARY_OPERATION,
    E_MATH_DOMAIN_ERROR,
    E_NOT_A_FUNCTION,
    E_WRONG_NUMBER_OF_ARGUMENTS,
    E_EXPECTED_STRING_TYPE,

    // standard library errors
    E_FILE_NOT_FOUND,
    E_FILE_ALREADY_EXISTS,
    E_FILE_PERMISSION_DENIED,

    E_UNKNOWN,
    E_AMOUNT
} ErrorType;



/// value.h
/// This part of the library is responsible for handling the values and types of the language.


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
    TYPE_FUNCTION,
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

#define UNDEFINED_VALUE (Value){ D_NULL, .defined = false, .is_variable_type = false }
#define BUILD_EXCEPTION(e_code) (Value){ TYPE_EXCEPTION, .as.longValue = e_code, .is_variable_type = false, .defined = true }
#define BUILD_HLT(exit_code) (Value){ TYPE_HLT, .as.longValue = exit_code, .is_variable_type = false, .defined = true }

extern void destroyValue(Value* value);
extern void printValue(Value value, bool extensive);
extern Value hardCopyValue(Value value);
extern ErrorType castValue(Value* value, DataType type);
extern bool valueEquals (Value* a, Value* b);
extern ErrorType incValue (Value* value, int amount);

extern char* valueToString (Value value, bool extensive);

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

extern void init_ValueArray(ValueArray* array);
extern void write_ValueArray(ValueArray* array, Value value);
extern void free_ValueArray(ValueArray* array);
extern void destroyValueArray(ValueArray* array);

extern void init_ValueObject(ValueObject* object);
extern void write_ValueObject(ValueObject* object, char* key, Value value);
extern void free_ValueObject(ValueObject* object);
extern bool hasKey(ValueObject* object, char* key);
extern Value* getValueAtKey(ValueObject* object, char* key);
extern void removeFromKey(ValueObject* object, char* key);


/// dynamic_library_loader.h
/// This part of the library is responsible for loading dynamic libraries and functions from them.

typedef Value (*DosatoFunction)(ValueArray, bool debug);
typedef void (*DosatoFunctionEmpty)();

typedef struct {
    char* name;
    DosatoFunction function;
} DosatoFunctionMap;

typedef struct {
    DosatoFunctionMap* functions;
    size_t count;
    size_t capacity;
} DosatoFunctionMapList;

extern void init_DosatoFunctionMapList(DosatoFunctionMapList* list);
extern void write_DosatoFunctionMapList(DosatoFunctionMapList* list, DosatoFunctionMap map);
extern void free_DosatoFunctionMapList(DosatoFunctionMapList* list);

/// filetools.h
extern long long int getFileSize(FILE *file);



#ifdef __cplusplus
}
#endif

#endif // __DOSATO_H__