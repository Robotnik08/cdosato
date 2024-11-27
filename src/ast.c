#include "../include/common.h"

#include "../include/token.h"
#include "../include/ast.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/virtual-machine.h"
#include "../include/memory.h"
#include "../include/filetools.h"

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

void load_AST (AST* ast, char* source, size_t length, char* name, int debug, VirtualMachine* vm) {
    ast->source = source;
    ast->length = length;
    ast->name = name;

    int res = tokenise(&ast->tokens, ast->source, length, vm, name);
    if (res != 0) {
        printf("Error tokenising source %d\n", res);
        exit(res);
    }
    if (debug & 0b100) {
        puts("==== Tokens: ====\n");
        printTokens(ast->tokens);
    }

    ast->root = parse(ast->source, ast->length, 0, ast->tokens.count, ast->tokens, NODE_PROGRAM, name);
}

void free_AST (AST* ast) {
    free_TokenList(&ast->tokens);
    destroyNodeList(&ast->root.body);
    free(ast->name);
    ast->name = NULL;
    free(ast->source);
    ast->source = NULL;
}


void parseFile (AST* ast, char* name, int debug, VirtualMachine* vm) {
    FILE* file = fopen(name, "r");
    if (!file) {
        printf("Error opening file: %s\n", name);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long long int length = getFileSize(file);
    fseek(file, 0, SEEK_SET);

    char* source = malloc(length + 1); // AST owns the source
    fread(source, 1, length, file);
    source[length] = '\0';
    fclose(file);

    int globals_count_before = vm->globals.count;

    load_AST(ast, source, length, name, debug, vm);

    for (int i = globals_count_before; i < vm->mappings.count; i++) { // add new mappings to globals
        write_ValueArray(&vm->globals, UNDEFINED_VALUE);
    }
}