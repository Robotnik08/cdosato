#include "include/common.h"

#include "include/virtual-machine.h"
#include "include/ast.h"
#include "include/debug.h"
#include "include/compiler.h"

int debug = 0b0;
#define DEBUG 0b1
#define DEBUG_SOURCE 0b10
#define DEBUG_LEX 0b100
#define DEBUG_PARSE 0b1000
#define DEBUG_COMPILE 0b10000

#define PRINT_USAGE printf("Usage: dosato <source file> \n");

int main (int argc, char** argv) {
    if (argc < 2) {
        PRINT_USAGE
        return 1;
    }

    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            PRINT_USAGE
            return 0;
        } else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            printf("Dosato version %s\n", DOSATO_VERSION);
            return 0;
        }
    }

    if (argc >= 3) {
        if (strcmp(argv[2], "-d") == 0 || strcmp(argv[2], "--debug") == 0) {
            debug = DEBUG;
        }
    }

    if (argc == 4 && debug) {
        if (strcmp(argv[3], "-s") == 0 || strcmp(argv[3], "--source") == 0) {
            debug |= DEBUG_SOURCE;
        } else if (strcmp(argv[3], "-l") == 0 || strcmp(argv[3], "--lexer") == 0) {
            debug |= DEBUG_LEX;
        } else if (strcmp(argv[3], "-p") == 0 || strcmp(argv[3], "--parse") == 0) {
            debug |= DEBUG_PARSE;
        } else if (strcmp(argv[3], "-c") == 0 || strcmp(argv[3], "--compile") == 0) {
            debug |= DEBUG_COMPILE;
        } else if (strcmp(argv[3], "-a") == 0 || strcmp(argv[3], "--all") == 0) {
            debug |= DEBUG_SOURCE | DEBUG_LEX | DEBUG_PARSE | DEBUG_COMPILE;
        } else if (strcmp(argv[3], "-n") == 0 || strcmp(argv[3], "--none") == 0) {
            // do nothing, but still debug mode
        } else {
            printf("Unknown debug mode '%s'\n", argv[3]);
            return 1;
        }
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

    if (debug & DEBUG_SOURCE) printf ("==== Source: (%d) ====\n%s\n", length, source);
    
    VirtualMachine vm;
    initVirtualMachine(&vm);


    AST main_ast;
    init_AST(&main_ast);
    load_AST(&main_ast, source, length, argv[1], debug, &vm);

    if (debug & DEBUG_PARSE) {
        printf("==== AST: ====\n");
        printNode(main_ast.root, 0);
    } 


    compile(&vm, main_ast);


    if (debug & DEBUG_COMPILE) {
        disassembleCode(vm.instance, "Main");
        for (int i = 0; i < vm.functions.count; i++) {
            printf("\n");
            disassembleCode(vm.functions.funcs[i].instance, vm.functions.funcs[i].name);
        }
    } 

    if (!debug) {
        runVirtualMachine(&vm, debug, main_ast);
    } else {
        // time it
        clock_t start = clock();
        runVirtualMachine(&vm, debug, main_ast);
        clock_t end = clock();
        double time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("\nExecution time: %f\n", time);
        printf("Done running\n");
    }

    freeVirtualMachine(&vm);

    free_AST(&main_ast);

    free(source);

    if (debug) printf("Succesfull cleanup\n");
    return 0;
}