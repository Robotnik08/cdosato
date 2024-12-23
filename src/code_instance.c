#include "../include/common.h"

#include "../include/memory.h"
#include "../include/code_instance.h"
#include "../include/ast.h"

void init_LocationList(LocationList* list) {
    list->locations = NULL;
    list->stack_count = NULL;
    list->count = 0;
    list->capacity = 0;
}

void write_LocationList(LocationList* list, size_t location, size_t stack_count) {
    if (list->capacity < list->count + 1) {
        size_t oldCapacity = list->capacity;
        list->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        list->locations = DOSATO_RESIZE_LIST(size_t, list->locations, oldCapacity, list->capacity);
        list->stack_count = DOSATO_RESIZE_LIST(size_t, list->stack_count, oldCapacity, list->capacity);
    }
    list->locations[list->count] = location;
    list->stack_count[list->count] = stack_count;
    list->count++;
}

void free_LocationList(LocationList* list) {
    DOSATO_FREE_LIST(size_t, list->locations, list->capacity);
    DOSATO_FREE_LIST(size_t, list->stack_count, list->capacity);
    init_LocationList(list);
}

DOSATO_LIST_FUNC_GEN(CodeInstanceList, CodeInstance, instances)

void destroy_CodeInstanceList(CodeInstanceList* list) {
    for (size_t i = 0; i < list->count; i++) {
        freeCodeInstance(&list->instances[i]);
    }
    free_CodeInstanceList(list);
}


void initCodeInstance(CodeInstance* instance) {
    instance->code = NULL;
    instance->token_indices = NULL;
    init_LocationList(&instance->loop_jump_locations);
    instance->count = 0;
    instance->capacity = 0;
    instance->ast = NULL;
}

void writeByteCode(CodeInstance* instance, uint8_t byte, size_t token_index) {
    if (instance->capacity < instance->count + 1) {
        size_t oldCapacity = instance->capacity;
        instance->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        instance->code = DOSATO_RESIZE_LIST(uint8_t, instance->code, oldCapacity, instance->capacity);
        instance->token_indices = DOSATO_RESIZE_LIST(size_t, instance->token_indices, oldCapacity, instance->capacity);
    }

    instance->code[instance->count] = byte;
    instance->token_indices[instance->count] = token_index;
    instance->count++;
}

void writeInstruction(CodeInstance* instance, size_t token_index, OpCode instruction, ...) {
    writeByteCode(instance, instruction, token_index);
    size_t offset = getOffset(instruction);
    va_list args;
    va_start(args, instruction);
    for (size_t i = 0; i < offset - 1; i++) {
        writeByteCode(instance, va_arg(args, int), token_index);
    }
    va_end(args);
}

void freeCodeInstance(CodeInstance* instance) {
    DOSATO_FREE_LIST(uint8_t, instance->code, instance->capacity);
    DOSATO_FREE_LIST(size_t, instance->token_indices, instance->capacity);
    instance->count = 0;
    free_LocationList(&instance->loop_jump_locations);
    instance->capacity = 0;
    free_AST(instance->ast);
    free(instance->ast);
}

// weak version of freeCodeInstance, does not free the ast (used by functions that reference the ast, but do not own it)
void freeCodeInstanceWeak(CodeInstance* instance) {
    DOSATO_FREE_LIST(uint8_t, instance->code, instance->capacity);
    DOSATO_FREE_LIST(size_t, instance->token_indices, instance->capacity);
    instance->count = 0;
    free_LocationList(&instance->loop_jump_locations);
    instance->capacity = 0;
}

int getOffset(OpCode instruction) {
    switch (instruction) {
        default:
            return 1; // simple instructions

        case OP_TYPE_CAST:
        case OP_CALL:
        case OP_RETURN:
        case OP_POP:
        case OP_END_FUNC:
            return 2; // 1 byte for the pop count

        case OP_DEFINE:
        case OP_DEFINE_POP:
        case OP_LOAD_CONSTANT:
        case OP_LOAD:
        case OP_STORE:
        case OP_LOAD_FAST:
        case OP_STORE_FAST:
        case OP_LOAD_LAMBDA:
        case OP_BUILD_LIST:
        case OP_BUILD_OBJECT:
        case OP_JUMP:
        case OP_JUMP_IF_FALSE:
        case OP_JUMP_IF_TRUE:
        case OP_JUMP_IF_MATCH:
        case OP_FOR_ITER:
        case OP_FOR_DISCARD:
        case OP_JUMP_IF_EXCEPTION:
        case OP_DECREMENT_FAST:
        case OP_INCREMENT_FAST:
        case OP_DECREMENT:
        case OP_INCREMENT:
        case OP_INCLUDE:
        case OP_STORE_FAST_POP:
        case OP_STORE_FAST_CONSTANT:
        case OP_STORE_FAST_POP_CONSTANT:
        case OP_DEFINE_CONSTANT:
        case OP_DEFINE_POP_CONSTANT:
        case OP_STORE_FAST_FORCED:
            return 3;

        case OP_JUMP_PEEK_IF_DEFINED:
            return 4;

        case OP_BREAK:
        case OP_CONTINUE:
            return 5; // 2 bytes for the location and 2 bytes for the pop count
    }
}