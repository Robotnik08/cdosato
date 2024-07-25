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

    NODE_MASTER_DO_BODY,
    NODE_MASTER_MAKE_BODY,
    NODE_MASTER_SET_BODY,
    NODE_MASTER_DEFINE_BODY,
    NODE_MASTER_INCLUDE_BODY,
    NODE_MASTER_IMPORT_BODY,
    NODE_MASTER_RETURN_BODY,
    NODE_MASTER_BREAK_BODY,
    NODE_MASTER_CONTINUE_BODY,
    

    NODE_WHEN_BODY,
    NODE_WHILE_BODY,
    NODE_ELSE_BODY,
    NODE_CATCH_BODY,
    NODE_THEN_BODY,
    NODE_FOR_BODY,
    NODE_IF_BODY,

    NODE_BLOCK,
    NODE_EXPRESSION,
    NODE_FUNCTION_CALL,
    NODE_TYPE,
    NODE_OPERATOR,
    NODE_INDENTIFIER,
    NODE_FUNCTION_DEFINITION_PARAMETERS,

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

#endif