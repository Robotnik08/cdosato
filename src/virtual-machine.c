#include "../include/common.h"

#include "../include/virtual-machine.h"
#include "../include/error.h"
#include "../include/ast.h"
#include "../include/memory.h"
#include "../include/dynamic_library_loader.h"

VirtualMachine* main_vm = NULL;

DOSATO_LIST_FUNC_GEN(FunctionList, Function, funcs)


DosatoObject* buildDosatoObject(void* body, DataType type, bool sweep, void* vm) {
    VirtualMachine* vm_intance = (VirtualMachine*)vm;

    DosatoObject* object = malloc(sizeof(DosatoObject));
    object->body = body;
    object->type = type;
    object->marked = true; // immune to garbage collection for the first sweep, to ensure values that are not on the stack are not deleted (for arithmetics)

    vm_intance->allocated_objects[vm_intance->allocated_objects_count++] = object;

    if (vm_intance->allocated_objects_count >= vm_intance->allocated_objects_capacity) {
        // mark and sweep
        if (sweep) {
            markObjects(vm_intance);
            sweepObjects(vm_intance);
        }
        // set new capacity to double the amount of allocated objects left
        vm_intance->allocated_objects_capacity = __max(vm_intance->allocated_objects_count * 3 / 2, GC_MIN_THRESHOLD);
        vm_intance->allocated_objects = realloc(vm_intance->allocated_objects, sizeof(DosatoObject*) * vm_intance->allocated_objects_capacity);
    }
    
    return object;
}

void markObjects (VirtualMachine* vm) {
    for (size_t i = 0; i < vm->stack.count; i++) {
        markValue(&vm->stack.values[i]);
    }

    for (size_t i = 0; i < vm->globals.count; i++) {
        markValue(&vm->globals.values[i]);
    }

    for (size_t i = 0; i < vm->constants.count; i++) {
        markValue(&vm->constants.values[i]);
    }
}

void sweepObjects (VirtualMachine* vm) {
    for (size_t i = 0; i < vm->allocated_objects_count; i++) {
        DosatoObject* object = vm->allocated_objects[i];
        if (!object->marked) {
            if (object->type == TYPE_ARRAY) {
                free_ValueArray((ValueArray*)object->body);
            } else if (object->type == TYPE_OBJECT) {
                free_ValueObject((ValueObject*)object->body);
            }

            if (object->type != TYPE_FUNCTION) {
                free(object->body);
            }
            free(object);
            
            for (size_t j = i; j < vm->allocated_objects_count - 1; j++) {
                vm->allocated_objects[j] = vm->allocated_objects[j + 1];
            }
            vm->allocated_objects_count--;
            i--; // recheck this index
        } else {
            object->marked = false;
        }
    }
}

void markValue(Value* value) {
    if (value->type == TYPE_ARRAY) {
        DosatoObject* object = value->as.objectValue;
        if (object->marked) return; // already marked
        object->marked = true;
        ValueArray* array = AS_ARRAY(*value);
        for (size_t i = 0; i < array->count; i++) {
            markValue(&array->values[i]);
        }
    } else if (value->type == TYPE_OBJECT) {
        DosatoObject* object = value->as.objectValue;
        if (object->marked) return; // already marked
        object->marked = true;
        ValueObject* objectList = AS_OBJECT(*value);
        for (size_t i = 0; i < objectList->count; i++) {
            markValue(&objectList->values[i]);
        }
    } else if (value->type == TYPE_STRING) {
        DosatoObject* object = value->as.objectValue;
        object->marked = true;
    } else if (value->type == TYPE_FUNCTION) {
        DosatoObject* object = value->as.objectValue;
        object->marked = true;
    }
}

