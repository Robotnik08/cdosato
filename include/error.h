#ifndef dosato_error_h
#define dosato_error_h

#include "common.h"

typedef enum {
    E_NULL,
    // token errors
    E_INVALID_NUMBER_LITERAL,
    E_INVALID_CHAR_LITERAL,

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

    // runtime errors
    E_UNDEFINED_VARIABLE,
    E_ALREADY_DEFINED_VARIABLE,
    E_NOT_AN_ARRAY,
    E_INDEX_OUT_OF_BOUNDS,

    E_UNKNOWN,
    E_AMOUNT
} ErrorType;

// extern const char* ERROR_MESSAGES[];

void printError (const char* full_code, const int pos, const ErrorType type);

#endif // dosato_error_h