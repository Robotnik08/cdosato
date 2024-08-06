#ifndef dosato_ast_h
#define dosato_ast_h

#include "common.h"
#include "node.h"
#include "token.h"

typedef struct {
    char* source;
    size_t length;
    char* name;
    TokenList tokens;
    Node root;
} AST;

void init_AST (AST* ast);
void load_AST (AST* ast, char* source, size_t length, char* name, int debug, VirtualMachine* vm);
void free_AST (AST* ast);

#endif