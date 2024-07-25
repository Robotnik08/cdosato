#include "include/common.h"

#include "include/virtual-machine.h"
#include "include/lexer.h"
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

    fseek(file, 0, SEEK_END); 
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source = malloc(length + 1);
    if (source == NULL) {
        printf("Could not allocate memory for source\n");
        return 1;
    }

    source[length] = '\0';
    // read into source
    fread(source, 1, length, file);
    


    /// TEST LEXER

    printf ("==== Source: ====\n%s\n", source);

    TokenList list;
    init_TokenList(&list);

    int res = tokenise(&list, source, length);
    if (res != 0) {
        printf("Error tokenising source %d\n", res);
        return 1;
    }

    printf("==== Tokens: (%d) ====\n", list.count);
    printTokens(list);

    free_TokenList(&list);

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