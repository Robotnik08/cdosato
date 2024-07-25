#include "include/common.h"

#include "include/virtual-machine.h"
#include "include/ast.h"
#include "include/debug.h"




int main (int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: dosato <source file>\n");
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Could not open file '%s'\n", argv[1]);
        return 1;
    }

    long long int length = getFileSize(file);

    char* source = malloc(length + 1);
    if (source == NULL) {
        printf("Could not allocate memory for source\n");
        return 1;
    }

    // read into source
    fread(source, 1, length, file);
    source[length] = '\0';
    

    AST main_ast;
    init_AST(&main_ast);
    load_AST(&main_ast, source, length, argv[1]);

    printf ("==== Source: (%d) ====\n%s\n", length, source);

    printf("==== Tokens: (%d) ====\n", main_ast.tokens.count);
    printTokens(main_ast.tokens);

    printf("==== AST: ====\n");
    printNode(main_ast.root, 0);


    free_AST(&main_ast);

    free(source);




    /// TEST VM

    // VirtualMachine vm;
    // initVirtualMachine(&vm);
    // CodeInstance* instance = vm.instance;

    // writeInstruction(instance, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(0));
    // writeInstruction(instance, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(1));
    // writeInstruction(instance, OP_BINARY_ADD);
    // writeInstruction(instance, OP_PRINT);
    // writeInstruction(instance, OP_RETURN);


    // disassembleCode(instance, "Main");

    // runVirtualMachine(&vm);

    // freeVirtualMachine(&vm);

    printf("\nDone\n");
    return 0;
}