void destroy_Function(Function* func) {
    free(func->name);
    if (!func->is_compiled) {
        free(func->argv);
        free(func->argt);
        freeCodeInstanceWeak(func->instance);
        free(func->instance);
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

    vm->allocated_objects = malloc(sizeof(DosatoObject*) * GC_MIN_THRESHOLD);
    vm->allocated_objects_count = 0;
    vm->allocated_objects_capacity = GC_MIN_THRESHOLD;
}

void freeVirtualMachine(VirtualMachine* vm) {
    freeCodeInstance(vm->instance);
    free(vm->instance);
    free_StackFrames(&vm->stack_frames);
    destroy_FunctionList(&vm->functions);
    free_NameMap(&vm->mappings);
    free_NameMap(&vm->constants_map);
    free_ErrorJumps(&vm->error_jumps);
    destroy_CodeInstanceList(&vm->includes);

    sweepObjects(vm); // sweep all objects
    sweepObjects(vm); // sweep remaining objects

    free_ValueArray(&vm->globals);
    free_ValueArray(&vm->stack);
    free_ValueArray(&vm->constants);

    free(vm->allocated_objects);
}

void pushValue(ValueArray* array, Value value) {
    write_ValueArray(array, value);
}

#define PRINT_ERROR(e_code) do { \
    if (vm->error_jumps.count > 0) { \
        ErrorJump jump = vm->error_jumps.jumps[--vm->error_jumps.count]; \
        while (vm->stack.count > jump.error_stack_count) { \
            POP_VALUE(); \
        } \
        vm->ip = jump.error_jump_loc; \
        vm->globals.values[0].as.longValue = e_code; \
        ip_stack_count = jump.error_active_index_count; \
        if (jump.error_active_index_count > 0) { \
            active_instance = active_stack[ip_stack_count - 1]; \
        } \
    } else { \
        size_t token_index = active_instance->token_indices[vm->ip - active_instance->code - 1]; \
        printError(((AST*)active_instance->ast)->source, ((AST*)active_instance->ast)->tokens.tokens[token_index].start - ((AST*)active_instance->ast)->source, ((AST*)active_instance->ast)->name, e_code, ((AST*)active_instance->ast)->tokens.tokens[token_index].length);\
    } \
} while(0); \
break;

