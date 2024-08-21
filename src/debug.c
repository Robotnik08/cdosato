#include "../include/common.h"

#include "../include/debug.h"
#include "../include/code_instance.h"

void printInstruction(uint8_t* code, size_t offset, int line) {
    // print offset (in hex with 4 digits) and line number (in decimal)
    printf("%04llx | %d | ", offset, line);

    OpCode instruction = code[offset++];

    int address = 0;
    switch (instruction) {
        case OP_RETURN:
            printf("OP_RETURN");
            break;
        case OP_STOP:
            printf("OP_STOP");
            break;

        case OP_POP:
            printf("OP_POP");
            break;

        case OP_LOAD_CONSTANT:
            printf("OP_LOAD_CONSTANT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);

            printf(": (0x%x)", address);
            break;
        case OP_LOAD_FAST:
            printf("OP_LOAD_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_STORE_FAST:
            printf("OP_STORE_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_LOAD:
            printf("OP_LOAD");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_STORE:
            printf("OP_STORE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_DEFINE:
            printf("OP_DEFINE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_TYPE_CAST:
            printf("OP_TYPE_CAST");
            int type = code[offset];
            printf(": (%d)", type);
            break;

        case OP_BUILD_LIST:
            printf("OP_BUILD_LIST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (%d)", address);
            break;

        case OP_BINARY_ADD:
            printf("OP_BINARY_ADD");
            break;
        case OP_BINARY_SUBTRACT:
            printf("OP_BINARY_SUBTRACT");
            break;
        case OP_BINARY_MULTIPLY:
            printf("OP_BINARY_MULTIPLY");
            break;
        case OP_BINARY_DIVIDE:
            printf("OP_BINARY_DIVIDE");
            break;
        case OP_BINARY_GREATER:
            printf("OP_BINARY_GREATER");
            break;
        case OP_BINARY_LESS:
            printf("OP_BINARY_LESS");
            break;
        case OP_BINARY_EQUAL:
            printf("OP_BINARY_EQUAL");
            break;
        case OP_BINARY_NOT_EQUAL:
            printf("OP_BINARY_NOT_EQUAL");
            break;
        case OP_BINARY_GREATER_EQUAL:
            printf("OP_BINARY_GREATER_EQUAL");
            break;
        case OP_BINARY_LESS_EQUAL:
            printf("OP_BINARY_LESS_EQUAL");
            break;
        case OP_BINARY_AND:
            printf("OP_BINARY_AND");
            break;
        case OP_BINARY_OR:
            printf("OP_BINARY_OR");
            break;
        case OP_BINARY_XOR:
            printf("OP_BINARY_XOR");
            break;
        case OP_BINARY_MODULO:
            printf("OP_BINARY_MODULO");
            break;
        case OP_BINARY_POWER:
            printf("OP_BINARY_POWER");
            break;
        case OP_BINARY_ROOT:
            printf("OP_BINARY_ROOT");
            break;
        case OP_BINARY_SHIFT_LEFT:
            printf("OP_BINARY_SHIFT_LEFT");
            break;
        case OP_BINARY_SHIFT_RIGHT:
            printf("OP_BINARY_SHIFT_RIGHT");
            break;
        case OP_BINARY_AND_BITWISE:
            printf("OP_BINARY_AND_BITWISE");
            break;
        case OP_BINARY_OR_BITWISE:
            printf("OP_BINARY_OR_BITWISE");
            break;
        case OP_BINARY_XOR_BITWISE:
            printf("OP_BINARY_XOR_BITWISE");
            break;
        case OP_BINARY_MIN:
            printf("OP_BINARY_MIN");
            break;
        case OP_BINARY_MAX:
            printf("OP_BINARY_MAX");
            break;
        case OP_BINARY_LOGICAL_AND:
            printf("OP_BINARY_LOGICAL_AND");
            break;
        case OP_BINARY_LOGICAL_OR:
            printf("OP_BINARY_LOGICAL_OR");
            break;

        case OP_GETLIST:
            printf("OP_GETLIST");
            break;
        case OP_GETOBJECT:
            printf("OP_GETOBJECT");
            break;

        case OP_JUMP:
            printf("OP_JUMP");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_JUMP_IF_FALSE:
            printf("OP_JUMP_IF_FALSE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_JUMP_IF_TRUE:
            printf("OP_JUMP_IF_TRUE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_PRINT:
            printf("OP_PRINT");
            break;

        case OP_PUSH_NULL:
            printf("OP_PUSH_NULL");
            break;


        default:
            printf("Unknown opcode: %d", instruction);
            break;
    }

    printf("\n");
}

void disassembleCode(CodeInstance* instance, const char* name) {
    printf("==== %s ====\n", name);

    int line = 0;
    for (size_t offset = 0; offset < instance->count; ) {
        printInstruction(instance->code, offset, line++);
        offset += getOffset(instance->code[offset]);
    }
}