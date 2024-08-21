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
    init_StackFrames(&vm->stack_frames);
    init_NameMap(&vm->mappings);
    vm->ip = vm->instance->code;
}

void freeVirtualMachine(VirtualMachine* vm) {
    freeCodeInstance(vm->instance);
    destroyValueArray(&vm->stack);    
    destroyValueArray(&vm->constants);
    destroyValueArray(&vm->globals);
    free_StackFrames(&vm->stack_frames);
    free_NameMap(&vm->mappings);
    free(vm->instance);
}

void pushValue(ValueArray* array, Value value) {
    write_ValueArray(array, value);
}

#define ERROR(e_code) printError(ast.source, ast.tokens.tokens[vm->instance->token_indices[vm->ip - vm->instance->code - 1]].start - ast.source, e_code)

#define DESTROYIFLITERAL(value) if (!value.defined) { destroyValue(value); }

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
                if (constant.type == TYPE_STRING) {
                    // copy the string so the original pointer stays intact
                    char* new_string = malloc(strlen(constant.as.stringValue) + 1);
                    strcpy(new_string, constant.as.stringValue);
                    constant.as.stringValue = new_string;
                }
                pushValue(&vm->stack, constant);
                break;
            }

            case OP_BINARY_ADD: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();
                Value result = (Value){ TYPE_LONG, .as.longValue = a.as.longValue + b.as.longValue };
                pushValue(&vm->stack, result);
                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_SUBTRACT: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();
                Value result = (Value){ TYPE_LONG, .as.longValue = a.as.longValue - b.as.longValue };
                pushValue(&vm->stack, result);
                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_MULTIPLY: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();
                Value result = (Value){ TYPE_LONG, .as.longValue = a.as.longValue * b.as.longValue };
                pushValue(&vm->stack, result);
                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }


            case OP_GETLIST: {
                Value index = POP_VALUE();
                Value list = POP_VALUE();
                if (list.array_depth == 0) {
                    ERROR(E_NOT_AN_ARRAY);
                }

                // TO DO type checking
                int i = index.as.longValue;
                ValueArray* array = (ValueArray*)list.as.objectValue;
                if (i < 0 || i >= array->count) {
                    ERROR(E_INDEX_OUT_OF_BOUNDS);
                }

                Value value = array->values[i];
                if (!value.defined) {
                    // hard copy the value to save it from literal destruction
                    value = hardCopyValue(value);
                }

                pushValue(&vm->stack, value);

                DESTROYIFLITERAL(list);
                DESTROYIFLITERAL(index);
                break;
            }


            case OP_LOAD_FAST: {
                uint16_t index = NEXT_SHORT();
                Value local = vm->stack.values[index];
                pushValue(&vm->stack, local);
                break;
            }
            case OP_STORE_FAST: {
                uint16_t index = NEXT_SHORT();
                Value value = POP_VALUE();

                // destroy old value
                destroyValue(vm->stack.values[index]);

                vm->stack.values[index] = value; // store to local
                markDefined(&vm->stack.values[index]);
                break;
            }

            case OP_PUSH_NULL: {
                pushValue(&vm->stack, UNDEFINED_VALUE);
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
                markDefined(&value);
                if (!vm->globals.values[index].defined) {
                    ERROR(E_UNDEFINED_VARIABLE);
                }
                // destroy old value
                destroyValue(vm->globals.values[index]);

                vm->globals.values[index] = value;
                break;
            }

            case OP_DEFINE: {
                uint16_t index = NEXT_SHORT();
                Value value = POP_VALUE();
                if (vm->globals.values[index].defined) {
                    ERROR(E_ALREADY_DEFINED_VARIABLE);
                }

                vm->globals.values[index] = value;
                markDefined(&vm->globals.values[index]);
                break;
            }


            case OP_TYPE_CAST: {
                // not available for now
                break;
            }

            case OP_BUILD_LIST: {
                int count = NEXT_SHORT();
                Value list = (Value){ TYPE_VAR, .as.objectValue = malloc(sizeof(ValueArray)), .array_depth = 1, .is_variable_type = true };
                init_ValueArray((ValueArray*)list.as.objectValue);
                for (int i = 0; i < count; i++) {
                    Value value = POP_VALUE();
                    write_ValueArray((ValueArray*)list.as.objectValue, value);
                }
                pushValue(&vm->stack, list);
                break;
            }

            
            case OP_PRINT: {
                Value value = POP_VALUE();

                printValue(value, false);
                printf("\n");
                DESTROYIFLITERAL(value);
                break;
            }
            case OP_POP: {
                POP_VALUE();
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