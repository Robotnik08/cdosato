#ifndef dosato_error_h
#define dosato_error_h

#include "common.h"

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

    // standard library errors
    E_FILE_NOT_FOUND,
    E_FILE_ALREADY_EXISTS,
    E_FILE_PERMISSION_DENIED,

    E_UNKNOWN,
    E_AMOUNT
} ErrorType;

// extern const char* ERROR_MESSAGES[];

void printError (const char* full_code, const int pos, const char* file_name, const ErrorType type);

#endif // dosato_error_h