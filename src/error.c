#include "../include/common.h"

#include "../include/error.h"

static const char* ERROR_MESSAGES[] = {
    "Invalid NULL error (report this)",

    "Invalid Number Literal",
    "Invalid Character Literal",
    "Unclosed String Literal",
    "Template Recursion Limit Exceeded",

    "Expected Master Keyword as first token (DO, MAKE, SET, DEFINE, INCLUDE, IMPORT, RETURN, BREAK, CONTINUE, SWITCH, CONST)",
    "Line is missing a seperator (;)",
    "Line is missing a closing parenthesis (')', '}', ']')",
    "Line is missing an opening parenthesis ('(', '{', '[')",
    "Unexpected Token",
    "Expected Identifier",
    "Expected Type Identifier (VAR, INT, FLOAT, STRING, BOOL, CHAR, ARRAY, OBJECT, BYTE, UBYTE, SHORT, USHORT, UINT, LONG, ULONG, DOUBLE, VOID, FUNCTION)",
    "Expected Assignment Operator (=, +=, -=, *=, /=, %=, ^=, |=, &=, **=, >>=, <<=, <|=, >|=, ^/=, ?\?=, ++, --)",
    "Expected Pure Assignment Operator (=)",
    "Expected Round Bracket '()'",
    "Expected Curly Bracket '{}'",
    "Expected Square Bracket '[]'",
    "Expression can not be empty",
    "Operator cannot be used as unary operator (-, ~, !, *, !-, ^/)",
    "Incomplete Ternary Operator, expected ':'",
    "Operator cannot be used as binary operator (+, -, *, /, %, =, >, <, &&, ||, &, ^, |, <<, >>, ??, **, ^/, <|, >|, :>, :<, :>=, :<=, #, ->, ?->)",
    "Expected Hash Operator (#)",
    "Invalid Expression",
    "Expected Colon Operator (':')",
    "Invalid Type",
    "Master keyword must be global (DEFINE, IMPORT, INCLUDE)",
    "Can't return outside of function",
    "Master keyword can't have extensions (MAKE, DEFINE, IMPORT, INCLUDE, CONST)",
    "Else keyword must come directly after an if or when keyword",
    "If keyword expects Then keyword",
    "Break keyword can only be used inside a loop",
    "Continue keyword can only be used inside a loop",
    "Expected String Literal",
    "Too many includes (Might be a circular include)",	

    "Undefined Variable",
    "Variable already defined",
    "Variable is not an array",
    "Variable is not an object",
    "Index is out of bounds",
    "Invalid key type, expected STRING",
    "Key is already defined",
    "Key is not defined",
    "Cannot convert type to integer",
    "Cannot convert type to float",
    "Cannot convert type to object",
    "Cannot convert type to array",
    "Cannot perform operation",
    "Cannot perform binary operation",
    "Cannot perform unary operation",
    "Math domain error",
    "Not a function",
    "Wrong number of arguments in function call",
    "Expected STRING type",
    "Expected number",
    "Cannot reassign constant",
    "Mismatch in tuple expression",

    "File not found",
    "File already exists",
    "Permission denied",
    "Value cannot be zero",
    "Value cannot be negative",

    // utility errors
    "Empty error", // this is for libraries to print their own error messages

    "Unknown Error",
    "Error Amount (report this)"
};

void printError (const char* full_code, const int pos, const char* file_name, const ErrorType type, const int token_size) {
    if (type != E_EMPTY_MESSAGE) {
        printf("\nERROR:\n");
        printf("E%d: %s\n", type, ERROR_MESSAGES[type < E_AMOUNT && E_AMOUNT > 0 ? type : E_UNKNOWN]);
    }

    if (pos < strlen(full_code) && pos >= 0) {
        printf("At line %i:%i in <%s>\n", getLine(full_code, pos), getLineCol(full_code, pos), file_name);
    } else {
        printf("At UNSPECIFIED position\n");
    }

    // print full line
    int start = pos;
    int end = pos;
    while (start > 0 && full_code[start - 1] != '\n') {
        start--;
    }

    while (end < strlen(full_code) && full_code[end] != '\n') {
        end++;
    }

    printf("\n%.*s\n", end - start, full_code + start);

    // print arrow to position
    for (int i = start; i < pos; i++) {
        printf(" ");
    }

    for (int i = pos; i < pos + token_size; i++) {
        printf("^");
    }

    printf("\n");

    #ifdef _WIN32
    _fcloseall(); // close all files if any were opened
    #endif
    exit(type);
}