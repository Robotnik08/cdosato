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
    E_EXPECTED_LONG,

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
    TYPE_FUNCTION
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

#define UNDEFINED_VALUE (Value){ D_NULL, .defined = false, .is_variable_type = false, 0 }
#define BUILD_EXCEPTION(e_code) (Value){ TYPE_EXCEPTION, .as.longValue = e_code, .is_variable_type = false, .defined = true }
#define BUILD_HLT(exit_code) (Value){ TYPE_HLT, .as.longValue = exit_code, .is_variable_type = false, .defined = true }

/**
 * @brief Destroys a value and frees the entire object safely.
 * The value may not be used after this function is called.
 */
extern void destroyValue(Value* value);

/**
 * @brief Prints the value to the console.
 * @param value The value to print.
 * @param extensive If true, prints the value in an extensive format. (e.g. strings are printed with quotes)
 */
extern void printValue(Value value, bool extensive);

/**
 * @brief Returns a hard copy of the value, this means you will have to destroy the value after you are done with it.
 */
extern Value hardCopyValue(Value value);

/**
 * @brief Casts a value to a specific type, if possible. The value will be destroyed and replaced with the new value, so make sure you have a copy of the value if you need it.
 * @param value The value to cast.
 * @param type The type to cast to.
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
 * @brief Builds an array value from a list of values.
 * @param count The number of values to include in the array.
 * @param ... The values to include in the array, must be of type 'Value'.
 * @return The built array value.
 */
extern Value buildArray(size_t count, ...);

/**
 * @brief Builds an object value from a list of key-value pairs.
 * @param count The number of key-value pairs to include in the object.
 * @param ... The key-value pairs to include in the object, must be of type 'char*' and 'Value'. Make sure to include the key first, then the value. Take note that the key is copied internally, so you are still the owner of the key.
 * @return The built object value.
 */
extern Value buildObject(size_t count, ...);

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

/**
 * @brief Gets the amount of characters in a file.
 * @param file The file to get the size of.
 * @return The amount of characters in the file, this is accurate based on the fact that on windows, \r\\n is counted as 1 character.
 */
extern long long int getFileSize(FILE *file);



#ifdef __cplusplus
}
#endif

#endif // __DOSATO_H__