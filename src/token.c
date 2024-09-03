#include "../include/common.h"

#include "../include/memory.h"
#include "../include/token.h"

DOSATO_LIST_FUNC_GEN(TokenList, Token, tokens)


BracketType getBracketType (char bracket) {
    switch (bracket) {
        case '(':
        case ')':
            return BRACKET_ROUND;
        case '[':
        case ']':
            return BRACKET_SQUARE;
        case '{':
        case '}':
            return BRACKET_CURLY;
        default:
            return BRACKET_NULL;
    }
}

char* getTokenString (Token token) {
    char* string = malloc(token.length + 1);
    for (int i = 0; i < token.length; i++) {
        string[i] = token.start[i];
    }
    string[token.length] = '\0';
    return string;
}

void printTokens (TokenList list) {
    return;
    Token* tokens = list.tokens;
    if (list.count == 0) {
        printf("No tokens.\n");
        return;
    }
    char* offset = tokens[0].start; // first token start
    for (int i = 0; i < list.count; i++) {
        char* string = getTokenString(tokens[i]);
        printf("Token %d: '%s' start: %d, end: %d, type: %d, carry: %d\n",
              i, string,  tokens[i].start - offset, tokens[i].start - offset + tokens[i].length, tokens[i].type, tokens[i].carry);
        
        free(string);
    }
}

int isAssignmentOperator (OperatorType operator) {
    switch (operator) {
        case OPERATOR_ASSIGN:
        case OPERATOR_ADD_ASSIGN:
        case OPERATOR_SUBTRACT_ASSIGN:
        case OPERATOR_MULTIPLY_ASSIGN:
        case OPERATOR_DIVIDE_ASSIGN:
        case OPERATOR_MODULO_ASSIGN:
        case OPERATOR_AND_ASSIGN:
        case OPERATOR_OR_ASSIGN:
        case OPERATOR_XOR_ASSIGN:
        case OPERATOR_SHIFT_LEFT_ASSIGN:
        case OPERATOR_SHIFT_RIGHT_ASSIGN:
        case OPERATOR_POWER_ASSIGN:
        case OPERATOR_MIN_ASSIGN:
        case OPERATOR_MAX_ASSIGN:
        case OPERATOR_INCREMENT:
        case OPERATOR_DECREMENT:
            return true;
        default:
            return false;
    }
}

int checkIfOnly (TokenList list, TokenType type, int start, int end) {
    Token* tokens = list.tokens;
    if (end - start < 2) {
        return false;
    }
    for (int i = start + 1; i < end; i++) {
        if (tokens[i].type != type) {
            return false;
        }
    }
    return true;
}

int isUnaryOperator (OperatorType operator) {
    switch (operator) {
        case OPERATOR_NOT:
        case OPERATOR_SUBTRACT:
        case OPERATOR_ADD:
        case OPERATOR_NOT_BITWISE:
        case OPERATOR_ABSOLUTE:
        case OPERATOR_ROOT:
            return true;
        default:
            return false;
    }
}