int runVirtualMachine (VirtualMachine* vm, int debug) {
    if (debug) printf("Running virtual machine\n");
    bool halt = false;
    vm->ip = vm->instance->code;
    CodeInstance* active_instance = vm->instance;
    char* ip_stack[RECURSION_LIMIT];
    CodeInstance* active_stack[RECURSION_LIMIT];
    size_t ip_stack_count = 0;

    // set global variable 0 to zero (underscore)
    Value zero = BUILD_LONG(0);
    vm->globals.values[0] = zero;

    // set all functions to globals
    for (size_t i = 0; i < vm->functions.count; i++) {
        Value func = BUILD_FUNCTION(&vm->functions.funcs[i], false);
        func.defined = true;
        func.is_constant = true;
        if (vm->functions.funcs[i].name_index != -1) {
            vm->globals.values[vm->functions.funcs[i].name_index] = func;
        }
    }

    while (!halt) {
        OpCode instruction = NEXT_BYTE();
        switch (instruction) {
            
            default: {
                printf("Unknown instruction: %d at: %x\n", instruction, vm->ip - active_instance->code - 1);
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
                break;
            }

            case OP_JUMP_IF_TRUE: {
                uint16_t offset = NEXT_SHORT();
                Value condition = POP_VALUE();
                if (condition.as.boolValue == true) {
                    vm->ip = offset + active_instance->code;
                }
                break;
            }

            case OP_JUMP_IF_MATCH: {
                uint16_t offset = NEXT_SHORT();
                Value match = POP_VALUE();
                Value value = PEEK_VALUE();

                // check if the two values are equal
                if (valueEquals(&match, &value)) {
                    // pop the value
                    POP_VALUE();
                    vm->ip = offset + active_instance->code;
                }
                
                break;
            }

            case OP_JUMP_IF_EXCEPTION: {
                uint16_t offset = NEXT_SHORT();
                size_t stack_count = vm->stack.count;
                write_ErrorJumps(&vm->error_jumps, (ErrorJump){ .error_jump_loc = active_instance->code + offset, .error_stack_count = stack_count, .error_active_index_count = ip_stack_count });
                break;
            }

            case OP_CONTINUE: {
                uint16_t offset = NEXT_SHORT();
                uint16_t pop_amount = NEXT_SHORT();
                vm->ip = offset + active_instance->code;
                for (size_t i = 0; i < pop_amount; i++) {
                    POP_VALUE();
                }
                break;
            }

            case OP_BREAK: {
                uint16_t offset = NEXT_SHORT();
                uint16_t pop_amount = NEXT_SHORT();
                vm->ip = offset + active_instance->code;
                
                for (size_t i = 0; i < pop_amount; i++) {
                    POP_VALUE();
                }

                if (*vm->ip == OP_FOR_ITER) {
                    // set the iterator to -2 (so the loop will end)
                    vm->stack.values[vm->stack.count - 1].as.longValue = -2;
                } else if ((*vm->ip == OP_JUMP_IF_FALSE) || (*vm->ip == OP_JUMP)) { 
                    NEXT_BYTE(); // skip the offset
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
                Function* function = AS_FUNCTION(func);
                if (function->is_compiled) {
                    // call the compiled function
                    ValueArray args;
                    init_ValueArray(&args);
                    for (int i = 0; i < arity; i++) {
                        pushValue(&args, POP_VALUE());
                    }
                    main_vm = vm;
                    // store old ip
                    uint8_t* old_ip = vm->ip;
                    Value return_val = ((DosatoFunction)function->func_ptr)(args, debug);
                    free_ValueArray(&args);
                    CodeInstance* old_instance = vm->instance;
                    vm->instance = active_instance;
                    vm->ip = old_ip;
                    if (return_val.type == TYPE_EXCEPTION) {
                        PRINT_ERROR(return_val.as.longValue);
                    }
                    if (return_val.type == TYPE_HLT) {
                        halt = true;
                        exit(return_val.as.longValue);
                        return return_val.as.longValue;
                    }
                    pushValue(&vm->stack, return_val);
                    vm->instance = old_instance;
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
                if (ip_stack_count == 0) {
                    halt = true;
                    break;
                }

                vm->ip = ip_stack[--ip_stack_count];
                active_instance = active_stack[ip_stack_count];

                // push return value (NULL because END_FUNC is always at the end of a function)
                pushValue(&vm->stack, UNDEFINED_VALUE);
                break;
            }


            case OP_LOAD_CONSTANT: {
                uint16_t index = NEXT_SHORT();
                Value constant = vm->constants.values[index];

                constant.is_constant = false;

                if (constant.type == TYPE_STRING) {
                    // copy the string so the original pointer stays intact
                    constant = BUILD_STRING(COPY_STRING(AS_STRING(constant)), true);
                }
                pushValue(&vm->stack, constant);
                break;
            }

            case OP_PUSH_FALSE: {
                Value value = BUILD_BOOL(false);
                pushValue(&vm->stack, value);
                break;
            }

            case OP_PUSH_TRUE: {
                Value value = BUILD_BOOL(true);
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
                    ValueArray* array = AS_ARRAY(list);
                    if (i < 0 || i >= array->count) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }

                    Value value = array->values[i];

                    pushValue(&vm->stack, value);
                } else if (list.type == TYPE_STRING) {
                    char* string = AS_STRING(list);
                    if (i < 0 || i >= strlen(string)) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }

                    char val = string[i];
                    Value value = BUILD_CHAR(val);
                    pushValue(&vm->stack, value);
                }

                break;
            }

            case OP_GETOBJECT: {
                Value key = POP_VALUE();
                Value object = POP_VALUE();
                if (object.type != TYPE_OBJECT) {
                    PRINT_ERROR(E_NOT_AN_OBJECT);
                }

                // cast key to string
                if (key.type != TYPE_STRING) {
                    ErrorType code = castValue(&key, TYPE_STRING);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                }

                ValueObject* obj = AS_OBJECT(object);
                if (!hasKey(obj, AS_STRING(key))) {
                    // push NULL
                    pushValue(&vm->stack, UNDEFINED_VALUE);
                    break;
                }

                Value value = *getValueAtKey(obj, AS_STRING(key));

                pushValue(&vm->stack, value);

                break;
            }
            
            case OP_GETOBJECT_SAFE: {
                Value key = POP_VALUE();
                Value object = POP_VALUE();
                if (object.type != TYPE_OBJECT) {
                    // push NULL
                    pushValue(&vm->stack, UNDEFINED_VALUE);
                    break;
                }

                // cast key to string
                if (key.type != TYPE_STRING) {
                    ErrorType code = castValue(&key, TYPE_STRING);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                }

                ValueObject* obj = AS_OBJECT(object);
                if (!hasKey(obj, AS_STRING(key))) {
                    // push NULL
                    pushValue(&vm->stack, UNDEFINED_VALUE);
                    break;
                }

                Value value = *getValueAtKey(obj, AS_STRING(key));

                pushValue(&vm->stack, value);

                break;
            }

            case OP_LOAD_FAST: {
                uint16_t index = NEXT_SHORT() + PEEK_STACK();
                Value local = vm->stack.values[index];

                pushValue(&vm->stack, local);
                break;
            }
            case OP_STORE_FAST: {
                uint16_t index = NEXT_SHORT() + PEEK_STACK();
                Value value = PEEK_VALUE();

                if (vm->stack.values[index].is_constant) {
                    PRINT_ERROR(E_CANNOT_ASSIGN_TO_CONSTANT);
                }


                if (!vm->stack.values[index].is_variable_type && vm->stack.values[index].defined) {
                    DataType type = vm->stack.values[index].type;
                    ErrorType castRes = castValue(&value, type);
                    if (castRes != E_NULL) {
                        PRINT_ERROR(castRes);
                    }
                } else if (vm->stack.values[index].defined) {
                    value.is_variable_type = true;
                }

                value.is_constant = false;

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
                ValueArray* array = AS_ARRAY(list);
                if (i >= array->count || i < 0) {
                    POP_VALUE(); // pop index
                    POP_VALUE(); // pop list
                    vm->ip = offset + active_instance->code;
                } else {
                    // push iterator
                    Value value = array->values[i];
                    pushValue(&vm->stack, value);
                }
                
                break;
            }

            case OP_FOR_DISCARD: {
                uint16_t offset = NEXT_SHORT();
                Value* index = &vm->stack.values[vm->stack.count - 1];
                Value list = PEEK_VALUE_TWO();
                if (list.type != TYPE_ARRAY) {
                    PRINT_ERROR(E_NOT_AN_ARRAY);
                }
                
                int i = ++index->as.longValue;
                ValueArray* array = AS_ARRAY(list);
                if (i >= array->count || i < 0) {
                    POP_VALUE(); // pop index
                    POP_VALUE(); // pop list
                    vm->ip = offset + active_instance->code;
                }
                // don't push iterator
                break;
            }

            case OP_PUSH_NULL: {
                pushValue(&vm->stack, UNDEFINED_VALUE);
                break;
            }
            case OP_PUSH_INFINITY: {
                Value val = BUILD_DOUBLE(INFINITY);
                pushValue(&vm->stack, val);
                break;
            }
            
            case OP_PUSH_NAN: {
                Value val = BUILD_DOUBLE(NAN);
                pushValue(&vm->stack, val);
                break;
            }

            case OP_PUSH_MINUS_ONE: {
                Value val = BUILD_LONG(-1);
                pushValue(&vm->stack, val);
                break;
            }

            case OP_LOAD: {
                uint16_t index = NEXT_SHORT();
                Value global = vm->globals.values[index];
                
                if (!global.defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                pushValue(&vm->stack, global);

                break;
            }
            case OP_STORE: {
                uint16_t index = NEXT_SHORT();
                Value value = PEEK_VALUE();
                if (!vm->globals.values[index].defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }
                
                if (vm->globals.values[index].is_constant) {
                    PRINT_ERROR(E_CANNOT_ASSIGN_TO_CONSTANT);
                }

                
                if (!vm->globals.values[index].is_variable_type) {
                    DataType type = vm->globals.values[index].type;
                    ErrorType castRes = castValue(&value, type);
                    if (castRes != E_NULL) {
                        PRINT_ERROR(castRes);
                    }
                } else {
                    value.is_variable_type = true;
                }
                
                value.is_constant = false;

                vm->globals.values[index] = value;
                markDefined(&vm->globals.values[index]);

                break;
            }

            case OP_DEFINE: {
                uint16_t index = NEXT_SHORT();
                Value value = PEEK_VALUE();
                if (vm->globals.values[index].defined) {
                    PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE);
                }
                
                vm->globals.values[index] = value;
                markDefined(&vm->globals.values[index]);

                break;
            }

            case OP_STORE_SUBSCR: {
                Value index = POP_VALUE();
                Value list = POP_VALUE();
                Value value = PEEK_VALUE();
                if (list.type != TYPE_ARRAY && list.type != TYPE_STRING) {
                    PRINT_ERROR(E_NOT_AN_ARRAY);
                }

                if (!list.defined) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE);
                }

                ErrorType code = castValue(&index, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }

                long long int i = index.as.longValue;
                if (list.type == TYPE_ARRAY) {
                    ValueArray* array = AS_ARRAY(list);
                    if (i < 0 || i >= array->count) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }

                    array->values[i] = value;
                    markDefined(&array->values[i]);
                } else if (list.type == TYPE_STRING) {
                    if (list.is_constant) {
                        PRINT_ERROR(E_CANNOT_ASSIGN_TO_CONSTANT);
                    }
                    char* string = AS_STRING(list);
                    if (i < 0 || i >= strlen(string)) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }

                    ErrorType code = castValue(&value, TYPE_CHAR);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }

                    string[i] = value.as.charValue;
                }

                break;
            }

            
            case OP_STORE_OBJ: {
                Value key = POP_VALUE();
                Value object = POP_VALUE();
                Value value = PEEK_VALUE();

                if (object.type != TYPE_OBJECT) {
                    PRINT_ERROR(E_NOT_AN_OBJECT);
                }

                // cast key to string
                if (key.type != TYPE_STRING) {
                    ErrorType code = castValue(&key, TYPE_STRING);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                }

                markDefined(&value);

                ValueObject* obj = AS_OBJECT(object);
                if (!hasKey(obj, AS_STRING(key))) {
                    // add key
                    write_ValueObject(obj, AS_STRING(key), value);
                } else {
                    // destroy old value
                    removeFromKey(obj, AS_STRING(key));

                    // set new value
                    write_ValueObject(obj, AS_STRING(key), value);
                }


                break;
            }

            case OP_BUILD_LIST: {
                int count = NEXT_SHORT();
                ValueArray *list = malloc(sizeof(ValueArray));
                init_ValueArray(list);
                for (int i = vm->stack.count - count; i < vm->stack.count; i++) {
                    write_ValueArray(list, vm->stack.values[i]);
                }
                
                vm->stack.count -= count;
                pushValue(&vm->stack, BUILD_ARRAY(list, true));
                break;
            }

            case OP_BUILD_OBJECT: {
                int count = NEXT_SHORT() * 2;

                ValueObject* obj = malloc(sizeof(ValueObject));
                init_ValueObject(obj);
                for (int i = vm->stack.count - count; i < vm->stack.count; i++) {
                    Value key = vm->stack.values[i];
                    Value value = vm->stack.values[++i];
                    if (key.type != TYPE_STRING) {
                        ErrorType code = castValue(&key, TYPE_STRING);
                        if (code != E_NULL) {
                            PRINT_ERROR(code);
                        }
                    }
                    if (hasKey(obj, AS_STRING(key))) {
                        PRINT_ERROR(E_KEY_ALREADY_DEFINED);
                    }
                    write_ValueObject(obj, AS_STRING(key), value);
                }

                vm->stack.count -= count;

                pushValue(&vm->stack, BUILD_OBJECT(obj, true));
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
                ValueArray* array = AS_ARRAY(list);
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
                ValueArray* array = AS_ARRAY(list);
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

                // cast key to string
                if (key.type != TYPE_STRING) {
                    ErrorType code = castValue(&key, TYPE_STRING);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                }

                ValueObject* obj = AS_OBJECT(object);
                if (!hasKey(obj, AS_STRING(key))) {
                    PRINT_ERROR(E_KEY_NOT_FOUND);
                }

                Value* value = getValueAtKey(obj, AS_STRING(key));
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

                // cast key to string
                if (key.type != TYPE_STRING) {
                    ErrorType code = castValue(&key, TYPE_STRING);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                }

                ValueObject* obj = AS_OBJECT(object);
                if (!hasKey(obj, AS_STRING(key))) {
                    PRINT_ERROR(E_KEY_NOT_FOUND);
                }

                Value* value = getValueAtKey(obj, AS_STRING(key));
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

            case OP_MARK_CONSTANT: {
                vm->stack.values[vm->stack.count - 1].is_constant = true;
                break;
            }

            case OP_COPY: {
                pushValue(&vm->stack, hardCopyValue(POP_VALUE()));
                break;
            }

            
            case OP_PRINT: {
                Value value = POP_VALUE();

                printValue(value, false);
                printf("\n");
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
                Value return_value = POP_VALUE();
                int pop_amount = NEXT_BYTE();
                for (int i = 0; i < pop_amount; i++) {
                    POP_VALUE();
                }

                if (ip_stack_count == 0) {
                    pushValue(&vm->stack, return_value);
                    halt = true;
                    break;
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

            /// arithmetic

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
                    Value result = BUILD_STRING(new_string, true);
                    pushValue(&vm->stack, result);
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
                    Value result = BUILD_DOUBLE(a.as.doubleValue + b.as.doubleValue);
                    pushValue(&vm->stack, result);
                    break;
                } else if (a.type == TYPE_ARRAY && b.type == TYPE_ARRAY) {
                    ValueArray* a_array = AS_ARRAY(a);
                    ValueArray* b_array = AS_ARRAY(b);
                    ValueArray* new_array = malloc(sizeof(ValueArray));
                    init_ValueArray(new_array);
                    for (int i = 0; i < a_array->count; i++) {
                        Value val = a_array->values[i];
                        write_ValueArray(new_array, val);
                    }
                    for (int i = 0; i < b_array->count; i++) {
                        Value val = b_array->values[i];
                        write_ValueArray(new_array, val);
                    }
                    Value result = BUILD_ARRAY(new_array, true);
                    pushValue(&vm->stack, result);
                    break;
                } else if (a.type == TYPE_OBJECT && b.type == TYPE_OBJECT) {
                    ValueObject* a_obj = AS_OBJECT(a);
                    ValueObject* b_obj = AS_OBJECT(b);
                    ValueObject* new_obj = malloc(sizeof(ValueObject));
                    init_ValueObject(new_obj);
                    for (int i = 0; i < a_obj->count; i++) {
                        Value val = a_obj->values[i];
                        write_ValueObject(new_obj, a_obj->keys[i], val);
                    }
                    for (int i = 0; i < b_obj->count; i++) {
                        if (hasKey(new_obj, b_obj->keys[i])) {
                            PRINT_ERROR(E_KEY_ALREADY_DEFINED);
                        }
                        Value val = b_obj->values[i];
                        write_ValueObject(new_obj, b_obj->keys[i], val);
                    }
                    Value result = BUILD_OBJECT(new_obj, true);
                    pushValue(&vm->stack, result);
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
                    pushValue(&vm->stack, BUILD_LONG(result));
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
                    Value result = BUILD_DOUBLE(a.as.doubleValue - b.as.doubleValue);
                    pushValue(&vm->stack, result);
                    break;
                } else if (a.type == TYPE_ARRAY && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ValueArray* a_array = AS_ARRAY(a);
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
                        Value val = a_array->values[i];
                        write_ValueArray(new_array, val);
                    }
                    Value result = BUILD_ARRAY(new_array, true);
                    pushValue(&vm->stack, result);
                    break;
                } else if (a.type == TYPE_OBJECT && b.type == TYPE_STRING) {
                    ValueObject* a_obj = AS_OBJECT(a);
                    char* key_to_remove = AS_STRING(b);
                    if (!hasKey(a_obj, key_to_remove)) {
                        PRINT_ERROR(E_KEY_NOT_FOUND);
                    }
                    removeFromKey(a_obj, key_to_remove);
                    pushValue(&vm->stack, a);
                } else if (a.type == TYPE_STRING && (ISINTTYPE(b.type) || ISFLOATTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    char* string = AS_STRING(a);
                    long long int pop_amount = 0;
                    ErrorType code = castValue(&b, TYPE_LONG);
                    if (code != E_NULL) {
                        PRINT_ERROR(code);
                    }
                    pop_amount = b.as.longValue;
                    if (pop_amount < 0) {
                        PRINT_ERROR(E_INDEX_OUT_OF_BOUNDS);
                    }
                    if (pop_amount > strlen(string)) {
                        pop_amount = strlen(string);
                    }
                    char* new_string = malloc(strlen(string) - pop_amount + 1);
                    strncpy(new_string, string, strlen(string) - pop_amount);
                    Value result = BUILD_STRING(new_string, true);
                    pushValue(&vm->stack, result);
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
                    long long int result = a.as.longValue - b.as.longValue;
                    pushValue(&vm->stack, BUILD_LONG(result));
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
                    Value result = BUILD_DOUBLE(a.as.doubleValue * b.as.doubleValue);
                    pushValue(&vm->stack, result);
                    break;
                } else if (a.type == TYPE_ARRAY && (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
                    ValueArray* a_array = AS_ARRAY(a);
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
                            Value val = a_array->values[j];
                            write_ValueArray(new_array, val);
                        }
                    }
                    Value result = BUILD_ARRAY(new_array, true);
                    pushValue(&vm->stack, result);
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
                    pushValue(&vm->stack, BUILD_LONG(result));
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
                    Value result = BUILD_DOUBLE(a.as.doubleValue / b.as.doubleValue);
                    pushValue(&vm->stack, result);
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
                    pushValue(&vm->stack, BUILD_LONG(result));
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
                    pushValue(&vm->stack, BUILD_BOOL(a.as.doubleValue > b.as.doubleValue));
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
                    pushValue(&vm->stack, BUILD_BOOL(a.as.longValue > b.as.longValue));
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
                    pushValue(&vm->stack, BUILD_BOOL(a.as.doubleValue < b.as.doubleValue));
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
                    pushValue(&vm->stack, BUILD_BOOL(a.as.longValue < b.as.longValue));
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
                pushValue(&vm->stack, BUILD_BOOL(result));
                break;
            }
            case OP_BINARY_NOT_EQUAL: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                bool result = !valueEquals(&a, &b);
                pushValue(&vm->stack, BUILD_BOOL(result));
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
                    pushValue(&vm->stack, BUILD_BOOL(a.as.doubleValue >= b.as.doubleValue));
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
                    pushValue(&vm->stack, BUILD_BOOL(a.as.longValue >= b.as.longValue));
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
                    pushValue(&vm->stack, BUILD_BOOL(a.as.doubleValue <= b.as.doubleValue));
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
                    pushValue(&vm->stack, BUILD_BOOL(a.as.longValue <= b.as.longValue));
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

                pushValue(&vm->stack, BUILD_BOOL(a.as.boolValue && b.as.boolValue));
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

                pushValue(&vm->stack, BUILD_BOOL(a.as.boolValue || b.as.boolValue));
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
                    pushValue(&vm->stack, BUILD_DOUBLE(result));
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
                    pushValue(&vm->stack, BUILD_LONG(result));
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
                    if (isnan(result)) {
                        PRINT_ERROR(E_MATH_DOMAIN_ERROR);
                    }
                    pushValue(&vm->stack, BUILD_DOUBLE(result));
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
                    pushValue(&vm->stack, BUILD_LONG(result));
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
                pushValue(&vm->stack, BUILD_DOUBLE(result));
                break;
            }
            case OP_BINARY_ROOT_REVERSE: {
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
                if (a.as.doubleValue < 0) {
                    PRINT_ERROR(E_MATH_DOMAIN_ERROR);
                }
                double result = 0;
                if (a.as.doubleValue > 0) {
                    result = powl(a.as.doubleValue, 1.0 / b.as.doubleValue);
                }
                pushValue(&vm->stack, BUILD_DOUBLE(result));
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
                pushValue(&vm->stack, BUILD_LONG(result));
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
                pushValue(&vm->stack, BUILD_LONG(result));
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
                pushValue(&vm->stack, BUILD_LONG(result));
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
                pushValue(&vm->stack, BUILD_LONG(result));
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
                pushValue(&vm->stack, BUILD_LONG(result));
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
                    pushValue(&vm->stack, BUILD_DOUBLE(result));
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
                    pushValue(&vm->stack, BUILD_LONG(result));
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
                    pushValue(&vm->stack, BUILD_DOUBLE(result));
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
                    pushValue(&vm->stack, BUILD_DOUBLE(result));
                }
                break;
            }
            case OP_BINARY_RANGE_UP: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                int cast_a = castValue(&a, TYPE_LONG);
                if (cast_a != E_NULL) {
                    PRINT_ERROR(cast_a);
                }
                int cast_b = castValue(&b, TYPE_LONG);
                if (cast_b != E_NULL) {
                    PRINT_ERROR(cast_b);
                }

                long long int start = a.as.longValue;

                long long int end = b.as.longValue;

                ValueArray* list = malloc(sizeof(ValueArray));
                init_ValueArray(list);
                for (long long int i = start; i < end; i++) {
                    write_ValueArray(list, BUILD_LONG(i));
                }

                pushValue(&vm->stack, BUILD_ARRAY(list, true));

                break;
            }
            case OP_BINARY_RANGE_DOWN: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                int cast_a = castValue(&a, TYPE_LONG);
                if (cast_a != E_NULL) {
                    PRINT_ERROR(cast_a);
                }
                int cast_b = castValue(&b, TYPE_LONG);
                if (cast_b != E_NULL) {
                    PRINT_ERROR(cast_b);
                }

                long long int start = a.as.longValue;

                long long int end = b.as.longValue;

                ValueArray* list = malloc(sizeof(ValueArray));
                init_ValueArray(list);
                for (long long int i = start; i > end; i--) {
                    write_ValueArray(list, BUILD_LONG(i));
                }

                pushValue(&vm->stack, BUILD_ARRAY(list, true));

                break;
            }
            case OP_BINARY_RANGE_UP_INCLUSIVE: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                int cast_a = castValue(&a, TYPE_LONG);
                if (cast_a != E_NULL) {
                    PRINT_ERROR(cast_a);
                }
                int cast_b = castValue(&b, TYPE_LONG);
                if (cast_b != E_NULL) {
                    PRINT_ERROR(cast_b);
                }

                long long int start = a.as.longValue;

                long long int end = b.as.longValue;

                ValueArray* list = malloc(sizeof(ValueArray));
                init_ValueArray(list);
                for (long long int i = start; i <= end; i++) {
                    write_ValueArray(list, BUILD_LONG(i));
                }

                pushValue(&vm->stack, BUILD_ARRAY(list, true));

                break;
            }
            case OP_BINARY_RANGE_DOWN_INCLUSIVE: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                int cast_a = castValue(&a, TYPE_LONG);
                if (cast_a != E_NULL) {
                    PRINT_ERROR(cast_a);
                }
                int cast_b = castValue(&b, TYPE_LONG);
                if (cast_b != E_NULL) {
                    PRINT_ERROR(cast_b);
                }

                long long int start = a.as.longValue;

                long long int end = b.as.longValue;

                ValueArray* list = malloc(sizeof(ValueArray));
                init_ValueArray(list);
                for (long long int i = start; i >= end; i--) {
                    write_ValueArray(list, BUILD_LONG(i));
                }

                pushValue(&vm->stack, BUILD_ARRAY(list, true));

                break;
            }
            case OP_BINARY_NULL_COALESCE: {
                Value b = POP_VALUE();
                Value a = POP_VALUE();

                if (a.type == D_NULL) {
                    pushValue(&vm->stack, b);
                } else {
                    pushValue(&vm->stack, a);
                }
                break;
            }
            
            // unary operations

            case OP_UNARY_NEGATE: {
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type)) {
                    Value result = BUILD_DOUBLE(-a.as.doubleValue);
                    pushValue(&vm->stack, result);
                } else if (ISINTTYPE(a.type)) {
                    Value result = BUILD_LONG(-a.as.longValue);
                    pushValue(&vm->stack, result);
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

                pushValue(&vm->stack, BUILD_BOOL(!a.as.boolValue));
                break;
            }
            case OP_UNARY_BITWISE_NOT: {
                Value a = POP_VALUE();

                ErrorType code = castValue(&a, TYPE_LONG);
                if (code != E_NULL) {
                    PRINT_ERROR(code);
                }

                pushValue(&vm->stack, BUILD_LONG(~a.as.longValue));
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
                pushValue(&vm->stack, BUILD_DOUBLE(result));
                break;
            }
            case OP_UNARY_ABSOLUTE: {
                Value a = POP_VALUE();

                if (ISFLOATTYPE(a.type)) {
                    double result = fabs(a.as.doubleValue);
                    pushValue(&vm->stack, BUILD_DOUBLE(result));
                } else if (ISINTTYPE(a.type)) {
                    long long int result = llabs(a.as.longValue);
                    pushValue(&vm->stack, BUILD_LONG(result));
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

Value callExternalFunction(size_t index, ValueArray args, bool debug) {
    Function function = main_vm->functions.funcs[index];

    if (function.is_compiled) {
        Value return_val = ((DosatoFunction)function.func_ptr)(args, debug);
        return return_val;
    } else {
        if (args.count != function.arity) {
            return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
        }
        // cast arguments
        for (int i = 0; i < args.count; i++) {
            ErrorType code = castValue(&args.values[i], function.argt[i]);
            if (code != E_NULL) {
                return BUILD_EXCEPTION(code);
            }
        }
        // call function

        CodeInstance* old = main_vm->instance;

        main_vm->instance = function.instance;

        write_StackFrames(&main_vm->stack_frames, main_vm->stack.count);
        // push arguments
        for (int i = 0; i < args.count; i++) {
            pushValue(&main_vm->stack, args.values[i]);
        }

        runVirtualMachine(main_vm, debug);

        main_vm->instance = old;

        return main_vm->stack.values[--main_vm->stack.count];
    }
}
