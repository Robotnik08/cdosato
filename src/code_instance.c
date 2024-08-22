#include "../include/common.h"

#include "../include/memory.h"
#include "../include/code_instance.h"

void initCodeInstance(CodeInstance* instance) {
    instance->code = NULL;
    instance->token_indices = NULL;
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
            

        case OP_JUMP:
            return 3; // 2 bytes for the location
        case OP_JUMP_IF_FALSE:
            return 3; // 2 bytes for the location
        case OP_JUMP_IF_TRUE:
            return 3; // 2 bytes for the location
        case OP_JUMP_ABSOLUTE:
            return 3; // 2 bytes for the location


        default:
            return 1; // simple instructions
    }
}