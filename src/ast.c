#include "../include/common.h"

#include "../include/token.h"
#include "../include/ast.h"
#include "../include/lexer.h"
#include "../include/parser.h"

void init_AST (AST* ast) {
    ast->source = NULL;
    ast->length = 0;
    ast->name = NULL;
    init_TokenList(&ast->tokens);
    init_NodeList(&ast->root.body);
    ast->root.start = 0;
    ast->root.end = 0;
    ast->root.type = NODE_PROGRAM;
}

void load_AST (AST* ast, char* source, size_t length, char* name) {
    ast->source = source;
    ast->length = length;
    ast->name = name;

    int res = tokenise(&ast->tokens, source, length);
    if (res != 0) {
        printf("Error tokenising source %d\n", res);
        exit(res);
    }

    ast->root = parse(ast->source, ast->length, 0, ast->tokens.count, ast->tokens, NODE_PROGRAM);
}

void free_AST (AST* ast) {
    free_TokenList(&ast->tokens);
    destroyNodeList(&ast->root.body);
}
