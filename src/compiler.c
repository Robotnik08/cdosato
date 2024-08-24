#include "../include/common.h"

#include "../include/compiler.h"
#include "../include/memory.h"
#include "../include/virtual-machine.h"
#include "../include/error.h"

void compile(VirtualMachine* vm, AST ast) { 

    // Compile the AST into byte code
    compileNode(vm, ast.root, ast, NULL);

    writeByteCode(vm->instance, OP_STOP, 0);
}

#define ERROR(e_code, index) printError(ast.source, ast.tokens.tokens[index].start - ast.source, e_code)

void compileNode (VirtualMachine* vm, Node node, AST ast, ScopeData* scope) {
    NodeType type = node.type;
    if (node.type == NODE_TEMP_SUBSCRIPT_EXPRESSION) {
        type = NODE_EXPRESSION;
    }

    switch (type) {
        case NODE_BLOCK: {
            if (scope == NULL) { // first scope after global
                ScopeData new_scope;
                initScopeData(&new_scope);
                scope = &new_scope;
            }
            // fall through
        }
        case NODE_PROGRAM: {
            int scope_start = -1;
            if (scope != NULL) {
                scope_start = scope->locals_count;
            }
            for (int i = 0; i < node.body.count; i++) {
                compileNode(vm, node.body.nodes[i], ast, scope);
            }
            if (scope != NULL) {
                int count = scope->locals_count - scope_start;
                for (int i = 0; i < count; i++) {
                    writeByteCode(vm->instance, OP_POP, 0);
                    popScopeData(scope);
                }
            }

            break;
        }

        case NODE_MASTER_MAKE:
        case NODE_MASTER_SET:
        case NODE_MASTER_DO: {
            compileNode(vm, node.body.nodes[0], ast, scope);
            break;
        }

        case NODE_MASTER_DO_BODY: {
            compileNode(vm, node.body.nodes[0], ast, scope);
            break;
        }

        case NODE_MASTER_MAKE_BODY: {
            // push the value to the stack
            if (scope != NULL) { // local scope (push null to the stack, used for the local variable)
                writeByteCode(vm->instance, OP_PUSH_NULL, node.body.nodes[1].start);
            }

            compileNode(vm, node.body.nodes[2], ast, scope);
            writeInstruction(vm->instance, node.start, OP_TYPE_CAST, ast.tokens.tokens[node.body.nodes[0].start].carry); // cast to the correct type
            
            if (scope == NULL) { // global scope
                writeInstruction(vm->instance, node.body.nodes[1].start, OP_DEFINE, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.body.nodes[1].start].carry));
            } else {
                if (inScope(scope, ast.tokens.tokens[node.body.nodes[1].start].carry)) {
                    ERROR(E_ALREADY_DEFINED_VARIABLE, node.body.nodes[1].start);
                }
                writeInstruction(vm->instance, node.body.nodes[1].start, OP_STORE_FAST, DOSATO_SPLIT_SHORT(scope->locals_count));
                pushScopeData(scope, ast.tokens.tokens[node.body.nodes[1].start].carry);
            }
            
            break;
        }

        case NODE_MASTER_SET_BODY: {

            // if operator is not an assignment operator, then it is a binary operator
            OperatorType operator = ast.tokens.tokens[node.body.nodes[1].start].carry;
            if (operator == OPERATOR_INCREMENT || operator == OPERATOR_DECREMENT) {
                // do nothing, handled later
            } else if (operator != OPERATOR_ASSIGN) {
                // compile the left side of the expression
                Node left = node.body.nodes[0];
                left.type = left.type == NODE_SUBSCRIPT_EXPRESSION ? NODE_TEMP_SUBSCRIPT_EXPRESSION : left.type;
                compileNode(vm, left, ast, scope);
                // push the value to the stack
                compileNode(vm, node.body.nodes[2], ast, scope);

                writeOperatorInstruction(vm, operator, node.body.nodes[1].start);
            } else {
                // push the value to the stack
                compileNode(vm, node.body.nodes[2], ast, scope);
            }

            

            if (node.body.nodes[0].type == NODE_INDENTIFIER) {
                if (inScope(scope, ast.tokens.tokens[node.body.nodes[0].start].carry)) {
                    size_t index = getScopeIndex(scope, ast.tokens.tokens[node.body.nodes[0].start].carry);
                    OpCode op = operator == OPERATOR_INCREMENT ? OP_INCREMENT_FAST : operator == OPERATOR_DECREMENT ? OP_DECREMENT_FAST : OP_STORE_FAST;
                    writeInstruction(vm->instance, node.body.nodes[0].start, op, DOSATO_SPLIT_SHORT(index));
                } else {
                    OpCode op = operator == OPERATOR_INCREMENT ? OP_INCREMENT : operator == OPERATOR_DECREMENT ? OP_DECREMENT : OP_STORE;
                    writeInstruction(vm->instance, node.body.nodes[0].start, op, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.body.nodes[0].start].carry));
                }
            } else if (node.body.nodes[0].type == NODE_SUBSCRIPT_EXPRESSION) {
                if (node.body.nodes[0].body.nodes[0].type != NODE_INDENTIFIER) {
                    compileNode(vm, node.body.nodes[0].body.nodes[0], ast, scope); // the left side of the subscript (the object)
                } else {
                    if (inScope(scope, ast.tokens.tokens[node.body.nodes[0].body.nodes[0].start].carry)) {
                        writeInstruction(vm->instance, node.body.nodes[0].body.nodes[0].start, OP_REFERENCE_FAST, DOSATO_SPLIT_SHORT(getScopeIndex(scope, ast.tokens.tokens[node.body.nodes[0].body.nodes[0].start].carry)));
                    } else {
                        writeInstruction(vm->instance, node.body.nodes[0].body.nodes[0].start, OP_REFERENCE, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.body.nodes[0].body.nodes[0].start].carry));
                    }
                }

                compileNode(vm, node.body.nodes[0].body.nodes[2], ast, scope); // the right side of the subscript (the object)

                OperatorType operator_inner = ast.tokens.tokens[node.body.nodes[0].body.nodes[1].start].carry; // operator for the subscript

                OpCode op = operator_inner == OPERATOR_HASH ? 
                    (operator == OPERATOR_INCREMENT ? OP_INCREMENT_SUBSCR : operator == OPERATOR_DECREMENT ? OP_DECREMENT_SUBSCR : OP_STORE_SUBSCR) 
                    : (operator == OPERATOR_INCREMENT ? OP_INCREMENT_OBJ : operator == OPERATOR_DECREMENT ? OP_DECREMENT_OBJ : OP_STORE_OBJ);
                    
                writeByteCode(vm->instance, op, node.body.nodes[0].start);
            }
            
            break;
        }

        case NODE_SUBSCRIPT_EXPRESSION: {
            if (node.body.nodes[0].type != NODE_INDENTIFIER) {
                compileNode(vm, node.body.nodes[0], ast, scope); // the left side of the subscript (the object)
            } else {
                if (inScope(scope, ast.tokens.tokens[node.body.nodes[0].start].carry)) {
                    writeInstruction(vm->instance, node.body.nodes[0].start, OP_REFERENCE_FAST, DOSATO_SPLIT_SHORT(getScopeIndex(scope, ast.tokens.tokens[node.body.nodes[0].start].carry)));
                } else {
                    writeInstruction(vm->instance, node.body.nodes[0].start, OP_REFERENCE, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.body.nodes[0].start].carry));
                }
            }
            
            if (node.body.count != 3) {
                ERROR(E_INVALID_EXPRESSION, node.start);
            }

            OperatorType operator = ast.tokens.tokens[node.body.nodes[1].start].carry;

            compileNode(vm, node.body.nodes[2], ast, scope);

            if (ast.tokens.tokens[node.body.nodes[1].start].carry != OPERATOR_HASH && ast.tokens.tokens[node.body.nodes[1].start].carry != OPERATOR_ARROW) {
                ERROR(E_EXPECTED_HASH_OPERATOR, node.body.nodes[1].start);
            }
            writeByteCode(vm->instance, operator == OPERATOR_HASH ? OP_REFERENCE_SUBSCR : OP_REFERENCE_GETOBJ, node.body.nodes[1].start);
            break;
        }

        case NODE_FUNCTION_CALL: {
            // push arguments
            for (int i = 0; i < node.body.nodes[1].body.count; i++) {
                compileNode(vm, node.body.nodes[1].body.nodes[i], ast, scope);
            }
            writeByteCode(vm->instance, OP_PRINT, node.start); // for now, a function call is just a print
            break;
        }


        case NODE_EXPRESSION: {
            if (node.body.count == 1) {
                compileNode(vm, node.body.nodes[0], ast, scope);
                break;
            }
            if (node.body.nodes[0].type == NODE_SUBSCRIPT_EXPRESSION && node.type == NODE_TEMP_SUBSCRIPT_EXPRESSION) {
                Node sub = node.body.nodes[0];
                sub.type = NODE_EXPRESSION;
                compileNode(vm, sub, ast, scope);
            } else {
                compileNode(vm, node.body.nodes[0], ast, scope);
            }

            compileNode(vm, node.body.nodes[2], ast, scope);

            int res = writeOperatorInstruction(vm, ast.tokens.tokens[node.body.nodes[1].start].carry, node.body.nodes[1].start);
            if (res == -1) {
                ERROR(E_NON_BINARY_OPERATOR, node.body.nodes[1].start);
            }

            break;
        }

        case NODE_TYPE_CAST: {
            compileNode(vm, node.body.nodes[1], ast, scope);
            writeInstruction(vm->instance, node.start, OP_TYPE_CAST, ast.tokens.tokens[node.body.nodes[0].start].carry);
            break;
        }
        
        case NODE_STRING_LITERAL:
        case NODE_NUMBER_LITERAL: {
            writeInstruction(vm->instance, node.start, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.start].carry));
            break;
        }

        case NODE_INDENTIFIER: {
            if (scope == NULL) { // global scope
                writeInstruction(vm->instance, node.start, OP_LOAD, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.start].carry)); // load the global variable
            } else {
                if (!inScope(scope, ast.tokens.tokens[node.start].carry)) {
                    writeInstruction(vm->instance, node.start, OP_LOAD, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.start].carry)); // load the global variable
                    break;
                }
                size_t index = getScopeIndex(scope, ast.tokens.tokens[node.start].carry);
                if (index == -1) {
                    ERROR(E_UNDEFINED_VARIABLE, node.start);
                }
                writeInstruction(vm->instance, node.start, OP_LOAD_FAST, DOSATO_SPLIT_SHORT(index)); // load the local variable
            }
            break;
        }

        case NODE_ARRAY_EXPRESSION: {
            for (int i = node.body.count - 1; i >= 0; i--) {
                compileNode(vm, node.body.nodes[i], ast, scope);
            }
            writeInstruction(vm->instance, node.start, OP_BUILD_LIST, DOSATO_SPLIT_SHORT(node.body.count)); // cast to the correct type
            break;
        }

        case NODE_OBJECT_EXPRESSION: {
            for (int i = node.body.count - 1; i >= 0; i--) {
                compileNode(vm, node.body.nodes[i], ast, scope);
            }
            writeInstruction(vm->instance, node.start, OP_BUILD_OBJECT, DOSATO_SPLIT_SHORT(node.body.count)); // cast to the correct type
            break;
        }

        case NODE_OBJECT_ENTRY: {
            compileNode(vm, node.body.nodes[0], ast, scope);
            compileNode(vm, node.body.nodes[1], ast, scope);
            break;
        }
    }
}

