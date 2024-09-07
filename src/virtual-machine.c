#include "../include/common.h"

#include "../include/virtual-machine.h"
#include "../include/error.h"
#include "../include/ast.h"
#include "../include/memory.h"
#include "../include/dynamic_library_loader.h"

DOSATO_LIST_FUNC_GEN(FunctionList, Function, funcs)

void destroy_Function(Function* func) {
    if (!func->is_compiled) {
        free(func->name);
        free(func->argv);
        free(func->argt);
    }
}

void destroy_FunctionList(FunctionList* list) {
    for (size_t i = 0; i < list->count; i++) {
        destroy_Function(&list->funcs[i]);
    }
    free_FunctionList(list);
}

void init_Function(Function* func) {
    func->name = NULL;
    func->name_index = 0;
    func->argv = NULL;
    func->argt = NULL;
    func->arity = 0;
    func->return_type = TYPE_VOID;
    func->is_compiled = false;
    func->func_ptr = NULL;
}

void initVirtualMachine(VirtualMachine* vm) {
    vm->instance = malloc(sizeof(CodeInstance));
    initCodeInstance(vm->instance);
    init_ValueArray(&vm->stack);
    init_ValueArray(&vm->constants);
    init_ValueArray(&vm->globals);
    init_StackFrames(&vm->stack_frames);
    init_FunctionList(&vm->functions);
    init_NameMap(&vm->mappings);
    init_NameMap(&vm->constants_map);
    init_ErrorJumps(&vm->error_jumps);
    init_CodeInstanceList(&vm->includes);
    vm->ip = vm->instance->code;
}

void freeVirtualMachine(VirtualMachine* vm) {
    freeCodeInstance(vm->instance);
    destroyValueArray(&vm->stack);    
    destroyValueArray(&vm->constants);
    destroyValueArray(&vm->globals);
    free_StackFrames(&vm->stack_frames);
    destroy_FunctionList(&vm->functions);
    free_NameMap(&vm->mappings);
    free_NameMap(&vm->constants_map);
    free_ErrorJumps(&vm->error_jumps);
    free_CodeInstanceList(&vm->includes);
    free(vm->instance);
}

void pushValue(ValueArray* array, Value value) {
    write_ValueArray(array, value);
}

#define PRINT_ERROR(e_code) do { \
    if (vm->error_jumps.count > 0) { \
        ErrorJump jump = vm->error_jumps.jumps[--vm->error_jumps.count]; \
        while (vm->stack.count > jump.error_stack_count) { \
            destroyValue(&POP_VALUE()); \
        } \
        vm->ip = jump.error_jump_loc; \
        vm->globals.values[0].as.longValue = e_code; \
    } else { \
        size_t token_index = active_instance->token_indices[vm->ip - active_instance->code - 1]; \
        printError(((AST*)active_instance->ast)->source, ((AST*)active_instance->ast)->tokens.tokens[token_index].start - ((AST*)active_instance->ast)->source, ((AST*)active_instance->ast)->name, e_code);\
    } \
} while(0)

#define DESTROYIFLITERAL(value) if (!value.defined) { destroyValue(&value); }

