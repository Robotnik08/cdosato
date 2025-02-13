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
            printf("%s", "OP_RETURN");
            pop_count = code[offset];
            printf(": (%d)", pop_count);
            break;
        case OP_STOP:
            printf("%s", "OP_STOP");
            break;

        case OP_POP:
            printf("%s", "OP_POP");
            pop_count = code[offset];
            printf(": (%d)", pop_count);
            break;

        case OP_LOAD_CONSTANT:
            printf("%s", "OP_LOAD_CONSTANT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);

            printf(": (0x%x)", address);
            break;
        case OP_LOAD_FAST:
            printf("%s", "OP_LOAD_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_STORE_FAST:
            printf("%s", "OP_STORE_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_LOAD:
            printf("%s", "OP_LOAD");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_STORE:
            printf("%s", "OP_STORE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_JUMP_PEEK_IF_DEFINED:
            printf("%s", "OP_JUMP_PEEK_IF_DEFINED");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            peek_count = code[offset + 2];
            printf(": (0x%x) (%d)", address, peek_count);
            break;

        case OP_LOAD_LAMBDA:
            printf("%s", "OP_LOAD_LAMBDA");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_DEFINE:
            printf("%s", "OP_DEFINE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_DEFINE_POP:
            printf("%s", "OP_DEFINE_POP");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_TYPE_CAST:
            printf("%s", "OP_TYPE_CAST");
            int type = code[offset];
            printf(": (%d)", type);
            break;

        case OP_BUILD_LIST:
            printf("%s", "OP_BUILD_LIST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (%d)", address);
            break;
        case OP_BUILD_OBJECT:
            printf("%s", "OP_BUILD_OBJECT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (%d)", address);
            break;

        case OP_BINARY_ADD:
            printf("%s", "OP_BINARY_ADD");
            break;
        case OP_BINARY_SUBTRACT:
            printf("%s", "OP_BINARY_SUBTRACT");
            break;
        case OP_BINARY_MULTIPLY:
            printf("%s", "OP_BINARY_MULTIPLY");
            break;
        case OP_BINARY_DIVIDE:
            printf("%s", "OP_BINARY_DIVIDE");
            break;
        case OP_BINARY_GREATER:
            printf("%s", "OP_BINARY_GREATER");
            break;
        case OP_BINARY_LESS:
            printf("%s", "OP_BINARY_LESS");
            break;
        case OP_BINARY_EQUAL:
            printf("%s", "OP_BINARY_EQUAL");
            break;
        case OP_BINARY_NOT_EQUAL:
            printf("%s", "OP_BINARY_NOT_EQUAL");
            break;
        case OP_BINARY_GREATER_EQUAL:
            printf("%s", "OP_BINARY_GREATER_EQUAL");
            break;
        case OP_BINARY_LESS_EQUAL:
            printf("%s", "OP_BINARY_LESS_EQUAL");
            break;
        case OP_BINARY_MODULO:
            printf("%s", "OP_BINARY_MODULO");
            break;
        case OP_BINARY_POWER:
            printf("%s", "OP_BINARY_POWER");
            break;
        case OP_BINARY_ROOT:
            printf("%s", "OP_BINARY_ROOT");
            break;
        case OP_BINARY_ROOT_REVERSE:
            printf("%s", "OP_BINARY_ROOT_REVERSE");
            break;
        case OP_BINARY_SHIFT_LEFT:
            printf("%s", "OP_BINARY_SHIFT_LEFT");
            break;
        case OP_BINARY_SHIFT_RIGHT:
            printf("%s", "OP_BINARY_SHIFT_RIGHT");
            break;
        case OP_BINARY_AND_BITWISE:
            printf("%s", "OP_BINARY_AND_BITWISE");
            break;
        case OP_BINARY_OR_BITWISE:
            printf("%s", "OP_BINARY_OR_BITWISE");
            break;
        case OP_BINARY_XOR_BITWISE:
            printf("%s", "OP_BINARY_XOR_BITWISE");
            break;
        case OP_BINARY_MIN:
            printf("%s", "OP_BINARY_MIN");
            break;
        case OP_BINARY_MAX:
            printf("%s", "OP_BINARY_MAX");
            break;
        case OP_BINARY_LOGICAL_AND:
            printf("%s", "OP_BINARY_LOGICAL_AND");
            break;
        case OP_BINARY_LOGICAL_OR:
            printf("%s", "OP_BINARY_LOGICAL_OR");
            break;
        case OP_BINARY_NULL_COALESCE:
            printf("%s", "OP_BINARY_NULL_COALESCE");
            break;
        case OP_BINARY_RANGE_UP:
            printf("%s", "OP_BINARY_RANGE_UP");
            break;
        case OP_BINARY_RANGE_DOWN:
            printf("%s", "OP_BINARY_RANGE_DOWN");
            break;
        case OP_BINARY_RANGE_UP_INCLUSIVE:
            printf("%s", "OP_BINARY_RANGE_UP_INCLUSIVE");
            break;
        case OP_BINARY_RANGE_DOWN_INCLUSIVE:
            printf("%s", "OP_BINARY_RANGE_DOWN_INCLUSIVE");
            break;
        case OP_GETOBJECT_SAFE:
            printf("%s", "OP_GETOBJECT_SAFE");
            break;
        case OP_BINARY_FALSEY_COALESCE:
            printf("%s", "OP_BINARY_FALSEY_COALESCE");
            break;
        case OP_BINARY_SPACE_SHIP:
            printf("%s", "OP_BINARY_SPACE_SHIP");
            break;

        case OP_UNARY_NEGATE:
            printf("%s", "OP_UNARY_NEGATE");
            break;
        case OP_UNARY_LOGICAL_NOT:
            printf("%s", "OP_UNARY_LOGICAL_NOT");
            break;
        case OP_UNARY_BITWISE_NOT:
            printf("%s", "OP_UNARY_BITWISE_NOT");
            break;
        case OP_UNARY_SQRT:
            printf("%s", "OP_UNARY_SQRT");
            break;
        case OP_UNARY_ABSOLUTE:
            printf("%s", "OP_UNARY_ABSOLUTE");
            break;

        case OP_GETLIST:
            printf("%s", "OP_GETLIST");
            break;
        case OP_GETOBJECT:
            printf("%s", "OP_GETOBJECT");
            break;

        case OP_STORE_SUBSCR:
            printf("%s", "OP_STORE_SUBSCR");
            break;

        case OP_INCREMENT_FAST:
            printf("%s", "OP_INCREMENT_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_DECREMENT_FAST:
            printf("%s", "OP_DECREMENT_FAST");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_INCREMENT:
            printf("%s", "OP_INCREMENT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_DECREMENT:
            printf("%s", "OP_DECREMENT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_INCREMENT_OBJ:
            printf("%s", "OP_INCREMENT_OBJ");
            break;
        case OP_DECREMENT_OBJ:
            printf("%s", "OP_DECREMENT_OBJ");
            break;
        case OP_INCREMENT_SUBSCR:
            printf("%s", "OP_INCREMENT_SUBSCR");
            break;
        case OP_DECREMENT_SUBSCR:
            printf("%s", "OP_DECREMENT_SUBSCR");
            break;

        case OP_STORE_OBJ:
            printf("%s", "OP_STORE_OBJ");
            break;

        case OP_MARK_CONSTANT:
            printf("%s", "OP_MARK_CONSTANT");
            break;


        case OP_COPY:
            printf("%s", "OP_COPY");
            break;

        case OP_JUMP:
            printf("%s", "OP_JUMP");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_JUMP_IF_FALSE:
            printf("%s", "OP_JUMP_IF_FALSE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_JUMP_IF_TRUE:
            printf("%s", "OP_JUMP_IF_TRUE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        case OP_JUMP_IF_MATCH:
            printf("%s", "OP_JUMP_IF_MATCH");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        
        case OP_FOR_ITER:
            printf("%s", "OP_FOR_ITER");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;
        
        case OP_FOR_DISCARD:
            printf("%s", "OP_FOR_DISCARD");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_JUMP_IF_EXCEPTION:
            printf("%s", "OP_JUMP_IF_EXCEPTION");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_PUSH_NULL:
            printf("%s", "OP_PUSH_NULL");
            break;
        case OP_PUSH_MINUS_ONE:
            printf("%s", "OP_PUSH_MINUS_ONE");
            break;
        case OP_PUSH_INFINITY:
            printf("%s", "OP_PUSH_INFINITY");
            break;
        case OP_PUSH_NAN:
            printf("%s", "OP_PUSH_NAN");
            break;


        case OP_CLEAR_EXCEPTION:
            printf("%s", "OP_CLEAR_EXCEPTION");
            break;

        case OP_PUSH_TRUE:
            printf("%s", "OP_PUSH_TRUE");
            break;
        case OP_PUSH_FALSE:
            printf("%s", "OP_PUSH_FALSE");
            break;


        case OP_CALL:
            printf("%s", "OP_CALL");
            int arity = code[offset];
            printf(": (%d)", arity);
            break;

        case OP_END_FUNC:
            printf("%s", "OP_END_FUNC");
            pop_count = code[offset];
            printf(": (%d)", pop_count);
            break;

        case OP_LOAD_UNDERSCORE:
            printf("%s", "OP_LOAD_UNDERSCORE");
            break;

        case OP_BREAK:
            printf("%s", "OP_BREAK");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            pop_count = DOSATO_GET_ADDRESS_SHORT(code, offset + 2);
            printf(": (0x%x) (%d)", address, pop_count);
            break;
        case OP_CONTINUE:
            printf("%s", "OP_CONTINUE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            pop_count = DOSATO_GET_ADDRESS_SHORT(code, offset + 2);
            printf(": (0x%x) (%d)", address, pop_count);
            break;

        case OP_INCLUDE:
            printf("%s", "OP_INCLUDE");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_END_INCLUDE:
            printf("%s", "OP_END_INCLUDE");
            break;

        case OP_NOP:
            printf("%s", "OP_NOP");
            break;

        case OP_TEMP:
            printf("%s", "OP_TEMP");
            break;

        case OP_STORE_FAST_POP:
            printf("%s", "OP_STORE_FAST_POP");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_STORE_FAST_CONSTANT:
            printf("%s", "OP_STORE_FAST_CONSTANT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_STORE_FAST_POP_CONSTANT:
            printf("%s", "OP_STORE_FAST_POP_CONSTANT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_DEFINE_CONSTANT:
            printf("%s", "OP_DEFINE_CONSTANT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_DEFINE_POP_CONSTANT:
            printf("%s", "OP_DEFINE_POP_CONSTANT");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        case OP_STORE_FAST_FORCED:
            printf("%s", "OP_STORE_FAST_FORCED");
            address = DOSATO_GET_ADDRESS_SHORT(code, offset);
            printf(": (0x%x)", address);
            break;

        default:
            printf("Unknown opcode: %d", instruction);
            break;
    }

    printf("%s", "\n");
}

void disassembleCode(CodeInstance* instance, const char* name) {
    printf("==== %s ====\n", name);

    int line = 0;
    for (size_t offset = 0; offset < instance->count; ) {
        printInstruction(instance->code, offset, line++);
        offset += getOffset(instance->code[offset]);
    }
}