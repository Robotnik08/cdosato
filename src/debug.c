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
            printf("OP_LOAD_NAME");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_STORE:
            printf("OP_STORE_NAME");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_LOAD_SMART:
            printf("OP_LOAD_SMART");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_STORE_SMART:
            printf("OP_STORE_SMART");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_DEFINE:
            printf("OP_DEFINE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_DEFINE_FAST:
            printf("OP_DEFINE_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_TYPE_CAST:
            printf("OP_TYPE_CAST");
            int type = code[offset];
            printf(": (%d)", type);
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