#include "include/common.h"

#include "include/virtual-machine.h"
#include "include/ast.h"
#include "include/debug.h"
#include "include/compiler.h"
#include "include/filetools.h"

#include "include/standard_libraries/load_std.h"

int debug = 0b0;
#define DEBUG 0b1
#define DEBUG_SOURCE 0b10
#define DEBUG_LEX 0b100
#define DEBUG_PARSE 0b1000
#define DEBUG_COMPILE 0b10000

#define PRINT_USAGE printf("%s", "Usage: dosato <source file> <args> \n");

int main (int argc, char** argv) {
    if (argc < 2) {
        PRINT_USAGE
        printf("%s", "Type 'dosato -h' for help\n");
        return 1;
    }

    bool cin_mode = false;

    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            PRINT_USAGE
            printf("%s", "Options:\n");
            printf("%s", "  -h, --help: Show this help message\n");
            printf("%s", "  -v, --version: Show version information\n");
            printf("%s", "  -c, --cin: Cin mode\n");
            printf("%s", "  -d, --debug: Enable debug mode (after the source file)\n");
            printf("%s", "     Debug options (prefix with -d):\n");
            printf("%s", "     -s, --source: Show source code\n");
            printf("%s", "     -l, --lexer: Show tokens\n");
            printf("%s", "     -p, --parser: Show AST\n");
            printf("%s", "     -c, --compiler: Show bytecode\n");
            printf("%s", "     -a, --all: Show all of the above\n");
            printf("%s", "     -n, --none: Show none\n");
            return 0;
        } else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            printf("Dosato version %s, compiled on %s\n", DOSATO_VERSION, DOSATO_DATE);
            printf("%s", "Type 'dosato -h' for help\n");
            return 0;
        } else if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--cin") == 0) {
            // cin mode
            cin_mode = true;
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
        } else if (strcmp(argv[3], "-p") == 0 || strcmp(argv[3], "--parser") == 0) {
            debug |= DEBUG_PARSE;
        } else if (strcmp(argv[3], "-c") == 0 || strcmp(argv[3], "--compiler") == 0) {
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

    char* source = NULL;
    long long int length = 0;

    if (!cin_mode) {
        FILE* file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Could not open file '%s'\n", argv[1]);
            return 1;
        }

        char* argument_dir = malloc(strlen(argv[1]) + 1);
        strcpy(argument_dir, argv[1]);
        char* index = strrchr(argument_dir, '/');
        if (index == NULL) {
            index = strrchr(argument_dir, '\\');
        }
        if (index != NULL) {
            *index = '\0';
        }

        chdir(argument_dir);

        free (argument_dir);

        length = getFileSize(file);

        source = malloc(length + 1);
        if (source == NULL) {
            printf("%s", "Could not allocate memory for source\n");
            return 1;
        }

        // read into source
        fread(source, 1, length, file);
        source[length] = '\0';

        fclose(file);
    } else {
        printf("%s", "Enter source code:\n");
        source = readStdin(&length);
    }

    if (debug & DEBUG_SOURCE) printf ("==== Source: (%d) ====\n%s\n", length, source);
    
    VirtualMachine* vm = malloc(sizeof(VirtualMachine));
    main_vm = vm;
    initVirtualMachine(vm);


    AST* main_ast = malloc(sizeof(AST));
    init_AST(main_ast);
    char* name = cin_mode ? COPY_STRING("stdin") : COPY_STRING(argv[1]);
    load_AST(main_ast, source, length, name, debug, vm);

    if (debug & DEBUG_PARSE) {
        printf("%s", "==== AST: ====\n");
        printNode(main_ast->root, 0);
    } 

    // initialize the standard library
    loadStandardLibrary(vm);

    init_ValueArray(&vm->globals);
    for (int i = 0; i < vm->mappings.count; i++) {
        write_ValueArray(&vm->globals, UNDEFINED_VALUE);
    }

    loadConstants(vm, argv, argc);

    compile(vm, main_ast);


    if (debug & DEBUG_COMPILE) {
        disassembleCode(vm->instance, "Main");
        for (int i = 0; i < vm->includes.count; i++) {
            disassembleCode(&vm->includes.instances[i], ((AST*)vm->includes.instances[i].ast)->name);
        }

        for (int i = 0; i < vm->functions.count; i++) {
            if (vm->functions.funcs[i].is_compiled) continue;
            printf("%s", "\n");
            disassembleCode(vm->functions.funcs[i].instance, vm->functions.funcs[i].name);
        }
    }

    int exit_code = 0;

    write_StackFrames(&vm->stack_frames, 0);

    if (!debug) {
        exit_code = runVirtualMachine(vm, debug, true);
    } else {
        // time it
        clock_t start = clock();
        exit_code = runVirtualMachine(vm, debug, true);
        clock_t end = clock();
        double time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("\nExecution time: %f\n", time);
        printf("Stack size: %d (%s)\n", vm->stack.count, vm->stack.count == 0 ? "passed" : "failed");
        if ((int)vm->stack.count > 0) {
            printf("%s", "Left over stack:\n");
            for (int i = 0; i < vm->stack.count; i++) {
                printf("%d: ", i);
                printValue(vm->stack.values[i], true);
                printf("%s", "\n");
            }
        }
        printf("%s", "Done running\n");
    }

    freeVirtualMachine(vm);
    free(vm);

    if (debug) printf("%s", "Succesfull cleanup\n");
    return exit_code;
}