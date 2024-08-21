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
    switch (node.type) {
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
            if (ast.tokens.tokens[node.body.nodes[0].start].carry != TYPE_VAR)
                writeInstruction(vm->instance, node.start, OP_TYPE_CAST, ast.tokens.tokens[node.body.nodes[0].start].carry); // cast to the correct type (if not var)
            
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
            // push the value to the stack
            compileNode(vm, node.body.nodes[2], ast, scope);
            
            writeInstruction(vm->instance, node.body.nodes[0].start, OP_STORE, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.body.nodes[0].start].carry)); // store the value in the simple variable
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
            compileNode(vm, node.body.nodes[0], ast, scope);
            compileNode(vm, node.body.nodes[2], ast, scope);

            switch (ast.tokens.tokens[node.body.nodes[1].start].carry) {
                case OPERATOR_ADD: {
                    writeByteCode(vm->instance, OP_BINARY_ADD, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_SUBTRACT: {
                    writeByteCode(vm->instance, OP_BINARY_SUBTRACT, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_MULTIPLY: {
                    writeByteCode(vm->instance, OP_BINARY_MULTIPLY, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_DIVIDE: {
                    writeByteCode(vm->instance, OP_BINARY_DIVIDE, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_GREATER: {
                    writeByteCode(vm->instance, OP_BINARY_GREATER, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_LESS: {
                    writeByteCode(vm->instance, OP_BINARY_LESS, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_EQUAL: {
                    writeByteCode(vm->instance, OP_BINARY_EQUAL, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_NOT_EQUAL: {
                    writeByteCode(vm->instance, OP_BINARY_NOT_EQUAL, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_GREATER_EQUAL: {
                    writeByteCode(vm->instance, OP_BINARY_GREATER_EQUAL, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_LESS_EQUAL: {
                    writeByteCode(vm->instance, OP_BINARY_LESS_EQUAL, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_AND: {
                    writeByteCode(vm->instance, OP_BINARY_AND, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_OR: {
                    writeByteCode(vm->instance, OP_BINARY_OR, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_AND_AND: {
                    writeByteCode(vm->instance, OP_BINARY_LOGICAL_AND, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_OR_OR: {
                    writeByteCode(vm->instance, OP_BINARY_LOGICAL_OR, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_XOR: {
                    writeByteCode(vm->instance, OP_BINARY_XOR, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_SHIFT_LEFT: {
                    writeByteCode(vm->instance, OP_BINARY_SHIFT_LEFT, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_SHIFT_RIGHT: {
                    writeByteCode(vm->instance, OP_BINARY_SHIFT_RIGHT, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_POWER: {
                    writeByteCode(vm->instance, OP_BINARY_POWER, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_ROOT: {
                    writeByteCode(vm->instance, OP_BINARY_ROOT, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_MODULO: {
                    writeByteCode(vm->instance, OP_BINARY_MODULO, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_MIN: {
                    writeByteCode(vm->instance, OP_BINARY_MIN, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_MAX: {
                    writeByteCode(vm->instance, OP_BINARY_MAX, node.body.nodes[1].start);
                    break;
                }

                case OPERATOR_HASH: {
                    writeByteCode(vm->instance, OP_GETLIST, node.body.nodes[1].start);
                    break;
                }
                case OPERATOR_ARROW: {
                    writeByteCode(vm->instance, OP_GETOBJECT, node.body.nodes[1].start);
                    break;
                }
                
                default: {
                    ERROR(E_NON_BINARY_OPERATOR, node.body.nodes[1].start);
                }

            }

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
    }
}

#undef ERROR

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