#undef ERROR

int writeOperatorInstruction (VirtualMachine* vm, OperatorType operator, size_t token_index) {
    switch (operator) {
        case OPERATOR_ADD_ASSIGN:
        case OPERATOR_ADD: {
            writeByteCode(vm->instance, OP_BINARY_ADD, token_index);
            break;
        }
        case OPERATOR_SUBTRACT_ASSIGN:
        case OPERATOR_SUBTRACT: {
            writeByteCode(vm->instance, OP_BINARY_SUBTRACT, token_index);
            break;
        }
        case OPERATOR_MULTIPLY_ASSIGN:
        case OPERATOR_MULTIPLY: {
            writeByteCode(vm->instance, OP_BINARY_MULTIPLY, token_index);
            break;
        }
        case OPERATOR_DIVIDE_ASSIGN:
        case OPERATOR_DIVIDE: {
            writeByteCode(vm->instance, OP_BINARY_DIVIDE, token_index);
            break;
        }
        case OPERATOR_GREATER: {
            writeByteCode(vm->instance, OP_BINARY_GREATER, token_index);
            break;
        }
        case OPERATOR_LESS: {
            writeByteCode(vm->instance, OP_BINARY_LESS, token_index);
            break;
        }
        case OPERATOR_EQUAL: {
            writeByteCode(vm->instance, OP_BINARY_EQUAL, token_index);
            break;
        }
        case OPERATOR_NOT_EQUAL: {
            writeByteCode(vm->instance, OP_BINARY_NOT_EQUAL, token_index);
            break;
        }
        case OPERATOR_GREATER_EQUAL: {
            writeByteCode(vm->instance, OP_BINARY_GREATER_EQUAL, token_index);
            break;
        }
        case OPERATOR_LESS_EQUAL: {
            writeByteCode(vm->instance, OP_BINARY_LESS_EQUAL, token_index);
            break;
        }
        case OPERATOR_AND_ASSIGN:
        case OPERATOR_AND: {
            writeByteCode(vm->instance, OP_BINARY_AND, token_index);
            break;
        }
        case OPERATOR_OR_ASSIGN:
        case OPERATOR_OR: {
            writeByteCode(vm->instance, OP_BINARY_OR, token_index);
            break;
        }
        case OPERATOR_AND_AND: {
            writeByteCode(vm->instance, OP_BINARY_LOGICAL_AND, token_index);
            break;
        }
        case OPERATOR_OR_OR: {
            writeByteCode(vm->instance, OP_BINARY_LOGICAL_OR, token_index);
            break;
        }
        case OPERATOR_XOR_ASSIGN:
        case OPERATOR_XOR: {
            writeByteCode(vm->instance, OP_BINARY_XOR, token_index);
            break;
        }
        case OPERATOR_SHIFT_LEFT_ASSIGN:
        case OPERATOR_SHIFT_LEFT: {
            writeByteCode(vm->instance, OP_BINARY_SHIFT_LEFT, token_index);
            break;
        }
        case OPERATOR_SHIFT_RIGHT_ASSIGN:
        case OPERATOR_SHIFT_RIGHT: {
            writeByteCode(vm->instance, OP_BINARY_SHIFT_RIGHT, token_index);
            break;
        }
        case OPERATOR_POWER_ASSIGN:
        case OPERATOR_POWER: {
            writeByteCode(vm->instance, OP_BINARY_POWER, token_index);
            break;
        }
        case OPERATOR_ROOT: {
            writeByteCode(vm->instance, OP_BINARY_ROOT, token_index);
            break;
        }
        case OPERATOR_MODULO_ASSIGN:
        case OPERATOR_MODULO: {
            writeByteCode(vm->instance, OP_BINARY_MODULO, token_index);
            break;
        }
        case OPERATOR_MIN_ASSIGN:
        case OPERATOR_MIN: {
            writeByteCode(vm->instance, OP_BINARY_MIN, token_index);
            break;
        }
        case OPERATOR_MAX_ASSIGN:
        case OPERATOR_MAX: {
            writeByteCode(vm->instance, OP_BINARY_MAX, token_index);
            break;
        }

        case OPERATOR_HASH: {
            writeByteCode(vm->instance, OP_GETLIST, token_index);
            break;
        }
        case OPERATOR_ARROW: {
            writeByteCode(vm->instance, OP_GETOBJECT, token_index);
            break;
        }
        
        default: {
            return -1;
        }

    }
    return 0;

}

