#ifndef dosato_token_h
#define dosato_token_h

#include "common.h"

#define MASTER_KEYWORDS {"do", "make", "set", "define", "include", "import", "return", "break", "continue", "switch", "const", "class", "implement", "enum", "if", "inherit", "match"}
#define EXTENSION_KEYWORDS {"when", "while", "else", "catch", "then", "for", "unless", "until"}
#define VAR_TYPES {"int", "bool", "string", "float", "double", "char", "short", "long", "byte", "void", "array", "uint", "ushort", "ulong", "ubyte", "object", "var", "function"}
#define BOOLEAN_KEYWORDS {"false", "true"}
#define NULL_KEYWORDS {"null"}
#define INFINITY_KEYWORDS {"Infinity"}
#define NAN_KEYWORDS {"NaN"}
#define RESERVED_KEYWORDS {"other", "in"}

#define BRACKETS {"()", "{}", "[]"}
#define OPERATORS {"+", "-", "*", "/", "%", "=", ">", "<", "!", "&", "^", "|", "~", "?", ":", "->",",", "#",  \
                   "+=","-=","*=","/=","%=","++","--","==","!=",">=","<=","&&","||","<<",">>","&=","|=","^=", \
                   "**","^/",">|","<|","!-","=>",">>=","<<=","**=",">|=","<|=",";", ":>",":<",":>=",":<=","??",\
                   "?\?=","?->","^/=","|>", "===","!==","|>=","!?","!?="}
// operator precedence is mostly borrowed from C, lower means higher precedence
#define OPERATOR_PRECEDENCE \
                  { 4,   4,   3,   3,   3,   14,  6,   6,   2,   8,   9,   10,  2,   13,  13,  1,   15,  1,   \
                    14,  14,  14,  14,  14,  2,   2,   7,   7,   6,   6,   11,  12,  5,   5,   14,  14,  14,  \
                    2,   2,   2,   2,   2,   15,  14,   14,   14,   14,   14,   13,  13,  13,  13,   13,   12,\
                    14,    1,    14,   12,   7,    7,    14,   12,  14}
#define UNARY_PRECEDENCE 0

typedef enum {
    TOKEN_NULL = -2,
    TOKEN_COMMENT = 0,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_NUMBER_HEX,
    TOKEN_NUMBER_BINARY,
    TOKEN_NUMBER_OCTAL,
    TOKEN_OPERATOR,
    TOKEN_MASTER_KEYWORD,
    TOKEN_EXT,
    TOKEN_IDENTIFIER,
    TOKEN_PARENTHESIS_OPEN,
    TOKEN_PARENTHESIS_CLOSED,
    TOKEN_VAR_TYPE,
    TOKEN_BOOLEAN,
    TOKEN_NULL_KEYWORD,
    TOKEN_INFINITY_KEYWORD,
    TOKEN_NAN_KEYWORD,
    TOKEN_RESERVED_KEYWORD,
    TOKEN_TEMPLATE,
    TOKEN_TEMPLATE_END,
    TOKEN_END = -1
} LexTokenType;

typedef enum {
    BRACKET_NULL,
    BRACKET_ROUND = 1 << 13,
    BRACKET_SQUARE = 1 << 14,
    BRACKET_CURLY = 1 << 15,
} BracketType;

#define CHECK_BRACKET_TYPE(bracket, type) (bracket & type)

typedef enum {
    OPERATOR_NULL = -1,
    OPERATOR_ADD,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_MODULO,
    OPERATOR_ASSIGN,
    OPERATOR_GREATER,
    OPERATOR_LESS,
    OPERATOR_NOT,
    OPERATOR_AND,
    OPERATOR_XOR,
    OPERATOR_OR,
    OPERATOR_NOT_BITWISE,
    OPERATOR_QUESTION,
    OPERATOR_COLON,
    OPERATOR_ARROW,
    OPERATOR_COMMA,
    OPERATOR_HASH,
    OPERATOR_ADD_ASSIGN,
    OPERATOR_SUBTRACT_ASSIGN,
    OPERATOR_MULTIPLY_ASSIGN,
    OPERATOR_DIVIDE_ASSIGN,
    OPERATOR_MODULO_ASSIGN,
    OPERATOR_INCREMENT,
    OPERATOR_DECREMENT,
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_GREATER_EQUAL,
    OPERATOR_LESS_EQUAL,
    OPERATOR_AND_AND,
    OPERATOR_OR_OR,
    OPERATOR_SHIFT_LEFT,
    OPERATOR_SHIFT_RIGHT,
    OPERATOR_AND_ASSIGN,
    OPERATOR_OR_ASSIGN,
    OPERATOR_XOR_ASSIGN,
    OPERATOR_POWER,
    OPERATOR_ROOT,
    OPERATOR_MAX,
    OPERATOR_MIN,
    OPERATOR_ABSOLUTE,
    OPERATOR_FAT_ARROW,
    OPERATOR_SHIFT_RIGHT_ASSIGN,
    OPERATOR_SHIFT_LEFT_ASSIGN,
    OPERATOR_POWER_ASSIGN,
    OPERATOR_MAX_ASSIGN,
    OPERATOR_MIN_ASSIGN,
    OPERATOR_SEMICOLON,
    OPERATOR_RANGE_UP,
    OPERATOR_RANGE_DOWN,
    OPERATOR_RANGE_UP_INCLUSIVE,
    OPERATOR_RANGE_DOWN_INCLUSIVE,
    OPERATOR_NULL_COALESCE,
    OPERATOR_NULL_COALESCE_ASSIGN,
    OPERATOR_NULL_COALESCE_ACCESS,
    OPERATOR_ROOT_ASSIGN,
    OPERATOR_PIPE,
    OPERATOR_TRIPLE_EQUAL,
    OPERATOR_TRIPLE_NOT_EQUAL,
    OPERATOR_PIPE_ASSIGN,
    OPERATOR_FALSEY_COALESCE,
    OPERATOR_FALSEY_COALESCE_ASSIGN
} OperatorType;

typedef enum {
    EXT_WHEN,
    EXT_WHILE,
    EXT_ELSE,
    EXT_CATCH,
    EXT_THEN,
    EXT_FOR,
    EXT_UNLESS,
    EXT_UNTIL,
    
    EXT_IF,

    EXT_NULL = -1
} ExtensionKeywordType;

typedef enum {
    KEYWORD_OTHER,
    KEYWORD_IN
} ReservedKeywordType;

typedef struct {
    char* start;
    int length;
    LexTokenType type;
    int carry; // flags for token data
} Token;

typedef struct {
    size_t count;
    size_t capacity;
    Token* tokens;
} TokenList;

void init_TokenList (TokenList* list);
void write_TokenList (TokenList* list, Token token);
void free_TokenList (TokenList* list);

#define DOSATO_ADD_TOKEN(list, type, start, end, carry) do {\
    Token token = { start, end + 1, type, carry }; \
    write_TokenList(list, token); \
} while (0)


#define DOSATO_TOKEN_START (source, start) (start - source)

BracketType getBracketType (char bracket);

char* getTokenString (Token token);

void printTokens (TokenList list);

int isAssignmentOperator (OperatorType operator);

int checkIfOnly (TokenList list, LexTokenType type, int start, int end);

int isUnaryOperator (OperatorType operator);

#endif