int runVirtualMachine (VirtualMachine* vm, int debug) {
    if (debug) printf("Running virtual machine\n");
    bool halt = false;
    vm->ip = vm->instance->code;
    CodeInstance* active_instance = vm->instance;
    char* ip_stack[RECURSION_LIMIT];
    CodeInstance* active_stack[RECURSION_LIMIT];
    size_t ip_stack_count = 0;
    PUSH_STACK(0); // first local frame

    // set global variable 0 to zero (underscore)
    Value zero = (Value){ TYPE_LONG, .as.longValue = 0, .defined = false, .is_variable_type = true };
    vm->globals.values[0] = zero;

    // set all functions to globals
    for (size_t i = 0; i < vm->functions.count; i++) {
        Value func = (Value){ TYPE_FUNCTION, .as.longValue = i, .defined = true, .is_variable_type = false };
        vm->globals.values[vm->functions.funcs[i].name_index] = func;
    }

    while (!halt) {
        OpCode instruction = NEXT_BYTE();
        switch (instruction) {
            
            default: {
                printf("Unknown instruction: %d at: %d\n", instruction, vm->ip - active_instance->code - 1);
                halt = true;
                break;
            }

            case OP_NOP: {
                break;
            }

            case OP_INCLUDE: {
                uint16_t index = NEXT_SHORT();
                CodeInstance* included_instance = &vm->includes.instances[index];
                active_stack[ip_stack_count] = active_instance;
                ip_stack[ip_stack_count++] = vm->ip;

                vm->ip = included_instance->code;
                active_instance = included_instance;
                break;
            }

            case OP_END_INCLUDE: {
                vm->ip = ip_stack[--ip_stack_count];
                active_instance = active_stack[ip_stack_count];
                break;
            }

            
            case OP_JUMP: {
                uint16_t offset = NEXT_SHORT();
                vm->ip = offset + active_instance->code;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = NEXT_SHORT();
                Value condition = POP_VALUE();
                if (condition.as.boolValue == false) {
                    vm->ip = offset + active_instance->code;
                }
                DESTROYIFLITERAL(condition);
                break;
            }

            case OP_JUMP_IF_TRUE: {
                uint16_t offset = NEXT_SHORT();
                Value condition = POP_VALUE();
                if (condition.as.boolValue == true) {
                    vm->ip = offset + active_instance->code;
                }
                DESTROYIFLITERAL(condition);
                break;
            }

            case OP_JUMP_IF_EXCEPTION: {
                uint16_t offset = NEXT_SHORT();
                size_t stack_count = vm->stack.count;
                write_ErrorJumps(&vm->error_jumps, (ErrorJump){ .error_jump_loc = active_instance->code + offset, .error_stack_count = stack_count });
                break;
            }

            case OP_CONTINUE: {
                uint16_t offset = NEXT_SHORT();
                uint16_t pop_amount = NEXT_SHORT();
                vm->ip = offset + active_instance->code;
                for (size_t i = 0; i < pop_amount; i++) {
                    destroyValue(&POP_VALUE());
                }
                break;
            }

            case OP_BREAK: {
                uint16_t offset = NEXT_SHORT();
                uint16_t pop_amount = NEXT_SHORT();
                vm->ip = offset + active_instance->code;
                for (size_t i = 0; i < pop_amount; i++) {
                    destroyValue(&POP_VALUE());
                }

                if (*vm->ip == OP_FOR_ITER) {
                    // set the iterator to -2 (so the loop will end)
                    vm->stack.values[vm->stack.count - 1].as.longValue = -2;
                } else if (NEXT_BYTE() == OP_JUMP_IF_FALSE) {
                    uint16_t offset = NEXT_SHORT();
                    vm->ip = offset + active_instance->code;
                }
                break;
            }

            case OP_CLEAR_EXCEPTION: {
                --vm->error_jumps.count;
                break;
            }

            case OP_CALL: {
                uint8_t arity = NEXT_BYTE();
                Value func = POP_VALUE();

                if (func.type != TYPE_FUNCTION) {
                    PRINT_ERROR(E_NOT_A_FUNCTION);
                }
                Function* function = &vm->functions.funcs[func.as.longValue];
                if (function->is_compiled) {
                    // call the compiled function
                    ValueArray args;
                    init_ValueArray(&args);
                    for (int i = 0; i < arity; i++) {
                        pushValue(&args, POP_VALUE());
                    }
                    Value return_val = ((DosatoFunction)function->func_ptr)(args, debug);
                    if (return_val.type == TYPE_EXPECTION) {
                        PRINT_ERROR(return_val.as.longValue);
                    }
                    pushValue(&vm->stack, return_val);

                    break;
                }

                if (arity != function->arity && function->arity != -1) {
                    PRINT_ERROR(E_WRONG_NUMBER_OF_ARGUMENTS);
                }

                // cast arguments
                for (int i = 0; i < arity; i++) {
                    Value* arg = &vm->stack.values[vm->stack.count - arity + i];
                    ErrorType code = castValue(arg, function->argt[i]);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                }
                
                // push new frame
                PUSH_STACK(vm->stack.count - arity);

                // ip stack
                active_stack[ip_stack_count] = active_instance;
                ip_stack[ip_stack_count++] = vm->ip;

                // set ip to function
                vm->ip = function->instance->code;
                active_instance = function->instance;
                
                break;
            }

            case OP_END_FUNC: {
                // pop frame
                size_t frame = POP_STACK();

                // pop ip
                vm->ip = ip_stack[--ip_stack_count];
                active_instance = active_stack[ip_stack_count];

                // push return value (NULL because END_FUNC is always at the end of a function)
                pushValue(&vm->stack, UNDEFINED_VALUE);
                break;
            }


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

            case OP_PUSH_FALSE: {
                Value value = (Value){ TYPE_BOOL, .as.boolValue = false };
                pushValue(&vm->stack, value);
                break;
            }

            case OP_PUSH_TRUE: {
                Value value = (Value){ TYPE_BOOL, .as.boolValue = true };
                pushValue(&vm->stack, value);
                break;
            }

            case OP_LOAD_UNDERSCORE: {
                // underscore is global variable 0
                Value underscore = vm->globals.values[0];
                pushValue(&vm->stack, underscore);
                break;
            }


            case OP_GETLIST: {
                Value index = POP_VALUE();
                Value list = POP_VALUE();
                if (list.type != TYPE_ARRAY && list.type != TYPE_STRING) {
                    PRINT_ERROR(E_NOT_AN_ARRAY);
                }

                ErrorType code = castValue(&index, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                long long int i = index.as.longValue;
                if (list.type == TYPE_ARRAY) {
                    ValueArray* array = (ValueArray*)list.as.objectValue;
                    if (i < 0 || i >= array->count) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }

                    Value value = array->values[i];
                    value = hardCopyValue(value);

                    pushValue(&vm->stack, value);
                } else if (list.type == TYPE_STRING) {
                    char* string = list.as.stringValue;
                    if (i < 0 || i >= strlen(string)) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }

                    char val = string[i];
                    Value value = (Value){ TYPE_CHAR, .as.charValue = val };
                    pushValue(&vm->stack, value);
                }

                DESTROYIFLITERAL(list);
                DESTROYIFLITERAL(index);
                break;
            }

            case OP_GETOBJECT: {
                Value key = POP_VALUE();
                Value object = POP_VALUE();
                if (object.type != TYPE_OBJECT) {
                    PRINT_ERROR(E_NOT_AN_OBJECT);
                }

                ValueObject* obj = (ValueObject*)object.as.objectValue;
                if (!hasKey(obj, key.as.stringValue)) {
                    PRINT_ERROR(E_KEY_NOT_FOUND);
                }

                Value value = *getValueAtKey(obj, key.as.stringValue);
                value = hardCopyValue(value);

                pushValue(&vm->stack, value);

                DESTROYIFLITERAL(object);
                DESTROYIFLITERAL(key);
                break;
            }

            case OP_REFERENCE: {
                uint16_t index = NEXT_SHORT();
                Value global = vm->globals.values[index];
                if (!global.defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                pushValue(&vm->stack, global);
                break;
            }

            case OP_REFERENCE_FAST: {
                uint16_t index = NEXT_SHORT() + PEEK_STACK();
                Value local = vm->stack.values[index];
                if (!local.defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                pushValue(&vm->stack, local);
                break;
            }

            case OP_REFERENCE_SUBSCR: {
                Value index = POP_VALUE();
                Value list = POP_VALUE();
                if (list.type != TYPE_ARRAY) {
                    PRINT_ERROR(E_NOT_AN_ARRAY);
                }
                if (!list.defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                // TO DO type checking
                int i = index.as.longValue;
                ValueArray* array = (ValueArray*)list.as.objectValue;
                if (i < 0 || i >= array->count) {
                    PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                }

                Value value = array->values[i];
                pushValue(&vm->stack, value);

                DESTROYIFLITERAL(index);
                break;
            }

            case OP_REFERENCE_GETOBJ: {
                Value key = POP_VALUE();
                Value object = POP_VALUE();

                if (object.type != TYPE_OBJECT) {
                    PRINT_ERROR(E_NOT_AN_OBJECT);
                }

                ValueObject* obj = (ValueObject*)object.as.objectValue;
                if (!hasKey(obj, key.as.stringValue)) {
                    PRINT_ERROR(E_KEY_NOT_FOUND);
                }

                Value value = *getValueAtKey(obj, key.as.stringValue);
                pushValue(&vm->stack, value);

                DESTROYIFLITERAL(key);
                break;
            }



            case OP_LOAD_FAST: {
                uint16_t index = NEXT_SHORT() + PEEK_STACK();
                Value local = vm->stack.values[index];

                Value copy = hardCopyValue(local);
                pushValue(&vm->stack, copy);
                break;
            }
            case OP_STORE_FAST: {
                uint16_t index = NEXT_SHORT() + PEEK_STACK();
                Value value = POP_VALUE();
                
                value = hardCopyValue(value);

                if (!vm->stack.values[index].is_variable_type && vm->stack.values[index].defined) {
                    DataType type = vm->stack.values[index].type;
                    ErrorType castRes = castValue(&value, type);
                    if (castRes != E_NULL) {
                        PRINT_ERROR(castRes);
                    }
                } else if (vm->stack.values[index].defined) {
                    value.is_variable_type = true;
                }

                // destroy old value
                destroyValue(&vm->stack.values[index]);

                vm->stack.values[index] = value; // store to local
                markDefined(&vm->stack.values[index]);
                break;
            }

            case OP_FOR_ITER: {
                uint16_t offset = NEXT_SHORT();
                Value* index = &vm->stack.values[vm->stack.count - 1];
                Value list = PEEK_VALUE_TWO();
                if (list.type != TYPE_ARRAY) {
                    PRINT_ERROR(E_NOT_AN_ARRAY);
                }
                
                int i = ++index->as.longValue;
                ValueArray* array = (ValueArray*)list.as.objectValue;
                if (i >= array->count || i < 0) {
                    destroyValue(&POP_VALUE()); // pop index
                    destroyValue(&POP_VALUE()); // pop list
                    vm->ip = offset + active_instance->code;
                } else {
                    // push iterator
                    Value value = array->values[i];
                    value = hardCopyValue(value);
                    pushValue(&vm->stack, value);
                }
                
                break;
            }

            case OP_PUSH_NULL: {
                pushValue(&vm->stack, UNDEFINED_VALUE);
                break;
            }
            case OP_PUSH_MINUS_ONE: {
                Value val = (Value){ TYPE_LONG, .as.longValue = -1, .is_variable_type = false };
                pushValue(&vm->stack, val);
                break;
            }

            case OP_LOAD: {
                uint16_t index = NEXT_SHORT();
                Value global = vm->globals.values[index];
                
                if (!global.defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }
                
                Value copy = hardCopyValue(global);

                pushValue(&vm->stack, copy);
                break;
            }
            case OP_STORE: {
                uint16_t index = NEXT_SHORT();
                Value value = POP_VALUE();
                if (!vm->globals.values[index].defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                value = hardCopyValue(value);
                
                if (!vm->globals.values[index].is_variable_type) {
                    DataType type = vm->globals.values[index].type;
                    ErrorType castRes = castValue(&value, type);
                    if (castRes != E_NULL) {
                        PRINT_ERROR(castRes);
                    }
                } else {
                    value.is_variable_type = true;
                }
                

                // destroy old value
                destroyValue(&vm->globals.values[index]);

                vm->globals.values[index] = value;
                markDefined(&vm->globals.values[index]);
                break;
            }

            case OP_DEFINE: {
                uint16_t index = NEXT_SHORT();
                Value value = POP_VALUE();
                if (vm->globals.values[index].defined) {
                    PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE);
                }

                value = hardCopyValue(value);

                vm->globals.values[index] = value;
                markDefined(&vm->globals.values[index]);
                break;
            }

            case OP_STORE_SUBSCR: {
                Value index = POP_VALUE();
                Value list = POP_VALUE();
                Value value = POP_VALUE();
                if (list.type != TYPE_ARRAY && list.type != TYPE_STRING) {
                    PRINT_ERROR(E_NOT_AN_ARRAY);
                }

                if (!list.defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                // TO DO type checking
                ErrorType code = castValue(&index, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                long long int i = index.as.longValue;
                if (list.type == TYPE_ARRAY) {
                    ValueArray* array = (ValueArray*)list.as.objectValue;
                    if (i < 0 || i >= array->count) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }

                    // destroy old value
                    destroyValue(&array->values[i]);

                    array->values[i] = value;
                    markDefined(&array->values[i]);
                } else if (list.type == TYPE_STRING) {
                    char* string = list.as.stringValue;
                    if (i < 0 || i >= strlen(string)) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }

                    ErrorType code = castValue(&value, TYPE_CHAR);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }

                    string[i] = value.as.charValue;
                }

                DESTROYIFLITERAL(index);
                break;
            }

            
            case OP_STORE_OBJ: {
                Value key = POP_VALUE();
                Value object = POP_VALUE();
                Value value = POP_VALUE();

                if (object.type != TYPE_OBJECT) {
                    PRINT_ERROR(E_NOT_AN_OBJECT);
                }

                markDefined(&value);

                ValueObject* obj = (ValueObject*)object.as.objectValue;
                if (!hasKey(obj, key.as.stringValue)) {
                    // add key
                    write_ValueObject(obj, key.as.stringValue, value);
                } else {
                    // destroy old value
                    removeFromKey(obj, key.as.stringValue);

                    // set new value
                    write_ValueObject(obj, key.as.stringValue, value);
                }

                break;
            }

            case OP_BUILD_LIST: {
                int count = NEXT_SHORT();
                Value list = (Value){ TYPE_ARRAY, .as.objectValue = malloc(sizeof(ValueArray)) };
                init_ValueArray((ValueArray*)list.as.objectValue);
                for (int i = 0; i < count; i++) {
                    Value value = POP_VALUE();
                    write_ValueArray((ValueArray*)list.as.objectValue, value);
                }
                pushValue(&vm->stack, list);
                break;
            }

            case OP_BUILD_OBJECT: {
                int count = NEXT_SHORT();
                Value object = (Value){ TYPE_OBJECT, .as.objectValue = malloc(sizeof(ValueObject)) };

                ValueObject* obj = (ValueObject*)object.as.objectValue;
                init_ValueObject(obj);
                for (int i = 0; i < count; i++) {
                    Value value = POP_VALUE();
                    Value key = POP_VALUE();
                    if (key.type != TYPE_STRING) {
                        PRINT_ERROR(E_INVALID_KEY_TYPE);
                    }
                    if (hasKey(obj, key.as.stringValue)) {
                        PRINT_ERROR(E_KEY_ALREADY_DEFINED);
                    }
                    write_ValueObject(obj, key.as.stringValue, value);
                }
                pushValue(&vm->stack, object);
                break;
            }


            case OP_INCREMENT: {
                uint16_t index = NEXT_SHORT();
                if (!vm->globals.values[index].defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                ErrorType code = incValue(&vm->globals.values[index], 1);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                break;
            }

            case OP_DECREMENT: {
                uint16_t index = NEXT_SHORT();
                if (!vm->globals.values[index].defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                ErrorType code = incValue(&vm->globals.values[index], -1);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                break;
            }

            case OP_INCREMENT_FAST: {
                uint16_t index = NEXT_SHORT() + PEEK_STACK();
                if (!vm->stack.values[index].defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                ErrorType code = incValue(&vm->stack.values[index], 1);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                break;
            }

            case OP_DECREMENT_FAST: {
                uint16_t index = NEXT_SHORT() + PEEK_STACK();
                if (!vm->stack.values[index].defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                // TO DO type checking
                ErrorType code = incValue(&vm->stack.values[index], -1);
                break;
            }

            case OP_INCREMENT_SUBSCR: {
                Value index = POP_VALUE();
                Value list = POP_VALUE();
                if (list.type != TYPE_ARRAY) {
                    PRINT_ERROR(E_NOT_AN_ARRAY);
                }
                if (!list.defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                // TO DO type checking
                int i = index.as.longValue;
                ValueArray* array = (ValueArray*)list.as.objectValue;
                if (i < 0 || i >= array->count) {
                    PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                }

                // TO DO type checking
                ErrorType code = incValue(&array->values[i], 1);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                break;
            }

            case OP_DECREMENT_SUBSCR: {
                Value index = POP_VALUE();
                Value list = POP_VALUE();
                if (list.type != TYPE_ARRAY) {
                    PRINT_ERROR(E_NOT_AN_ARRAY);
                }
                if (!list.defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                // TO DO type checking
                int i = index.as.longValue;
                ValueArray* array = (ValueArray*)list.as.objectValue;
                if (i < 0 || i >= array->count) {
                    PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                }

                // TO DO type checking
                ErrorType code = incValue(&array->values[i], -1);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                break;
            }

            case OP_INCREMENT_OBJ: {
                Value key = POP_VALUE();
                Value object = POP_VALUE();
                if (object.type != TYPE_OBJECT) {
                    PRINT_ERROR(E_NOT_AN_OBJECT);
                }

                ValueObject* obj = (ValueObject*)object.as.objectValue;
                if (!hasKey(obj, key.as.stringValue)) {
                    PRINT_ERROR(E_KEY_NOT_FOUND);
                }

                Value* value = getValueAtKey(obj, key.as.stringValue);
                // to do type checking
                ErrorType code = incValue(value, 1);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                break;
            }

            case OP_DECREMENT_OBJ: {
                Value key = POP_VALUE();
                Value object = POP_VALUE();
                if (object.type != TYPE_OBJECT) {
                    PRINT_ERROR(E_NOT_AN_OBJECT);
                }

                ValueObject* obj = (ValueObject*)object.as.objectValue;
                if (!hasKey(obj, key.as.stringValue)) {
                    PRINT_ERROR(E_KEY_NOT_FOUND);
                }

                Value* value = getValueAtKey(obj, key.as.stringValue);
                // to do type checking
                ErrorType code = incValue(value, -1);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                break;
            }
            

            case OP_TYPE_CAST: {
                Value val = POP_VALUE();
                DataType type = NEXT_BYTE();
                
                ErrorType error = castValue(&val, type);
                if (error != E_NULL) {
                    PRINT_ERROR(error);
                }
                pushValue(&vm->stack, val);

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
                destroyValue(&POP_VALUE());
                break;
            }

            case OP_STOP: {
                halt = true;
                break;
            }
            case OP_RETURN: {
                Value return_value = POP_VALUE();
                int pop_amount = NEXT_BYTE();
                for (int i = 0; i < pop_amount; i++) {
                    destroyValue(&POP_VALUE());
                }

                // pop frame
                size_t frame = POP_STACK();

                // pop ip
                vm->ip = ip_stack[--ip_stack_count];
                active_instance = active_stack[ip_stack_count];

                // push return value
                pushValue(&vm->stack, return_value);
                break;
            }

            // arithmetic

            case OP_BINARY_ADD: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (a.type == TYPE_STRING || b.type == TYPE_STRING) {
                    char* a_str = valueToString(a, false);
                    char* b_str = valueToString(b, false);
                    char* new_string = malloc(strlen(a_str) + strlen(b_str) + 1);
                    strcpy(new_string, a_str);
                    strcat(new_string, b_str);
                    free(a_str);
                    free(b_str);
                    Value result = (Value){ TYPE_STRING, .as.stringValue = new_string };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    Value result = (Value){ TYPE_DOUBLE, .as.doubleValue = a.as.doubleValue + b.as.doubleValue };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if (a.type == TYPE_ARRAY && b.type == TYPE_ARRAY) {
                    ValueArray* a_array = (ValueArray*)a.as.objectValue;
                    ValueArray* b_array = (ValueArray*)b.as.objectValue;
                    ValueArray* new_array = malloc(sizeof(ValueArray));
                    init_ValueArray(new_array);
                    for (int i = 0; i < a_array->count; i++) {
                        Value val = hardCopyValue(a_array->values[i]);
                        write_ValueArray(new_array, val);
                    }
                    for (int i = 0; i < b_array->count; i++) {
                        Value val = hardCopyValue(b_array->values[i]);
                        write_ValueArray(new_array, val);
                    }
                    Value result = (Value){ TYPE_ARRAY, .as.objectValue = new_array };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if (a.type == TYPE_OBJECT && b.type == TYPE_OBJECT) {
                    ValueObject* a_obj = (ValueObject*)a.as.objectValue;
                    ValueObject* b_obj = (ValueObject*)b.as.objectValue;
                    ValueObject* new_obj = malloc(sizeof(ValueObject));
                    init_ValueObject(new_obj);
                    for (int i = 0; i < a_obj->count; i++) {
                        Value val = hardCopyValue(a_obj->values[i]);
                        write_ValueObject(new_obj, a_obj->keys[i], val);
                    }
                    for (int i = 0; i < b_obj->count; i++) {
                        if (hasKey(new_obj, b_obj->keys[i])) {
                            PRINT_ERROR(E_KEY_ALREADY_DEFINED);
                        }
                        Value val = hardCopyValue(b_obj->values[i]);
                        write_ValueObject(new_obj, b_obj->keys[i], val);
                    }
                    Value result = (Value){ TYPE_OBJECT, .as.objectValue = new_obj };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if ((ISINTTYPE(a.type) || a.type == TYPE_CHAR || a.type == TYPE_BOOL) && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    long long int result = a.as.longValue + b.as.longValue;
                    pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else {
                    PRINT_ERROR(E_CANT_PERFORM_BINARY_OPERATION);
                }
                break;
            }
            case OP_BINARY_SUBTRACT: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    Value result = (Value){ TYPE_DOUBLE, .as.doubleValue = a.as.doubleValue - b.as.doubleValue };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if (a.type == TYPE_ARRAY && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ValueArray* a_array = (ValueArray*)a.as.objectValue;
                    ErrorType code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    long long int pop_amount = b.as.longValue;
                    if (pop_amount < 0) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }
                    if (pop_amount > a_array->count) {
                        pop_amount = a_array->count;
                    }
                    ValueArray* new_array = malloc(sizeof(ValueArray));
                    init_ValueArray(new_array);
                    for (int i = 0; i < a_array->count - pop_amount; i++) {
                        Value val = hardCopyValue(a_array->values[i]);
                        write_ValueArray(new_array, val);
                    }
                    Value result = (Value){ TYPE_ARRAY, .as.objectValue = new_array };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if (a.type == TYPE_OBJECT && b.type == TYPE_STRING) {
                    ValueObject* a_obj = (ValueObject*)a.as.objectValue;
                    char* key_to_remove = b.as.stringValue;
                    if (!hasKey(a_obj, key_to_remove)) {
                        PRINT_ERROR(E_KEY_NOT_FOUND);
                    }
                    removeFromKey(a_obj, key_to_remove);
                    pushValue(&vm->stack, a);
                    DESTROYIFLITERAL(b);
                } else if ((ISINTTYPE(a.type) || a.type == TYPE_CHAR || a.type == TYPE_BOOL) && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    long long int result = a.as.longValue - b.as.longValue;
                    pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else {
                    PRINT_ERROR(E_CANT_PERFORM_BINARY_OPERATION);
                }
                break;
            }
            case OP_BINARY_MULTIPLY: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    Value result = (Value){ TYPE_DOUBLE, .as.doubleValue = a.as.doubleValue * b.as.doubleValue };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if (a.type == TYPE_ARRAY && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ValueArray* a_array = (ValueArray*)a.as.objectValue;
                    ErrorType code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    long long int multiply_amount = b.as.longValue;
                    if (multiply_amount < 0) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }
                    ValueArray* new_array = malloc(sizeof(ValueArray));
                    init_ValueArray(new_array);
                    for (int i = 0; i < multiply_amount; i++) {
                        for (int j = 0; j < a_array->count; j++) {
                            Value val = hardCopyValue(a_array->values[j]);
                            write_ValueArray(new_array, val);
                        }
                    }
                    Value result = (Value){ TYPE_ARRAY, .as.objectValue = new_array };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } if ((ISINTTYPE(a.type) || a.type == TYPE_CHAR || a.type == TYPE_BOOL) && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    long long int result = a.as.longValue * b.as.longValue;
                    pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else {
                    PRINT_ERROR(E_CANT_PERFORM_BINARY_OPERATION);
                }
                break;
            }
            case OP_BINARY_DIVIDE: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    if (b.as.doubleValue == 0) {
                        PRINT_ERROR(E_MATH_DOMAIN_ERROR);
                    }
                    Value result = (Value){ TYPE_DOUBLE, .as.doubleValue = a.as.doubleValue / b.as.doubleValue };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if ((ISINTTYPE(a.type) || a.type == TYPE_CHAR || a.type == TYPE_BOOL) && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    if (b.as.longValue == 0) {
                        PRINT_ERROR(E_MATH_DOMAIN_ERROR);
                    }
                    long long int result = a.as.longValue / b.as.longValue;
                    pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else {
                    PRINT_ERROR(E_CANT_PERFORM_BINARY_OPERATION);
                }
                break;
            }
            case OP_BINARY_GREATER: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = a.as.doubleValue > b.as.doubleValue });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if ((ISINTTYPE(a.type) || a.type == TYPE_CHAR || a.type == TYPE_BOOL) && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = a.as.longValue > b.as.longValue });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else {
                    PRINT_ERROR(E_CANT_PERFORM_BINARY_OPERATION);
                }
                break;
            }
            case OP_BINARY_LESS: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = a.as.doubleValue < b.as.doubleValue });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if ((ISINTTYPE(a.type) || a.type == TYPE_CHAR || a.type == TYPE_BOOL) && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = a.as.longValue < b.as.longValue });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else {
                    PRINT_ERROR(E_CANT_PERFORM_BINARY_OPERATION);
                }
                break;
            }
            case OP_BINARY_EQUAL: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                bool result = valueEquals(&a, &b);
                pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = result });
                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_NOT_EQUAL: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                bool result = !valueEquals(&a, &b);
                pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = result });
                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_GREATER_EQUAL: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = a.as.doubleValue >= b.as.doubleValue });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if ((ISINTTYPE(a.type) || a.type == TYPE_CHAR || a.type == TYPE_BOOL) && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = a.as.longValue >= b.as.longValue });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else {
                    PRINT_ERROR(E_CANT_PERFORM_BINARY_OPERATION);
                }
                break;
            }
            case OP_BINARY_LESS_EQUAL: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = a.as.doubleValue <= b.as.doubleValue });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else if ((ISINTTYPE(a.type) || a.type == TYPE_CHAR || a.type == TYPE_BOOL) && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = a.as.longValue <= b.as.longValue });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                    break;
                } else {
                    PRINT_ERROR(E_CANT_PERFORM_BINARY_OPERATION);
                }
                break;
            }
            case OP_BINARY_LOGICAL_AND: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_BOOL);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                code = castValue(&b, TYPE_BOOL);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }

                pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = a.as.boolValue && b.as.boolValue });
                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_LOGICAL_OR: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_BOOL);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                code = castValue(&b, TYPE_BOOL);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }

                pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = a.as.boolValue || b.as.boolValue });
                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_MODULO: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    double result = fmod(a.as.doubleValue, b.as.doubleValue);
                    pushValue(&vm->stack, (Value){ TYPE_DOUBLE, .as.doubleValue = result });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                } else {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    long long int result = a.as.longValue % b.as.longValue;
                    pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                }
                break;
            }
            case OP_BINARY_POWER: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    double result = pow(a.as.doubleValue, b.as.doubleValue);
                    pushValue(&vm->stack, (Value){ TYPE_DOUBLE, .as.doubleValue = result });

                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                } else {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    long long int result = powl(a.as.longValue, b.as.longValue);
                    pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });
                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                }
                break;
            }
            case OP_BINARY_ROOT: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_DOUBLE);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                code = castValue(&b, TYPE_DOUBLE);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                if (b.as.doubleValue < 0) {
                    PRINT_ERROR(E_MATH_DOMAIN_ERROR);
                }
                double result = 0;
                if (b.as.doubleValue > 0) {
                    result = powl(b.as.doubleValue, 1.0 / a.as.doubleValue);
                }
                pushValue(&vm->stack, (Value){ TYPE_DOUBLE, .as.doubleValue = result });

                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_SHIFT_LEFT: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                code = castValue(&b, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                long long int result = a.as.longValue << b.as.longValue;
                pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });

                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_SHIFT_RIGHT: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                code = castValue(&b, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                long long int result = a.as.longValue >> b.as.longValue;
                pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });

                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_AND_BITWISE: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                code = castValue(&b, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                long long int result = a.as.longValue & b.as.longValue;
                pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });

                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_OR_BITWISE: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                code = castValue(&b, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                long long int result = a.as.longValue | b.as.longValue;
                pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });

                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_XOR_BITWISE: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                code = castValue(&b, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                long long int result = a.as.longValue ^ b.as.longValue;
                pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });

                DESTROYIFLITERAL(a);
                DESTROYIFLITERAL(b);
                break;
            }
            case OP_BINARY_MIN: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    double result = fmin(a.as.doubleValue, b.as.doubleValue);
                    pushValue(&vm->stack, (Value){ TYPE_DOUBLE, .as.doubleValue = result });

                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                } else {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    long long int result = a.as.longValue < b.as.longValue ? a.as.longValue : b.as.longValue;
                    pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });

                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                }
                break;
            }
            case OP_BINARY_MAX: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
                    ErrorType code = castValue(&a, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_DOUBLE);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    double result = fmax(a.as.doubleValue, b.as.doubleValue);
                    pushValue(&vm->stack, (Value){ TYPE_DOUBLE, .as.doubleValue = result });

                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                } else {
                    ErrorType code = castValue(&a, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    long long int result = a.as.longValue > b.as.longValue ? a.as.longValue : b.as.longValue;
                    pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });

                    DESTROYIFLITERAL(a);
                    DESTROYIFLITERAL(b);
                }
                break;
            }
            
            // unary operations

            case OP_UNARY_NEGATE: {
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type)) {
                    Value result = (Value){ TYPE_DOUBLE, .as.doubleValue = -a.as.doubleValue };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                } else if (ISINTTYPE(a.type)) {
                    Value result = (Value){ TYPE_LONG, .as.longValue = -a.as.longValue };
                    pushValue(&vm->stack, result);
                    DESTROYIFLITERAL(a);
                } else {
                    PRINT_ERROR(E_CANT_PERFORM_UNARY_OPERATION);
                }
                break;
            }
            case OP_UNARY_LOGICAL_NOT: {
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_BOOL);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }

                pushValue(&vm->stack, (Value){ TYPE_BOOL, .as.boolValue = !a.as.boolValue });
                DESTROYIFLITERAL(a);
                break;
            }
            case OP_UNARY_BITWISE_NOT: {
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }

                pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = ~a.as.longValue });
                DESTROYIFLITERAL(a);
                break;
            }
            case OP_UNARY_SQRT: {
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_DOUBLE);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }
                if (a.as.doubleValue < 0) {
                    PRINT_ERROR(E_MATH_DOMAIN_ERROR);
                }
                double result = sqrt(a.as.doubleValue);
                pushValue(&vm->stack, (Value){ TYPE_DOUBLE, .as.doubleValue = result });

                DESTROYIFLITERAL(a);
                break;
            }
            case OP_UNARY_ABSOLUTE: {
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type)) {
                    double result = fabs(a.as.doubleValue);
                    pushValue(&vm->stack, (Value){ TYPE_DOUBLE, .as.doubleValue = result });
                    DESTROYIFLITERAL(a);
                } else if (ISINTTYPE(a.type)) {
                    long long int result = llabs(a.as.longValue);
                    pushValue(&vm->stack, (Value){ TYPE_LONG, .as.longValue = result });
                    DESTROYIFLITERAL(a);
                } else {
                    PRINT_ERROR(E_CANT_PERFORM_UNARY_OPERATION);
                }
                break;
            }
        }

    }
    return 0;
}

#undef PRINT_ERROR