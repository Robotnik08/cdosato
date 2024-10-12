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

char* getTokenEnumString (LexTokenType type) {
    switch (type) {
        case TOKEN_NULL:
            return "NULL";
        case TOKEN_COMMENT:
            return "COMMENT";
        case TOKEN_STRING:
            return "STRING";
        case TOKEN_NUMBER:
            return "NUMBER";
        case TOKEN_OPERATOR:
            return "OPERATOR";
        case TOKEN_MASTER_KEYWORD:
            return "MASTER_KEYWORD";
        case TOKEN_EXT:
            return "EXTENSION KEYWORD";
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN_PARENTHESIS_OPEN:
            return "PARENTHESIS_OPEN";
        case TOKEN_PARENTHESIS_CLOSED:
            return "PARENTHESIS_CLOSED";
        case TOKEN_VAR_TYPE:
            return "VAR_TYPE";
        case TOKEN_BOOLEAN:
            return "BOOLEAN";
        case TOKEN_NULL_KEYWORD:
            return "NULL_KEYWORD";
        case TOKEN_INFINITY_KEYWORD:
            return "INFINITY_KEYWORD";
        case TOKEN_NAN_KEYWORD:
            return "NAN_KEYWORD";
        case TOKEN_RESERVED_KEYWORD:
            return "RESERVED_KEYWORD";
        case TOKEN_TEMPLATE:
            return "TEMPLATE";
        case TOKEN_TEMPLATE_END:
            return "TEMPLATE_END";
        case TOKEN_END:
            return "END";
        default:
            return "UNKNOWN";
    }
}

void printTokens (TokenList list) {
    Token* tokens = list.tokens;
    if (list.count == 0) {
        printf("No tokens.\n");
        return;
    }
    char* offset = tokens[0].start; // first token start
    for (int i = 0; i < list.count; i++) {
        char* string = getTokenString(tokens[i]);
        char* type_string = getTokenEnumString(tokens[i].type);
        printf("Token %d: `%s`\t type: %s, carry: %d, char: %d->%d\n",
              i, string, type_string, tokens[i].carry, tokens[i].start - offset, tokens[i].start - offset + tokens[i].length);
        
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

int checkIfOnly (TokenList list, LexTokenType type, int start, int end) {
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
        case OPERATOR_NOT_BITWISE:
        case OPERATOR_ABSOLUTE:
        case OPERATOR_ROOT:
        case OPERATOR_MULTIPLY:
            return true;
        default:
            return false;
    }
}