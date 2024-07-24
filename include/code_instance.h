#ifndef dosato_byte_code_instance_h
#define dosato_byte_code_instance_h

#include "common.h"

typedef enum {
    OP_NOP,
    
    OP_STOP,
    OP_RETURN,
    
    OP_LOAD_CONSTANT,
    OP_LOAD_FAST, // load from local variable
    OP_STORE_FAST, // store to local variable
    OP_LOAD_NAME, 
    OP_STORE_NAME,

    OP_BINARY_ADD,
    OP_BINARY_SUBTRACT,
    OP_BINARY_MULTIPLY,
    OP_BINARY_DIVIDE,

    OP_JUMP, 
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,
    OP_JUMP_ABSOLUTE,

    OP_PRINT // print the top of the stack, this is a debug instruction to be removed

} OpCode;

typedef struct {
    uint8_t* code;
    size_t count;
    size_t capacity;
} CodeInstance;

void initCodeInstance(CodeInstance* instance);
void writeByteCode(CodeInstance* instance, uint8_t byte);
void writeByteCodeAt(CodeInstance* instance, uint8_t byte, size_t index);
void insertByteCode(CodeInstance* instance, uint8_t byte, size_t index);
void writeInstruction(CodeInstance* instance, OpCode instruction, ...);
void freeCodeInstance(CodeInstance* instance);

int getOffset(OpCode instruction);

#define DOSATO_GET_ADDRESS_SHORT(code, offset) (*(uint16_t *)(code + offset))

#define DOSATO_SPLIT_SHORT(value) (value & 0xFF), (value >> 8)

#endif // dosato_byte_code_instance_h