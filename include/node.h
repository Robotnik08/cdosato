#ifndef dosato_node_h
#define dosato_node_h

#include "common.h"
#include "token.h"

typedef enum {
    NODE_PROGRAM, // root node

    NODE_MASTER_DO,
    NODE_MASTER_MAKE,
    NODE_MASTER_SET,
    NODE_MASTER_DEFINE,
    NODE_MASTER_INCLUDE,
    NODE_MASTER_IMPORT,
    NODE_MASTER_RETURN,
    NODE_MASTER_BREAK,
    NODE_MASTER_CONTINUE,
    NODE_MASTER_SWITCH,
    NODE_MASTER_CONST,
    NODE_MASTER_CLASS,
    NODE_MASTER_IMPLEMENT,
    NODE_MASTER_ENUM,

    NODE_MASTER_DO_BODY,
    NODE_MASTER_MAKE_BODY,
    NODE_MASTER_SET_BODY,
    NODE_MASTER_DEFINE_BODY,
    NODE_MASTER_INCLUDE_BODY,
    NODE_MASTER_IMPORT_BODY,
    NODE_MASTER_RETURN_BODY,
    NODE_MASTER_BREAK_BODY,
    NODE_MASTER_CONTINUE_BODY,
    NODE_MASTER_SWITCH_BODY,
    NODE_MASTER_CONST_BODY,
    NODE_MASTER_CLASS_BODY,
    NODE_MASTER_IMPLEMENT_BODY,
    NODE_MASTER_ENUM_BODY,
    

    NODE_WHEN_BODY,
    NODE_WHILE_BODY,
    NODE_ELSE_BODY,
    NODE_CATCH_BODY,
    NODE_THEN_BODY,
    NODE_FOR_BODY,
    NODE_IF_BODY,

    NODE_BLOCK,
    NODE_EXPRESSION,
    NODE_SUBSCRIPT_EXPRESSION,
    NODE_TEMP_SUBSCRIPT_EXPRESSION,
    NODE_UNARY_EXPRESSION,
    NODE_ARRAY_EXPRESSION,
    NODE_OBJECT_EXPRESSION,
    NODE_TERNARY_EXPRESSION,
    NODE_FUNCTION_CALL,
    NODE_TYPE,
    NODE_OPERATOR,
    NODE_IDENTIFIER,
    NODE_TRUE,
    NODE_FALSE,
    NODE_NULL_KEYWORD,
    NODE_INFINITY_KEYWORD,
    NODE_NAN_KEYWORD,
    NODE_NUMBER_LITERAL,
    NODE_STRING_LITERAL,
    NODE_FUNCTION_DEFINITION_PARAMETERS,
    NODE_FUNCTION_DEFINITION_ARGUMENT,
    NODE_ARGUMENTS,
    NODE_OBJECT_ENTRY,
    NODE_TYPE_CAST,
    NODE_SWITCH_BODY,
    NODE_SWITCH_CASE,
    NODE_OTHER,
    NODE_TEMPLATE_LITERAL,
    NODE_TEMPLATE_STRING_PART,
    NODE_LAMBDA_EXPRESSION,
    NODE_ENUM_BODY,
    NODE_ENUM_EXPRESSION
} NodeType;

typedef struct Node Node;

typedef struct {
    size_t count;
    size_t capacity;
    Node* nodes;
} NodeList;

struct Node {
    int start, end;
    NodeList body;
    NodeType type;
};


void init_NodeList (NodeList* list);
void write_NodeList (NodeList* list, Node node);
void free_NodeList (NodeList* list);

void destroyNodeList (NodeList* list);

void printNode (Node node, int depth);

int getEndOfLine (TokenList tokens, int start);
int getEndOfBlock (TokenList tokens, int start);
int getStartOfBlock (TokenList tokens, int start);

#endif