#include "../include/common.h"

#include "../include/virtual-machine.h"
#include "../include/error.h"
#include "../include/ast.h"

void initVirtualMachine(VirtualMachine* vm) {
    vm->instance = malloc(sizeof(CodeInstance));
    initCodeInstance(vm->instance);
    init_ValueArray(&vm->stack);
    init_ValueArray(&vm->constants);
    init_ValueArray(&vm->globals);
    init_ValueArray(&vm->locals);
    init_StackFrames(&vm->stack_frames);
    init_NameMap(&vm->mappings);
    vm->ip = vm->instance->code;
}

void freeVirtualMachine(VirtualMachine* vm) {
    freeCodeInstance(vm->instance);
    destroyValueArray(&vm->stack);    
    destroyValueArray(&vm->constants);
    destroyValueArray(&vm->globals);
    destroyValueArray(&vm->locals);
    free_StackFrames(&vm->stack_frames);
    free_NameMap(&vm->mappings);
    free(vm->instance);
}

void pushValue(ValueArray* array, Value value) {
    write_ValueArray(array, value);
}

#define ERROR(e_code) printError(ast.source, ast.tokens.tokens[vm->instance->token_indices[vm->ip - vm->instance->code - 1]].start - ast.source, e_code)

int runVirtualMachine (VirtualMachine* vm, int debug, AST ast) {
    if (debug) printf("Running virtual machine\n");
    bool halt = false;
    vm->ip = vm->instance->code;
    while (!halt) {
        OpCode instruction = NEXT_BYTE();
        switch (instruction) {
            case OP_LOAD_CONSTANT: {
                uint16_t index = NEXT_SHORT();
                Value constant = vm->constants.values[index];
                pushValue(&vm->stack, constant);
                break;
            }
            case OP_BINARY_ADD: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();
                Value result = (Value){ TYPE_LONG, .as.longValue = a.as.longValue + b.as.longValue };
                pushValue(&vm->stack, result);
                break;
            }
            case OP_BINARY_SUBTRACT: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();
                Value result = (Value){ TYPE_LONG, .as.longValue = a.as.longValue - b.as.longValue };
                pushValue(&vm->stack, result);
                break;
            }
            case OP_BINARY_MULTIPLY: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();
                Value result = (Value){ TYPE_LONG, .as.longValue = a.as.longValue * b.as.longValue };
                pushValue(&vm->stack, result);
                break;
            }

            case OP_LOAD_FAST: {
                uint16_t index = NEXT_SHORT();
                Value local = vm->locals.values[index];
                if (!local.defined) {
                    ERROR(E_UNDEFINED_VARIABLE);
                }
                pushValue(&vm->stack, local);
                break;
            }
            case OP_STORE_FAST: {
                uint16_t index = NEXT_SHORT();
                Value value = POP_VALUE();
                if (!vm->locals.values[index].defined) {
                    ERROR(E_UNDEFINED_VARIABLE);
                }
                vm->locals.values[index] = value;
                break;
            }

            case OP_LOAD: {
                uint16_t index = NEXT_SHORT();
                Value global = vm->globals.values[index];
                if (!global.defined) {
                    ERROR(E_UNDEFINED_VARIABLE);
                }
                pushValue(&vm->stack, global);
                break;
            }
            case OP_STORE: {
                uint16_t index = NEXT_SHORT();
                Value value = POP_VALUE();
                if (!vm->globals.values[index].defined) {
                    ERROR(E_UNDEFINED_VARIABLE);
                }
                vm->globals.values[index] = value;
                break;
            }

            case OP_LOAD_SMART: {
                uint16_t index = NEXT_SHORT();
                Value local = vm->locals.values[index];
                if (local.defined) {
                    pushValue(&vm->stack, local);
                } else {
                    Value global = vm->globals.values[index];
                    if (!global.defined) {
                        ERROR(E_UNDEFINED_VARIABLE);
                    }
                    pushValue(&vm->stack, global);
                }
                break;
            }

            case OP_STORE_SMART: {
                uint16_t index = NEXT_SHORT();
                Value value = POP_VALUE();
                Value local = vm->locals.values[index];
                if (local.defined) {
                    vm->locals.values[index] = value;
                } else {
                    if (!vm->globals.values[index].defined) {
                        ERROR(E_UNDEFINED_VARIABLE);
                    }
                    vm->globals.values[index] = value;
                }
                break;
            }

            case OP_DEFINE: {
                uint16_t index = NEXT_SHORT();
                Value value = POP_VALUE();
                if (vm->globals.values[index].defined) {
                    ERROR(E_ALREADY_DEFINED_VARIABLE);
                }

                vm->globals.values[index] = value;
                vm->globals.values[index].defined = true;
                break;
            }

            case OP_DEFINE_FAST: {
                uint16_t index = NEXT_SHORT();
                Value value = POP_VALUE();
                if (vm->locals.values[index].defined) {
                    ERROR(E_ALREADY_DEFINED_VARIABLE);
                }

                vm->locals.values[index] = value;
                vm->locals.values[index].defined = true;
                break;
            }


            case OP_TYPE_CAST: {
                // not available for now
                break;
            }

            
            case OP_PRINT: {
                Value value = POP_VALUE();
                switch (value.type) {
                    case TYPE_ULONG: {
                        printf("%llu\n", value.as.ulongValue);
                        break;
                    }
                    case TYPE_LONG: {
                        printf("%lld\n", value.as.longValue);
                        break;
                    }
                    case TYPE_STRING: {
                        printf("%s\n", value.as.stringValue);
                        break;
                    }
                    case TYPE_CHAR: {
                        printf("%c\n", value.as.charValue);
                        break;
                    }
                    case TYPE_FLOAT: {
                        printf("%f\n", value.as.floatValue);
                        break;
                    }
                    case TYPE_DOUBLE: {
                        printf("%f\n", value.as.doubleValue);
                        break;
                    }
                    case TYPE_BOOL: {
                        printf("%s\n", value.as.boolValue ? "TRUE" : "FALSE");
                        break;
                    }
                    default: {
                        printf("Unknown type\n");
                        break;
                    }
                }
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

#undef ERROR