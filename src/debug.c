#include "../include/common.h"

#include "../include/debug.h"
#include "../include/code_instance.h"

void printInstruction(uint8_t* code, size_t offset, int line) {
    // print offset (in hex with 4 digits) and line number (in decimal)
    printf("%04llx | %d | ", offset, line);

    OpCode instruction = code[offset++];

    int address = 0;
    int peek_count = 0;
    int pop_count = 0;
    switch (instruction) {
        case OP_RETURN:
            puts("OP_RETURN");
            pop_count = code[offset];
            printf(": (%d)", pop_count);
            break;
        case OP_STOP:
            puts("OP_STOP");
            break;

        case OP_POP:
            puts("OP_POP");
            break;

        case OP_LOAD_CONSTANT:
            puts("OP_LOAD_CONSTANT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);

            printf(": (0x%x)", address);
            break;
        case OP_LOAD_FAST:
            puts("OP_LOAD_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_STORE_FAST:
            puts("OP_STORE_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_LOAD:
            puts("OP_LOAD");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_STORE:
            puts("OP_STORE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_JUMP_PEEK_IF_DEFINED:
            puts("OP_JUMP_PEEK_IF_DEFINED");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            peek_count = code[offset + 2];
            printf(": (0x%x) (%d)", address, peek_count);
            break;
        case OP_STORE_PEEK:
            puts("OP_STORE_PEEK");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            peek_count = code[offset + 2];
            printf(": (0x%x) (%d)", address, peek_count);
            break;

        case OP_LOAD_LAMBDA:
            puts("OP_LOAD_LAMBDA");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_DEFINE:
            puts("OP_DEFINE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_TYPE_CAST:
            puts("OP_TYPE_CAST");
            int type = code[offset];
            printf(": (%d)", type);
            break;

        case OP_BUILD_LIST:
            puts("OP_BUILD_LIST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (%d)", address);
            break;
        case OP_BUILD_OBJECT:
            puts("OP_BUILD_OBJECT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (%d)", address);
            break;

        case OP_BINARY_ADD:
            puts("OP_BINARY_ADD");
            break;
        case OP_BINARY_SUBTRACT:
            puts("OP_BINARY_SUBTRACT");
            break;
        case OP_BINARY_MULTIPLY:
            puts("OP_BINARY_MULTIPLY");
            break;
        case OP_BINARY_DIVIDE:
            puts("OP_BINARY_DIVIDE");
            break;
        case OP_BINARY_GREATER:
            puts("OP_BINARY_GREATER");
            break;
        case OP_BINARY_LESS:
            puts("OP_BINARY_LESS");
            break;
        case OP_BINARY_EQUAL:
            puts("OP_BINARY_EQUAL");
            break;
        case OP_BINARY_NOT_EQUAL:
            puts("OP_BINARY_NOT_EQUAL");
            break;
        case OP_BINARY_GREATER_EQUAL:
            puts("OP_BINARY_GREATER_EQUAL");
            break;
        case OP_BINARY_LESS_EQUAL:
            puts("OP_BINARY_LESS_EQUAL");
            break;
        case OP_BINARY_MODULO:
            puts("OP_BINARY_MODULO");
            break;
        case OP_BINARY_POWER:
            puts("OP_BINARY_POWER");
            break;
        case OP_BINARY_ROOT:
            puts("OP_BINARY_ROOT");
            break;
        case OP_BINARY_ROOT_REVERSE:
            puts("OP_BINARY_ROOT_REVERSE");
            break;
        case OP_BINARY_SHIFT_LEFT:
            puts("OP_BINARY_SHIFT_LEFT");
            break;
        case OP_BINARY_SHIFT_RIGHT:
            puts("OP_BINARY_SHIFT_RIGHT");
            break;
        case OP_BINARY_AND_BITWISE:
            puts("OP_BINARY_AND_BITWISE");
            break;
        case OP_BINARY_OR_BITWISE:
            puts("OP_BINARY_OR_BITWISE");
            break;
        case OP_BINARY_XOR_BITWISE:
            puts("OP_BINARY_XOR_BITWISE");
            break;
        case OP_BINARY_MIN:
            puts("OP_BINARY_MIN");
            break;
        case OP_BINARY_MAX:
            puts("OP_BINARY_MAX");
            break;
        case OP_BINARY_LOGICAL_AND:
            puts("OP_BINARY_LOGICAL_AND");
            break;
        case OP_BINARY_LOGICAL_OR:
            puts("OP_BINARY_LOGICAL_OR");
            break;
        case OP_BINARY_NULL_COALESCE:
            puts("OP_BINARY_NULL_COALESCE");
            break;
        case OP_BINARY_RANGE_UP:
            puts("OP_BINARY_RANGE_UP");
            break;
        case OP_BINARY_RANGE_DOWN:
            puts("OP_BINARY_RANGE_DOWN");
            break;
        case OP_BINARY_RANGE_UP_INCLUSIVE:
            puts("OP_BINARY_RANGE_UP_INCLUSIVE");
            break;
        case OP_BINARY_RANGE_DOWN_INCLUSIVE:
            puts("OP_BINARY_RANGE_DOWN_INCLUSIVE");
            break;
        case OP_GETOBJECT_SAFE:
            puts("OP_GETOBJECT_SAFE");
            break;

        case OP_UNARY_NEGATE:
            puts("OP_UNARY_NEGATE");
            break;
        case OP_UNARY_LOGICAL_NOT:
            puts("OP_UNARY_LOGICAL_NOT");
            break;
        case OP_UNARY_BITWISE_NOT:
            puts("OP_UNARY_BITWISE_NOT");
            break;
        case OP_UNARY_SQRT:
            puts("OP_UNARY_SQRT");
            break;
        case OP_UNARY_ABSOLUTE:
            puts("OP_UNARY_ABSOLUTE");
            break;

        case OP_GETLIST:
            puts("OP_GETLIST");
            break;
        case OP_GETOBJECT:
            puts("OP_GETOBJECT");
            break;

        case OP_STORE_SUBSCR:
            puts("OP_STORE_SUBSCR");
            break;

        case OP_INCREMENT_FAST:
            puts("OP_INCREMENT_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_DECREMENT_FAST:
            puts("OP_DECREMENT_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_INCREMENT:
            puts("OP_INCREMENT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_DECREMENT:
            puts("OP_DECREMENT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_INCREMENT_OBJ:
            puts("OP_INCREMENT_OBJ");
            break;
        case OP_DECREMENT_OBJ:
            puts("OP_DECREMENT_OBJ");
            break;
        case OP_INCREMENT_SUBSCR:
            puts("OP_INCREMENT_SUBSCR");
            break;
        case OP_DECREMENT_SUBSCR:
            puts("OP_DECREMENT_SUBSCR");
            break;

        case OP_STORE_OBJ:
            puts("OP_STORE_OBJ");
            break;

        case OP_MARK_CONSTANT:
            puts("OP_MARK_CONSTANT");
            break;


        case OP_COPY:
            puts("OP_COPY");
            break;

        case OP_JUMP:
            puts("OP_JUMP");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_JUMP_IF_FALSE:
            puts("OP_JUMP_IF_FALSE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_JUMP_IF_TRUE:
            puts("OP_JUMP_IF_TRUE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_JUMP_IF_MATCH:
            puts("OP_JUMP_IF_MATCH");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        
        case OP_FOR_ITER:
            puts("OP_FOR_ITER");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        
        case OP_FOR_DISCARD:
            puts("OP_FOR_DISCARD");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_JUMP_IF_EXCEPTION:
            puts("OP_JUMP_IF_EXCEPTION");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_PUSH_NULL:
            puts("OP_PUSH_NULL");
            break;
        case OP_PUSH_MINUS_ONE:
            puts("OP_PUSH_MINUS_ONE");
            break;
        case OP_PUSH_INFINITY:
            puts("OP_PUSH_INFINITY");
            break;
        case OP_PUSH_NAN:
            puts("OP_PUSH_NAN");
            break;


        case OP_CLEAR_EXCEPTION:
            puts("OP_CLEAR_EXCEPTION");
            break;

        case OP_PUSH_TRUE:
            puts("OP_PUSH_TRUE");
            break;
        case OP_PUSH_FALSE:
            puts("OP_PUSH_FALSE");
            break;


        case OP_CALL:
            puts("OP_CALL");
            int arity = code[offset];
            printf(": (%d)", arity);
            break;

        case OP_END_FUNC:
            puts("OP_END_FUNC");
            break;

        case OP_LOAD_UNDERSCORE:
            puts("OP_LOAD_UNDERSCORE");
            break;

        case OP_BREAK:
            puts("OP_BREAK");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            pop_count = DOSATO_GET_ADDRESS_SHORT(code, offset + 2);
            printf(": (0x%x) (%d)", address, pop_count);
            break;
        case OP_CONTINUE:
            puts("OP_CONTINUE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            pop_count = DOSATO_GET_ADDRESS_SHORT(code, offset + 2);
            printf(": (0x%x) (%d)", address, pop_count);
            break;

        case OP_INCLUDE:
            puts("OP_INCLUDE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_END_INCLUDE:
            puts("OP_END_INCLUDE");
            break;

        case OP_NOP:
            puts("OP_NOP");
            break;

        case OP_TEMP:
            puts("OP_TEMP");
            break;

        default:
            printf("Unknown opcode: %d", instruction);
            break;
    }

    puts("\n");
}

void disassembleCode(CodeInstance* instance, const char* name) {
    printf("==== %s ====\n", name);

    int line = 0;
    for (size_t offset = 0; offset < instance->count; ) {
        printInstruction(instance->code, offset, line++);
        offset += getOffset(instance->code[offset]);
    }
}