#include "../include/common.h"

#include "../include/error.h"

static const char* ERROR_MESSAGES[] = {
    "Invalid NULL error (report this)",

    "Invalid Number Literal",
    "Invalid Character Literal",
    "Unclosed String Literal",
    "Template Recursion Limit Exceeded",

    "Expected Master Keyword as first token (do, make, set, define, include, import, return, break, continue, switch, const, class, implement, enum, if, inherit)",
    "Line is missing a closing parenthesis (')', '}', ']')",
    "Line is missing an opening parenthesis ('(', '{', '[')",
    "Unexpected Token",
    "Expected Identifier",
    "Expected Type Identifier (var, int, float, string, bool, char, array, object, byte, ubyte, short, ushort, uint, long, ulong, double, void, function)",
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
    "Master keyword must be global (define, import, include, class, enum)",
    "Can't return outside of function",
    "Master keyword can't have extensions (make, define, import, include, const, class, implement, enum, inherit)",
    "Else keyword must come directly after an if or when keyword",
    "If keyword expects Then keyword",
    "Break keyword can only be used inside a loop",
    "Continue keyword can only be used inside a loop",
    "Expected String Literal",
    "Too many includes (You might have a circular include)",
    "Default arguments must be last in function call",

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
    "Identifier cannot be used in this context",
    "Value is not iterable",
    "Master keyword cannot be used outside of a class definition",

    "File not found",
    "File already exists",
    "Permission denied",
    "Value cannot be zero",
    "Value cannot be negative",
    "Directory must be empty",

    // utility errors
    "Empty error", // this is for libraries to print their own error messages

    "Unknown Error",
    "Error Amount (report this)"
};

void printError (const char* full_code, const int pos, const char* file_name, const ErrorType type, const int token_size) {
    if (type != E_EMPTY_MESSAGE) {
        printf("%s", "\nERROR:\n");
        printf("E%d: %s\n", type, ERROR_MESSAGES[type < E_AMOUNT && E_AMOUNT > 0 ? type : E_UNKNOWN]);
    }

    if (pos < strlen(full_code) && pos >= 0) {
        printf("At line %i:%i in <%s>\n", getLine(full_code, pos), getLineCol(full_code, pos), file_name);
    } else {
        printf("%s", "At UNSPECIFIED position\n");
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
        printf("%s", " ");
    }

    for (int i = pos; i < pos + token_size; i++) {
        printf("%s", "^");
    }

    printf("%s", "\n");

    #ifdef _WIN32
    _fcloseall(); // close all files if any were opened
    #endif
    exit(type);
}