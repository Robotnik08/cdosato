#include "../include/common.h"

#include "../include/compiler.h"
#include "../include/memory.h"
#include "../include/virtual-machine.h"
#include "../include/error.h"
#include "../include/code_instance.h"
#include "../include/debug.h"

void compile(VirtualMachine* vm, AST ast) { 

    // Compile the AST into byte code
    compileNode(vm, vm->instance, ast.root, ast, NULL);


    writeByteCode(vm->instance, OP_STOP, 0);
}

#define ERROR(e_code, index) printError(ast.source, ast.tokens.tokens[index].start - ast.source, e_code)

int compileNode (VirtualMachine* vm, CodeInstance* ci, Node node, AST ast, ScopeData* scope) {
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
                compileNode(vm, ci, node.body.nodes[i], ast, scope);
            }
            if (scope != NULL) {
                int count = scope->locals_count - scope_start;
                for (int i = 0; i < count; i++) {
                    writeByteCode(ci, OP_POP, 0);
                    popScopeData(scope);
                }
            }

            break;
        }

        case NODE_MASTER_RETURN:
        case NODE_MASTER_DEFINE:
        case NODE_MASTER_MAKE:
        case NODE_MASTER_SET:
        case NODE_MASTER_DO: {
            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            break;
        }

        case NODE_MASTER_RETURN_BODY: {
            if (scope->return_type != D_NULL) {
                if (node.body.count != 0) {
                    compileNode(vm, ci, node.body.nodes[0], ast, scope);
                    // cast to the correct type
                    writeInstruction(ci, node.start, OP_TYPE_CAST, scope->return_type);
                } else {
                    writeByteCode(ci, OP_PUSH_NULL, node.start);
                }
                writeInstruction(ci, node.start, OP_RETURN, DOSATO_SPLIT_SHORT(scope->locals_count));
            } else {
                ERROR(E_CANT_RETURN_OUTSIDE_FUNCTION, node.body.nodes[0].start);
            }
            break;
        }

        case NODE_MASTER_DEFINE_BODY: {
            // compile function

            if (scope != NULL) { 
                ERROR(E_MUST_BE_GLOBAL, node.start - 1);
            }

            DataType type = ast.tokens.tokens[node.body.nodes[0].start].carry;
            char* name = getTokenString(ast.tokens.tokens[node.body.nodes[1].start]);
            size_t name_index = ast.tokens.tokens[node.body.nodes[1].start].carry;
            CodeInstance* instance = malloc(sizeof(CodeInstance));
            initCodeInstance(instance);
            ScopeData new_scope;
            initScopeData(&new_scope);

            int arity = node.body.nodes[2].body.count;
            for (int i = 0; i < arity; i++) {
                pushScopeData(&new_scope, ast.tokens.tokens[node.body.nodes[2].body.nodes[i].body.nodes[1].start].carry);
            }
            new_scope.return_type = type;

            ScopeData* scope = &new_scope;
            compileNode(vm, instance, node.body.nodes[3], ast, scope);

            for (int i = 0; i < arity; i++) {
                writeByteCode(instance, OP_POP, node.body.nodes[2].body.nodes[i].end);
            }
            writeByteCode(instance, OP_END_FUNC, node.body.nodes[3].end);

            size_t* name_indexs = malloc(sizeof(size_t) * arity); 
            DataType* types = malloc(sizeof(DataType) * arity);
            for (int i = 0; i < arity; i++) {
                name_indexs[i] = ast.tokens.tokens[node.body.nodes[2].body.nodes[i].body.nodes[1].start].carry;
                types[i] = ast.tokens.tokens[node.body.nodes[2].body.nodes[i].body.nodes[0].start].carry;
            }          

            Function func;
            init_Function(&func);
            func.name = name;
            func.name_index = name_index;
            func.argv = name_indexs;
            func.argt = types;
            func.arity = node.body.nodes[2].body.count;
            func.instance = instance;
            func.return_type = type;

            write_FunctionList(&vm->functions, func);
            break;
        }

        case NODE_MASTER_DO_BODY: {
            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            // pop the return value
            if (ci->code[ci->count - 1] != OP_PRINT) writeByteCode(ci, OP_POP, node.body.nodes[0].end);
            break;
        }

        case NODE_MASTER_MAKE_BODY: {
            // push the value to the stack
            if (scope != NULL) { // local scope (push null to the stack, used for the local variable)
                writeByteCode(ci, OP_PUSH_NULL, node.body.nodes[1].start);
            }

            compileNode(vm, ci, node.body.nodes[2], ast, scope);
            writeInstruction(ci, node.start, OP_TYPE_CAST, ast.tokens.tokens[node.body.nodes[0].start].carry); // cast to the correct type
            
            if (scope == NULL) { // global scope
                writeInstruction(ci, node.body.nodes[1].start, OP_DEFINE, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.body.nodes[1].start].carry));
            } else {
                if (inScope(scope, ast.tokens.tokens[node.body.nodes[1].start].carry)) {
                    ERROR(E_ALREADY_DEFINED_VARIABLE, node.body.nodes[1].start);
                }
                writeInstruction(ci, node.body.nodes[1].start, OP_STORE_FAST, DOSATO_SPLIT_SHORT(scope->locals_count));
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
                compileNode(vm, ci, left, ast, scope);
                // push the value to the stack
                compileNode(vm, ci, node.body.nodes[2], ast, scope);

                writeOperatorInstruction(ci, operator, node.body.nodes[1].start);
            } else {
                // push the value to the stack
                compileNode(vm, ci, node.body.nodes[2], ast, scope);
            }

            

            if (node.body.nodes[0].type == NODE_IDENTIFIER) {
                if (inScope(scope, ast.tokens.tokens[node.body.nodes[0].start].carry)) {
                    size_t index = getScopeIndex(scope, ast.tokens.tokens[node.body.nodes[0].start].carry);
                    OpCode op = operator == OPERATOR_INCREMENT ? OP_INCREMENT_FAST : operator == OPERATOR_DECREMENT ? OP_DECREMENT_FAST : OP_STORE_FAST;
                    writeInstruction(ci, node.body.nodes[0].start, op, DOSATO_SPLIT_SHORT(index));
                } else {
                    OpCode op = operator == OPERATOR_INCREMENT ? OP_INCREMENT : operator == OPERATOR_DECREMENT ? OP_DECREMENT : OP_STORE;
                    writeInstruction(ci, node.body.nodes[0].start, op, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.body.nodes[0].start].carry));
                }
            } else if (node.body.nodes[0].type == NODE_SUBSCRIPT_EXPRESSION) {
                if (node.body.nodes[0].body.nodes[0].type != NODE_IDENTIFIER) {
                    compileNode(vm, ci, node.body.nodes[0].body.nodes[0], ast, scope); // the left side of the subscript (the object)
                } else {
                    if (inScope(scope, ast.tokens.tokens[node.body.nodes[0].body.nodes[0].start].carry)) {
                        writeInstruction(ci, node.body.nodes[0].body.nodes[0].start, OP_REFERENCE_FAST, DOSATO_SPLIT_SHORT(getScopeIndex(scope, ast.tokens.tokens[node.body.nodes[0].body.nodes[0].start].carry)));
                    } else {
                        writeInstruction(ci, node.body.nodes[0].body.nodes[0].start, OP_REFERENCE, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.body.nodes[0].body.nodes[0].start].carry));
                    }
                }

                compileNode(vm, ci, node.body.nodes[0].body.nodes[2], ast, scope); // the right side of the subscript (the object)

                OperatorType operator_inner = ast.tokens.tokens[node.body.nodes[0].body.nodes[1].start].carry; // operator for the subscript

                OpCode op = operator_inner == OPERATOR_HASH ? 
                    (operator == OPERATOR_INCREMENT ? OP_INCREMENT_SUBSCR : operator == OPERATOR_DECREMENT ? OP_DECREMENT_SUBSCR : OP_STORE_SUBSCR) 
                    : (operator == OPERATOR_INCREMENT ? OP_INCREMENT_OBJ : operator == OPERATOR_DECREMENT ? OP_DECREMENT_OBJ : OP_STORE_OBJ);
                    
                writeByteCode(ci, op, node.body.nodes[0].start);
            }
            
            break;
        }

        case NODE_SUBSCRIPT_EXPRESSION: {
            if (node.body.nodes[0].type != NODE_IDENTIFIER) {
                compileNode(vm, ci, node.body.nodes[0], ast, scope); // the left side of the subscript (the object)
            } else {
                if (inScope(scope, ast.tokens.tokens[node.body.nodes[0].start].carry)) {
                    writeInstruction(ci, node.body.nodes[0].start, OP_REFERENCE_FAST, DOSATO_SPLIT_SHORT(getScopeIndex(scope, ast.tokens.tokens[node.body.nodes[0].start].carry)));
                } else {
                    writeInstruction(ci, node.body.nodes[0].start, OP_REFERENCE, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.body.nodes[0].start].carry));
                }
            }
            
            if (node.body.count != 3) {
                ERROR(E_INVALID_EXPRESSION, node.start);
            }

            OperatorType operator = ast.tokens.tokens[node.body.nodes[1].start].carry;

            compileNode(vm, ci, node.body.nodes[2], ast, scope);

            if (ast.tokens.tokens[node.body.nodes[1].start].carry != OPERATOR_HASH && ast.tokens.tokens[node.body.nodes[1].start].carry != OPERATOR_ARROW) {
                ERROR(E_EXPECTED_HASH_OPERATOR, node.body.nodes[1].start);
            }
            writeByteCode(ci, operator == OPERATOR_HASH ? OP_REFERENCE_SUBSCR : OP_REFERENCE_GETOBJ, node.body.nodes[1].start);
            break;
        }

        case NODE_FUNCTION_CALL: {

            // debug force SAY to be the print function
            if (strcmp(getTokenString(ast.tokens.tokens[node.body.nodes[0].start]), "SAY") == 0) {
                // push first argument
                compileNode(vm, ci, node.body.nodes[1].body.nodes[0], ast, scope);
                writeInstruction(ci, node.start, OP_PRINT, 0);
                break;
            }


            // push arguments

            int arity = node.body.nodes[1].body.count;
            for (int i = 0; i < arity; i++) {
                compileNode(vm, ci, node.body.nodes[1].body.nodes[i], ast, scope);
            }
            // push the function
            compileNode(vm, ci, node.body.nodes[0], ast, scope);

            writeInstruction(ci, node.start, OP_CALL, arity); 
            break;
        }


        case NODE_EXPRESSION: {
            if (node.body.count == 1) {
                compileNode(vm, ci, node.body.nodes[0], ast, scope);
                break;
            }
            if (node.body.nodes[0].type == NODE_SUBSCRIPT_EXPRESSION && node.type == NODE_TEMP_SUBSCRIPT_EXPRESSION) {
                Node sub = node.body.nodes[0];
                sub.type = NODE_EXPRESSION;
                compileNode(vm, ci, sub, ast, scope);
            } else {
                compileNode(vm, ci, node.body.nodes[0], ast, scope);
            }

            compileNode(vm, ci, node.body.nodes[2], ast, scope);

            int res = writeOperatorInstruction(ci, ast.tokens.tokens[node.body.nodes[1].start].carry, node.body.nodes[1].start);
            if (res == -1) {
                ERROR(E_NON_BINARY_OPERATOR, node.body.nodes[1].start);
            }

            break;
        }

        case NODE_UNARY_EXPRESSION: {
            compileNode(vm, ci, node.body.nodes[1], ast, scope);
            int res = writeUnaryInstruction(ci, ast.tokens.tokens[node.body.nodes[0].start].carry, node.body.nodes[0].start);
            if (res == -1) {
                ERROR(E_NON_UNARY_OPERATOR, node.body.nodes[0].start);
            }
            break;
        }

        case NODE_TYPE_CAST: {
            compileNode(vm, ci, node.body.nodes[1], ast, scope);
            writeInstruction(ci, node.start, OP_TYPE_CAST, ast.tokens.tokens[node.body.nodes[0].start].carry);
            break;
        }
        
        case NODE_STRING_LITERAL:
        case NODE_NUMBER_LITERAL: {
            writeInstruction(ci, node.start, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.start].carry));
            break;
        }

        case NODE_IDENTIFIER: {
            if (scope == NULL) { // global scope
                writeInstruction(ci, node.start, OP_LOAD, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.start].carry)); // load the global variable
            } else {
                if (!inScope(scope, ast.tokens.tokens[node.start].carry)) {
                    writeInstruction(ci, node.start, OP_LOAD, DOSATO_SPLIT_SHORT(ast.tokens.tokens[node.start].carry)); // load the global variable
                    break;
                }
                size_t index = getScopeIndex(scope, ast.tokens.tokens[node.start].carry);
                if (index == -1) {
                    ERROR(E_UNDEFINED_VARIABLE, node.start);
                }
                writeInstruction(ci, node.start, OP_LOAD_FAST, DOSATO_SPLIT_SHORT(index)); // load the local variable
            }
            break;
        }

        case NODE_ARRAY_EXPRESSION: {
            for (int i = node.body.count - 1; i >= 0; i--) {
                compileNode(vm, ci, node.body.nodes[i], ast, scope);
            }
            writeInstruction(ci, node.start, OP_BUILD_LIST, DOSATO_SPLIT_SHORT(node.body.count)); // cast to the correct type
            break;
        }

        case NODE_OBJECT_EXPRESSION: {
            for (int i = node.body.count - 1; i >= 0; i--) {
                compileNode(vm, ci, node.body.nodes[i], ast, scope);
            }
            writeInstruction(ci, node.start, OP_BUILD_OBJECT, DOSATO_SPLIT_SHORT(node.body.count)); // cast to the correct type
            break;
        }

        case NODE_OBJECT_ENTRY: {
            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            compileNode(vm, ci, node.body.nodes[1], ast, scope);
            break;
        }
    }
    return 0;
}

