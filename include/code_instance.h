#ifndef dosato_byte_code_instance_h
#define dosato_byte_code_instance_h

#include "common.h"

typedef enum {
    OP_NOP,

    OP_STOP,
    OP_RETURN,

    OP_POP,

    OP_LOAD_CONSTANT,
    OP_LOAD_FAST, // load from local variable
    OP_STORE_FAST, // store to local variable
    OP_LOAD,
    OP_STORE,

    OP_DEFINE, // define a global variable

    OP_TYPE_CAST,

    OP_BUILD_LIST,

    OP_BINARY_ADD,
    OP_BINARY_SUBTRACT,
    OP_BINARY_MULTIPLY,
    OP_BINARY_DIVIDE,
    OP_BINARY_GREATER,
    OP_BINARY_LESS,
    OP_BINARY_EQUAL,
    OP_BINARY_NOT_EQUAL,
    OP_BINARY_GREATER_EQUAL,
    OP_BINARY_LESS_EQUAL,
    OP_BINARY_AND,
    OP_BINARY_OR,
    OP_BINARY_XOR,
    OP_BINARY_MODULO,
    OP_BINARY_POWER,
    OP_BINARY_ROOT,
    OP_BINARY_SHIFT_LEFT,
    OP_BINARY_SHIFT_RIGHT,
    OP_BINARY_AND_BITWISE,
    OP_BINARY_OR_BITWISE,
    OP_BINARY_XOR_BITWISE,
    OP_BINARY_MIN,
    OP_BINARY_MAX,
    OP_BINARY_LOGICAL_AND,
    OP_BINARY_LOGICAL_OR,

    OP_GETLIST,
    OP_GETOBJECT,

    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,
    OP_JUMP_ABSOLUTE,

    OP_PUSH_NULL,

    OP_PRINT // print the top of the stack, this is a debug instruction to be removed

} OpCode;

typedef struct {
    uint8_t* code;
    size_t* token_indices;
    size_t count;
    size_t capacity;
} CodeInstance;

void initCodeInstance(CodeInstance* instance);
void writeByteCode(CodeInstance* instance, uint8_t byte, size_t token_index);
void writeByteCodeAt(CodeInstance* instance, uint8_t byte, size_t token_index, size_t index);
void insertByteCode(CodeInstance* instance, uint8_t byte, size_t token_index, size_t index);
void writeInstruction(CodeInstance* instance, size_t token_index, OpCode instruction, ...);
void freeCodeInstance(CodeInstance* instance);

int getOffset(OpCode instruction);

#define DOSATO_GET_ADDRESS_SHORT(code, offset) (*(uint16_t *)(code + offset))

#define DOSATO_SPLIT_SHORT(value) (value & 0xFF), (value >> 8)

#endif // dosato_byte_code_instance_h