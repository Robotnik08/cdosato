#include "include/common.h"

#include "include/virtual-machine.h"
#include "include/debug.h"

int main (void) {
    VirtualMachine vm;
    initVirtualMachine(&vm);
    CodeInstance* instance = vm.instance;

    clock_t start_compile = clock();
    writeInstruction(instance, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(0));
    for (int i = 0; i < 1000000; i++) {
        writeInstruction(instance, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(1));
        writeInstruction(instance, OP_BINARY_ADD);
    }
    writeInstruction(instance, OP_PRINT);
    writeInstruction(instance, OP_RETURN);
    clock_t end_compile = clock();
    double time_compile = (double)(end_compile - start_compile) / CLOCKS_PER_SEC;
    printf("Time to compile: %f\n", time_compile);


    // disassembleCode(instance, "Main");

    clock_t start = clock();
    runVirtualMachine(&vm);
    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time: %f\n", time);

    freeVirtualMachine(&vm);

    printf("\nDone\n");
    return 0;
}