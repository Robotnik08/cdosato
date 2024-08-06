#include "../include/common.h"

#include "../include/compiler.h"

void compile(VirtualMachine* vm, AST ast) {
    writeByteCode(vm->instance, OP_STOP);
}