#undef ERROR

int writeOperatorInstruction (CodeInstance* ci, OperatorType operator, size_t token_index) {
    switch (operator) {
        case OPERATOR_ADD_ASSIGN:
        case OPERATOR_ADD: {
            writeByteCode(ci, OP_BINARY_ADD, token_index);
            break;
        }
        case OPERATOR_SUBTRACT_ASSIGN:
        case OPERATOR_SUBTRACT: {
            writeByteCode(ci, OP_BINARY_SUBTRACT, token_index);
            break;
        }
        case OPERATOR_MULTIPLY_ASSIGN:
        case OPERATOR_MULTIPLY: {
            writeByteCode(ci, OP_BINARY_MULTIPLY, token_index);
            break;
        }
        case OPERATOR_DIVIDE_ASSIGN:
        case OPERATOR_DIVIDE: {
            writeByteCode(ci, OP_BINARY_DIVIDE, token_index);
            break;
        }
        case OPERATOR_GREATER: {
            writeByteCode(ci, OP_BINARY_GREATER, token_index);
            break;
        }
        case OPERATOR_LESS: {
            writeByteCode(ci, OP_BINARY_LESS, token_index);
            break;
        }
        case OPERATOR_EQUAL: {
            writeByteCode(ci, OP_BINARY_EQUAL, token_index);
            break;
        }
        case OPERATOR_NOT_EQUAL: {
            writeByteCode(ci, OP_BINARY_NOT_EQUAL, token_index);
            break;
        }
        case OPERATOR_GREATER_EQUAL: {
            writeByteCode(ci, OP_BINARY_GREATER_EQUAL, token_index);
            break;
        }
        case OPERATOR_LESS_EQUAL: {
            writeByteCode(ci, OP_BINARY_LESS_EQUAL, token_index);
            break;
        }
        case OPERATOR_AND_ASSIGN:
        case OPERATOR_AND: {
            writeByteCode(ci, OP_BINARY_AND_BITWISE, token_index);
            break;
        }
        case OPERATOR_OR_ASSIGN:
        case OPERATOR_OR: {
            writeByteCode(ci, OP_BINARY_OR_BITWISE, token_index);
            break;
        }
        case OPERATOR_AND_AND: {
            writeByteCode(ci, OP_BINARY_LOGICAL_AND, token_index);
            break;
        }
        case OPERATOR_OR_OR: {
            writeByteCode(ci, OP_BINARY_LOGICAL_OR, token_index);
            break;
        }
        case OPERATOR_XOR_ASSIGN:
        case OPERATOR_XOR: {
            writeByteCode(ci, OP_BINARY_XOR_BITWISE, token_index);
            break;
        }
        case OPERATOR_SHIFT_LEFT_ASSIGN:
        case OPERATOR_SHIFT_LEFT: {
            writeByteCode(ci, OP_BINARY_SHIFT_LEFT, token_index);
            break;
        }
        case OPERATOR_SHIFT_RIGHT_ASSIGN:
        case OPERATOR_SHIFT_RIGHT: {
            writeByteCode(ci, OP_BINARY_SHIFT_RIGHT, token_index);
            break;
        }
        case OPERATOR_POWER_ASSIGN:
        case OPERATOR_POWER: {
            writeByteCode(ci, OP_BINARY_POWER, token_index);
            break;
        }
        case OPERATOR_ROOT: {
            writeByteCode(ci, OP_BINARY_ROOT, token_index);
            break;
        }
        case OPERATOR_MODULO_ASSIGN:
        case OPERATOR_MODULO: {
            writeByteCode(ci, OP_BINARY_MODULO, token_index);
            break;
        }
        case OPERATOR_MIN_ASSIGN:
        case OPERATOR_MIN: {
            writeByteCode(ci, OP_BINARY_MIN, token_index);
            break;
        }
        case OPERATOR_MAX_ASSIGN:
        case OPERATOR_MAX: {
            writeByteCode(ci, OP_BINARY_MAX, token_index);
            break;
        }

        case OPERATOR_HASH: {
            writeByteCode(ci, OP_GETLIST, token_index);
            break;
        }
        case OPERATOR_ARROW: {
            writeByteCode(ci, OP_GETOBJECT, token_index);
            break;
        }
        
        default: {
            return -1;
        }

    }
    return 0;

}

int writeUnaryInstruction (CodeInstance* ci, OperatorType operator, size_t token_index) {
    switch (operator) {
        case OPERATOR_SUBTRACT: {
            writeByteCode(ci, OP_UNARY_NEGATE, token_index);
            break;
        }
        case OPERATOR_NOT: {
            writeByteCode(ci, OP_UNARY_LOGICAL_NOT, token_index);
            break;
        }
        case OPERATOR_NOT_BITWISE: {
            writeByteCode(ci, OP_UNARY_BITWISE_NOT, token_index);
            break;
        }
        case OPERATOR_ROOT: {
            writeByteCode(ci, OP_UNARY_SQRT, token_index);
            break;
        }
        case OPERATOR_ABSOLUTE: {
            writeByteCode(ci, OP_UNARY_ABSOLUTE, token_index);
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
    scope->return_type = D_NULL;
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