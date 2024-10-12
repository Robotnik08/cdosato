#ifndef __DOSATO_H__
#define __DOSATO_H__

/**
 * This is the main header file for the Dosato library API.
 * Most of the functions and structures are defined here.
 * For more information on how to use the library, please refer to the documentation.
 * Some functions do require manual memory management, so be sure to attend to the documentation.
 */


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

#define GET_ARG(args, index) (args.values[args.count - index - 1])

#define CAST_SAFE(value, type) \
    do { \
        ErrorType cast_result_##value = castValue(&value, type); \
        if (cast_result_##value != 0) { \
            return BUILD_EXCEPTION(cast_result_##value); \
        } \
    } while (0)

#define CAST_TO_STRING(value) \
    do { \
        if (value.type == TYPE_STRING) break; \
        char* str = valueToString(value, false); \
        value = BUILD_STRING(str); \
    } while (0)

/// error.h
/// Error enum.

typedef enum {
    E_NULL,
    // token errors
    E_INVALID_NUMBER_LITERAL,
    E_INVALID_CHAR_LITERAL,
    E_UNCLOSED_STRING_LITERAL,
    E_TEMPLATE_RECURSION_LIMIT,

    // parser errors
    E_EXPECTED_MASTER,
    E_MISSING_SEPARATOR,
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
    E_EXPECTED_NUMBER,
    E_CANNOT_ASSIGN_TO_CONSTANT,

    // standard library errors
    E_FILE_NOT_FOUND,
    E_FILE_ALREADY_EXISTS,
    E_FILE_PERMISSION_DENIED,
    E_CANNOT_BE_ZERO,
    E_CANNOT_BE_NEGATIVE,

    // utility errors
    E_EMPTY_MESSAGE,

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

/**
 * @brief Prints the value to the console.
 * @param value The value to print.
 * @param extensive If true, prints the value in an extensive format. (e.g. strings are printed with quotes)
 */
extern void printValue(Value value, bool extensive);

/**
 * @brief Casts a value to a specific type, if possible. The value will be destroyed and replaced with the new value, so make sure you have a copy of the value if you need it.
 * @param value The value to cast.
 * @param type The type to cast to, must be a value type, if you want to cast to a string, use the CAST_TO_STRING macro.
 * @return An error code, 0 if successful. if you get an error code, make sure to handle it accordingly.
 */
extern ErrorType castValue(Value* value, DataType type);

/**
 * @brief Compares two values and returns true if they are equal.
 * This is based on the same logic as the == operator in the Language.
 */
extern bool valueEquals (Value* a, Value* b);

/**
 * @brief Increments a value by a specific amount.
 * @param value The value to increment.
 * @param amount The amount to increment by.
 * @return An error code, 0 if successful. if you get an error code, make sure to handle it accordingly.
 */
extern ErrorType incValue (Value* value, int amount);


/**
 * @brief Converts a value to a string.
 * @param value The value to convert.
 * @param extensive If true, prints the value in an extensive format. (e.g. strings are printed with quotes)
 * @return The string representation of the value. Make sure to free the string after you are done with it.
 */
extern char* valueToString (Value value, bool extensive);

/**
 * @brief Converts a DataType to a string.
 * @param type The DataType to convert.
 * @return The string representation of the DataType. This is a constant string, so you don't need to free it.
 */
extern char* dataTypeToString (DataType type);

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

/**
 * @brief Builds an array value from a list of values.
 * @param count The number of values to include in the array.
 * @param ... The values to include in the array, must be of type 'Value'.
 * @return The built array value.
 */
extern ValueArray* buildArray(size_t count, ...);

/**
 * @brief Builds an object value from a list of key-value pairs.
 * @param count The number of key-value pairs to include in the object.
 * @param ... The key-value pairs to include in the object, must be of type 'char*' and 'Value'. Make sure to include the key first, then the value. Take note that the key is copied internally, so you are still the owner of the key.
 * @return The built object value.
 */
extern ValueObject* buildObject(size_t count, ...);

extern void init_ValueArray(ValueArray* array);
extern void write_ValueArray(ValueArray* array, Value value);
extern void free_ValueArray(ValueArray* array);
extern void destroyValueArray(ValueArray* array);

extern void init_ValueObject(ValueObject* object);
extern void write_ValueObject(ValueObject* object, char* key, Value value);
extern void free_ValueObject(ValueObject* object);

/**
 * @brief Checks if an object has a key.
 * @param object The object to check.
 * @param key The key to check for.
 */
extern bool hasKey(ValueObject* object, char* key);

/**
 * @brief Gets the value at a specific key in an object.
 * @param object The object to get the value from.
 * @param key The key to get the value from.
 * @return The value at the key, or NULL if the key does not exist.
 */
extern Value* getValueAtKey(ValueObject* object, char* key);

/**
 * @brief Removes a key from an object, this does safely destroy the value and the key of the object.
 * @param object The object to remove the key from.
 * @param key The key to remove.
 */
extern void removeFromKey(ValueObject* object, char* key);

/// virtual-machine.h
/// This part of the library is responsible for handling virtual machine gc generation and running.

/**
 * @brief Don't call this function, use the BUILD_STRING, BUILD_ARRAY, BUILD_OBJECT macros instead.
 */
extern DosatoObject* buildDosatoObject(void* body, DataType type, bool sweep, void* vm);

#define BUILD_STRING(value) (Value){ TYPE_STRING, .as.objectValue = buildDosatoObject(value, TYPE_STRING, false, main_vm), .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_ARRAY(value) (Value){ TYPE_ARRAY, .as.objectValue = buildDosatoObject(value, TYPE_ARRAY, false, main_vm), .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_OBJECT(value) (Value){ TYPE_OBJECT, .as.objectValue = buildDosatoObject(value, TYPE_OBJECT, false, main_vm), .defined = false, .is_variable_type = false, .is_constant = false }

#define RETURN_STRING(value) (Value){ TYPE_STRING, .as.objectValue = buildDosatoObject(value, TYPE_STRING, true, main_vm), .defined = false, .is_variable_type = false, .is_constant = false }
#define RETURN_ARRAY(value) (Value){ TYPE_ARRAY, .as.objectValue = buildDosatoObject(value, TYPE_ARRAY, true, main_vm), .defined = false, .is_variable_type = false, .is_constant = false }
#define RETURN_OBJECT(value) (Value){ TYPE_OBJECT, .as.objectValue = buildDosatoObject(value, TYPE_OBJECT, true, main_vm), .defined = false, .is_variable_type = false, .is_constant = false }

/// dynamic_library_loader.h
/// This part of the library is responsible for loading dynamic libraries and functions from them.

typedef Value (*DosatoFunction)(ValueArray, bool debug);
typedef void (*DosatoInitFunction)(void*);

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

/**
 * @brief Gets the amount of characters in a file.
 * @param file The file to get the size of.
 * @return The amount of characters in the file, this is accurate based on the fact that on windows, \r\\n is counted as 1 character.
 */
extern long long int getFileSize(FILE *file);


/// library specific

/**
 * Prints the arguments (like printf, first is the format string, the rest are the arguments).
 * Based on the debug formatting used by dosato, meaning it'll stay consistent with the rest of the language.
 */
#define PRINT_ERROR(...) do { \
    printf("ERROR: \n"); \
    printf(__VA_ARGS__); \
} while(0)

/**
 * Copy string to the heap, ready for use in BUILD_STRING.
 */
#define COPY_STRING(str) strcpy(malloc(strlen(str) + 1), str)


#ifdef __cplusplus
}
#endif

#endif // __DOSATO_H__