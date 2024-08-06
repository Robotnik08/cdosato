#include "../include/common.h"

#include "../include/error.h"

static const char* ERROR_MESSAGES[] = {
    "Invalid NULL error (report this)",

    "Invalid Number Literal",

    "Expected Master Keyword as first token (DO, MAKE, SET, DEFINE, INCLUDE, IMPORT, RETURN, BREAK, CONTINUE)",
    "Line is missing a seperator (;)",
    "Line is missing a closing parenthesis (')', '}', ']')",
    "Line is missing an opening parenthesis ('(', '{', '[')",
    "Unexpected Token",
    "Expected Identifier",
    "Expected Type Identifier (VAR, INT, FLOAT, STRING, BOOL, CHAR, ARRAY, OBJECT, BYTE, UBYTE, SHORT, USHORT, UINT, LONG, ULONG, DOUBLE, VOID)",
    "Expected Assignment Operator (=, +=, -=, *=, /=, %=, ^=, |=, &=, **=, >>=, <<=, <|=, |>=, ++, --)",
    "Expected Assignment Operator (=)",
    "Expected Round Bracket '()'",
    "Expected Curly Bracket '{}'",
    "Expected Square Bracket '[]'",
    "Expression can not be empty",
    "Operator cannot be used as unary operator (+, -, ~, !, !-, ^/)",
    "Incomplete Ternary Operator, expected ':'",

    "Unknown Error",
    "Error Amount (report this)"
};

void printError (const char* full_code, const int pos, const ErrorType type) {
    printf("\nERROR:\n");
    printf("E%d: %s\n", type, ERROR_MESSAGES[type < E_AMOUNT && E_AMOUNT > 0 ? type : E_UNKNOWN]);

    if (pos < strlen(full_code)) {
        printf("At line %i:%i\n", getLine(full_code, pos), getLineCol(full_code, pos));
    } else {
        printf("At UNSPECIFIED position\n");
    }
    #ifdef _WIN32
    _fcloseall(); // close all files if any were opened
    #endif
    exit(type);
}