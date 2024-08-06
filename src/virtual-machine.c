#include "../include/common.h"

#include "../include/virtual-machine.h"

void initVirtualMachine(VirtualMachine* vm) {
    vm->instance = malloc(sizeof(CodeInstance));
    initCodeInstance(vm->instance);
    init_ValueArray(&vm->stack);
    init_ValueArray(&vm->constants);
    init_StackFrames(&vm->stack_frames);
    vm->ip = vm->instance->code;
}

void freeVirtualMachine(VirtualMachine* vm) {
    freeCodeInstance(vm->instance);
    destroyValueArray(&vm->stack);    
    destroyValueArray(&vm->constants);
    free_StackFrames(&vm->stack_frames);
    free(vm->instance);
}

void pushValue(ValueArray* array, Value value) {
    write_ValueArray(array, value);
}

int runVirtualMachine (VirtualMachine* vm, int debug) {
    if (debug) printf("Running virtual machine\n");
    bool halt = false;
    vm->ip = vm->instance->code;
    while (!halt) {
        OpCode instruction = NEXT_BYTE();
        switch (instruction) {
            case OP_LOAD_CONSTANT: {
                Value constant = (Value){ TYPE_INT, NEXT_SHORT() };
                pushValue(&vm->stack, constant);
                break;
            }
            case OP_BINARY_ADD: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();
                Value result = (Value){ TYPE_INT, a.as.intValue + b.as.intValue };
                pushValue(&vm->stack, result);
                break;
            }
            case OP_BINARY_SUBTRACT: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();
                Value result = (Value){ TYPE_INT, a.as.intValue - b.as.intValue };
                pushValue(&vm->stack, result);
                break;
            }
            case OP_BINARY_MULTIPLY: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();
                Value result = (Value){ TYPE_INT, a.as.intValue * b.as.intValue };
                pushValue(&vm->stack, result);
                break;
            }
            case OP_PRINT: {
                Value value = POP_VALUE();
                printf("%d\n", value.as.intValue);
                break;
            }
            case OP_STOP: {
                halt = true;
                break;
            }
            case OP_RETURN: {
                halt = true;
                break;
            }
            default: {
                printf("Unknown instruction: %d\n", instruction);
                halt = true;
                break;
            }
        }
    }
    return 0;
}