void initScopeData(ScopeData* scope) {
    scope->locals_count = 0;
    scope->locals_capacity = 0;
    scope->locals_lookup = NULL;
}

void pushScopeData (ScopeData* list, size_t item) {
    if (list->locals_capacity < list->locals_count + 1) {
        size_t oldCapacity = list->locals_capacity;
        list->locals_capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        list->locals_lookup = DOSATO_RESIZE_LIST(size_t, list->locals_lookup, oldCapacity, list->locals_capacity);
    }
    list->locals_lookup[list->locals_count] = item;
    list->locals_count++;
}

size_t popScopeData (ScopeData* list) {
    if (list->locals_count == 0) return 0;
    list->locals_count--;
    return list->locals_lookup[list->locals_count];
}

void freeScopeData(ScopeData* list) {
    DOSATO_FREE_LIST(size_t, list->locals_lookup, list->locals_capacity);
    list->locals_count = 0;
    list->locals_capacity = 0;
}

bool inScope(ScopeData* scope, size_t index) {
    if (scope == NULL) return false;
    for (size_t i = 0; i < scope->locals_count; i++) {
        if (scope->locals_lookup[i] == index) return true;
    }
    return false;
}

size_t getScopeIndex(ScopeData* scope, size_t index) {
    for (size_t i = 0; i < scope->locals_count; i++) {
        if (scope->locals_lookup[i] == index) return i;
    }
    return -1;
}