#include "../include/common.h"

#include "../include/memory.h"
#include "../include/code_instance.h"

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




void initCodeInstance(CodeInstance* instance) {
    instance->code = NULL;
    instance->token_indices = NULL;
    init_LocationList(&instance->loop_jump_locations);
    instance->count = 0;
    instance->capacity = 0;
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

void writeByteCodeAt(CodeInstance* instance, uint8_t byte, size_t token_index, size_t index) {
    if (instance->capacity < instance->count + 1) {
        size_t oldCapacity = instance->capacity;
        instance->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        instance->code = DOSATO_RESIZE_LIST(uint8_t, instance->code, oldCapacity, instance->capacity);
        instance->token_indices = DOSATO_RESIZE_LIST(size_t, instance->token_indices, oldCapacity, instance->capacity);
    }

    instance->code[index] = byte;
    instance->token_indices[index] = token_index;
}

void insertByteCode(CodeInstance* instance, uint8_t byte, size_t token_index, size_t index) {
    if (instance->capacity < instance->count + 1) {
        size_t oldCapacity = instance->capacity;
        instance->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        instance->code = DOSATO_RESIZE_LIST(uint8_t, instance->code, oldCapacity, instance->capacity);
        instance->token_indices = DOSATO_RESIZE_LIST(size_t, instance->token_indices, oldCapacity, instance->capacity);
    }

    for (size_t i = instance->count; i > index; i--) {
        instance->code[i] = instance->code[i - 1];
    }

    instance->code[index] = byte;
    instance->token_indices[index] = token_index;
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
}

int getOffset(OpCode instruction) {
    switch (instruction) {
        case OP_LOAD_CONSTANT:
            return 3; // 2 bytes for address
        case OP_LOAD:
            return 3; // 2 bytes for address
        case OP_STORE:
            return 3; // 2 bytes for address
        case OP_LOAD_FAST:
            return 3; // 2 bytes for address
        case OP_STORE_FAST:
            return 3; // 2 bytes for address

        case OP_DEFINE:
            return 3; // 2 bytes for address

        case OP_BUILD_LIST:
            return 3; // 2 bytes for the count
        case OP_BUILD_OBJECT:
            return 3; // 2 bytes for the count

        case OP_REFERENCE:
            return 3; // 2 bytes for the address
        case OP_REFERENCE_FAST:
            return 3; // 2 bytes for the address

        case OP_DECREMENT_FAST:
            return 3; // 2 bytes for the address
        case OP_INCREMENT_FAST:
            return 3; // 2 bytes for the address
        case OP_DECREMENT:
            return 3; // 2 bytes for the address
        case OP_INCREMENT:
            return 3; // 2 bytes for the address
            

        case OP_JUMP:
            return 3; // 2 bytes for the location
        case OP_JUMP_IF_FALSE:
            return 3; // 2 bytes for the location
        case OP_JUMP_IF_TRUE:
            return 3; // 2 bytes for the location
        case OP_FOR_ITER:
            return 3; // 2 bytes for the location

        case OP_JUMP_IF_EXCEPTION:
            return 3; // 2 bytes for the location

        case OP_BREAK:
            return 5; // 2 bytes for the location and 2 bytes for the pop count
        case OP_CONTINUE:
            return 5; // 2 bytes for the location and 2 bytes for the pop count


        case OP_TYPE_CAST:
            return 2; // 1 byte for the type

        case OP_CALL:
            return 2; // 1 byte for the arity
        case OP_RETURN:
            return 2; // 1 byte for the arity


        default:
            return 1; // simple instructions
    }
}