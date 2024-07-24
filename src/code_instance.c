#include "../include/common.h"

#include "../include/memory.h"
#include "../include/code_instance.h"

void initCodeInstance(CodeInstance* instance) {
    instance->code = NULL;
    instance->count = 0;
    instance->capacity = 0;
}

void writeByteCode(CodeInstance* instance, uint8_t byte) {
    if (instance->capacity < instance->count + 1) {
        size_t oldCapacity = instance->capacity;
        instance->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        instance->code = DOSATO_RESIZE_LIST(uint8_t, instance->code, oldCapacity, instance->capacity);
    }

    instance->code[instance->count] = byte;
    instance->count++;
}

void writeByteCodeAt(CodeInstance* instance, uint8_t byte, size_t index) {
    if (instance->capacity < instance->count + 1) {
        size_t oldCapacity = instance->capacity;
        instance->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        instance->code = DOSATO_RESIZE_LIST(uint8_t, instance->code, oldCapacity, instance->capacity);
    }

    instance->code[index] = byte;
}

void insertByteCode(CodeInstance* instance, uint8_t byte, size_t index) {
    if (instance->capacity < instance->count + 1) {
        size_t oldCapacity = instance->capacity;
        instance->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        instance->code = DOSATO_RESIZE_LIST(uint8_t, instance->code, oldCapacity, instance->capacity);
    }

    for (size_t i = instance->count; i > index; i--) {
        instance->code[i] = instance->code[i - 1];
    }

    instance->code[index] = byte;
    instance->count++;
}

void writeInstruction(CodeInstance* instance, OpCode instruction, ...) {
    writeByteCode(instance, instruction);
    size_t offset = getOffset(instruction);
    va_list args;
    va_start(args, instruction);
    for (size_t i = 0; i < offset - 1; i++) {
        writeByteCode(instance, va_arg(args, int));
    }
    va_end(args);
}

void freeCodeInstance(CodeInstance* instance) {
    DOSATO_FREE_LIST(uint8_t, instance->code, instance->capacity);
    instance->count = 0;
    instance->capacity = 0;
}

int getOffset(OpCode instruction) {
    switch (instruction) {
        case OP_RETURN:
            return 1;

        case OP_LOAD_CONSTANT:
            return 3; // 2 bytes for address
        case OP_LOAD_NAME:
            return 3; // 2 bytes for address
        case OP_STORE_NAME:
            return 3; // 2 bytes for address

        case OP_BINARY_ADD:
            return 1;
        case OP_BINARY_SUBTRACT:
            return 1;
        case OP_BINARY_MULTIPLY:
            return 1;
        case OP_BINARY_DIVIDE:
            return 1;

        case OP_JUMP:
            return 3; // 2 bytes for the location
        case OP_JUMP_IF_FALSE:
            return 3; // 2 bytes for the location
        case OP_JUMP_IF_TRUE:
            return 3; // 2 bytes for the location
        case OP_JUMP_ABSOLUTE:
            return 3; // 2 bytes for the location
        default:
            return 1;
    }
}