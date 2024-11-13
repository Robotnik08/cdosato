#include "../include/common.h"

#include "../include/compiler.h"
#include "../include/memory.h"
#include "../include/virtual-machine.h"
#include "../include/error.h"
#include "../include/code_instance.h"
#include "../include/debug.h"
#include "../include/dynamic_library_loader.h"

void compile(VirtualMachine* vm, AST* ast) {

    // Compile the AST into byte code
    vm->instance->ast = ast;
    compileNode(vm, vm->instance, ast->root, ast, NULL);


    writeByteCode(vm->instance, OP_STOP, 0);

}

#define PRINT_ERROR(e_code, index) printError(ast->source, ast->tokens.tokens[index].start - ast->source, ast->name, e_code, ast->tokens.tokens[index].length)

int compileNode (VirtualMachine* vm, CodeInstance* ci, Node node, AST* ast, ScopeData* scope) {
    NodeType type = node.type;
    if (node.type == NODE_TEMP_SUBSCRIPT_EXPRESSION) {
        type = NODE_EXPRESSION;
    }
    bool created_scope = false;

    switch (type) {
        default: {
            break; // do nothing
        }

        case NODE_BLOCK: {
            if (scope == NULL) { // first scope after global
                ScopeData* new_scope = malloc(sizeof(ScopeData));
                initScopeData(new_scope);
                scope = new_scope;
                created_scope = true;
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
                if (created_scope) {
                    freeScopeData(scope);
                    free(scope);
                }
            }

            break;
        }

        case NODE_MASTER_METHOD:
        case NODE_MASTER_CLASS:
        case NODE_MASTER_CONST:
        case NODE_MASTER_IMPORT:
        case NODE_MASTER_INCLUDE:
        case NODE_MASTER_DEFINE:
        case NODE_MASTER_MAKE: {
            if (node.body.nodes[0].type != NODE_MASTER_DO_BODY + type - 1 || node.body.count > 1) {
                PRINT_ERROR(E_MASTER_CANT_HAVE_EXTENSIONS, node.start);
            }
            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            break;      
        }

        case NODE_MASTER_SWITCH:
        case NODE_MASTER_CONTINUE:
        case NODE_MASTER_BREAK:
        case NODE_MASTER_RETURN:
        case NODE_MASTER_SET:
        case NODE_MASTER_DO: {
            NodeType last_node_type = 0;
            int last_result = -1;
            for (int i = 0; i < node.body.count; i++) {
                if (node.body.nodes[i].type == NODE_ELSE_BODY) {
                    if (last_node_type == NODE_WHEN_BODY) {
                        // write Jump instruction
                        writeInstruction(ci, node.body.nodes[i].start, OP_JUMP, DOSATO_SPLIT_SHORT(0));
                        int jump_index = ci->count - getOffset(OP_JUMP);
                        compileNode(vm, ci, node.body.nodes[i], ast, scope);
                        // update the jump instruction
                        ci->code[jump_index + 1] = ci->count & 0xFF;
                        ci->code[jump_index + 2] = ci->count >> 8;
                        // update the jump instruction for the if statement (the last_result is the jump instruction of the if statement)
                        ci->code[last_result + 1] = (jump_index + getOffset(OP_JUMP)) & 0xFF;
                        ci->code[last_result + 2] = (jump_index + getOffset(OP_JUMP)) >> 8;
                    } else {
                        PRINT_ERROR(E_ELSE_WITHOUT_IF, node.body.nodes[i].start - 1);
                    }
                } else {
                    last_result = compileNode(vm, ci, node.body.nodes[i], ast, scope);
                }
                last_node_type = node.body.nodes[i].type;
            }
            break;
        }

        case NODE_MASTER_RETURN_BODY: {
            if (scope == NULL) {
                PRINT_ERROR(E_CANT_RETURN_OUTSIDE_FUNCTION, node.start - 1);
            }
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
                PRINT_ERROR(E_CANT_RETURN_OUTSIDE_FUNCTION, node.start - 1);
            }
            break;
        }

        case NODE_MASTER_DEFINE_BODY: {
            if (scope != NULL) { 
                PRINT_ERROR(E_MUST_BE_GLOBAL, node.start - 1);
            }

            DataType data_type = TYPE_VAR;
            int identifier_index = 0;
            if (node.body.count == 4) {
                data_type = ast->tokens.tokens[node.body.nodes[0].start].carry;
                identifier_index = 1;
            }

            char* name = getTokenString(ast->tokens.tokens[node.body.nodes[identifier_index].start]);
            // check if function is already in the function list
            for (int i = 0; i < vm->functions.count; i++) {
                if (strcmp(vm->functions.funcs[i].name, name) == 0) {
                    PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, node.body.nodes[identifier_index].start);
                }
            }

            size_t name_index = ast->tokens.tokens[node.body.nodes[identifier_index].start].carry;
            CodeInstance* instance = malloc(sizeof(CodeInstance));
            initCodeInstance(instance);
            instance->ast = ast;
            ScopeData* new_scope = malloc(sizeof(ScopeData));
            initScopeData(new_scope);

            int arity = node.body.nodes[identifier_index + 1].body.count;
            int* hash_map = malloc(sizeof(int) * arity);
            for (int i = 0; i < arity; i++) {
                Node arg = node.body.nodes[identifier_index + 1].body.nodes[i];
                int carry = ast->tokens.tokens[arg.body.nodes[arg.body.count - 1].start].carry;
                for (int j = 0; j < i; j++) {
                    if (hash_map[j] == carry) {
                        PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, arg.body.nodes[arg.body.count - 1].start);
                    }
                }
                hash_map[i] = carry;
                pushScopeData(new_scope, carry);
            }
            free(hash_map);
            new_scope->return_type = data_type;

            compileNode(vm, instance, node.body.nodes[identifier_index + 2], ast, new_scope);

            freeScopeData(new_scope);
            free(new_scope);

            for (int i = 0; i < arity; i++) {
                writeByteCode(instance, OP_POP, node.body.nodes[identifier_index + 1].body.nodes[i].end);
            }
            writeByteCode(instance, OP_END_FUNC, node.body.nodes[identifier_index + 2].end);

            size_t* name_indexs = malloc(sizeof(size_t) * arity); 
            DataType* types = malloc(sizeof(DataType) * arity);
            for (int i = 0; i < arity; i++) {
                NodeList list = node.body.nodes[identifier_index + 1].body.nodes[i].body;
                name_indexs[i] = ast->tokens.tokens[list.nodes[list.count - 1].start].carry;
                types[i] = list.count == 1 ? TYPE_VAR : ast->tokens.tokens[list.nodes[0].start].carry;
            }          

            Function func;
            init_Function(&func);
            func.name = name;
            func.name_index = name_index;
            func.argv = name_indexs;
            func.argt = types;
            func.arity = node.body.nodes[identifier_index + 1].body.count;
            func.instance = instance;
            func.return_type = data_type;

            func.captured_count = 0;

            func.captured = NULL;
            func.captured_indices = NULL;

            write_FunctionList(&vm->functions, func);

            break;
        }

        case NODE_MASTER_METHOD_BODY: {
            if (scope != NULL) { 
                PRINT_ERROR(E_MUST_BE_GLOBAL, node.start - 1);
            }

            DataType data_type = TYPE_VAR;
            int identifier_index = 0;
            if (node.body.count == 4) {
                data_type = ast->tokens.tokens[node.body.nodes[0].start].carry;
                identifier_index = 1;
            }

            char* name = getTokenString(ast->tokens.tokens[node.body.nodes[identifier_index].start]);
            // check if function is already in the function list
            for (int i = 0; i < vm->functions.count; i++) {
                if (strcmp(vm->functions.funcs[i].name, name) == 0) {
                    PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, node.body.nodes[identifier_index].start);
                }
            }

            size_t name_index = ast->tokens.tokens[node.body.nodes[identifier_index].start].carry;
            CodeInstance* instance = malloc(sizeof(CodeInstance));
            initCodeInstance(instance);
            instance->ast = ast;
            ScopeData* new_scope = malloc(sizeof(ScopeData));
            initScopeData(new_scope);

            int arity = node.body.nodes[identifier_index + 1].body.count;
            int* hash_map = malloc(sizeof(int) * arity);
            for (int i = 0; i < arity; i++) {
                Node arg = node.body.nodes[identifier_index + 1].body.nodes[i];
                int carry = ast->tokens.tokens[arg.body.nodes[arg.body.count - 1].start].carry;
                for (int j = 0; j < i; j++) {
                    if (hash_map[j] == carry) {
                        PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, arg.body.nodes[arg.body.count - 1].start);
                    }
                }
                hash_map[i] = carry;
                pushScopeData(new_scope, carry);
            }
            free(hash_map);
            new_scope->return_type = data_type;

            compileNode(vm, instance, node.body.nodes[identifier_index + 2], ast, new_scope);

            freeScopeData(new_scope);
            free(new_scope);

            for (int i = 0; i < arity; i++) {
                writeByteCode(instance, OP_POP, node.body.nodes[identifier_index + 1].body.nodes[i].end);
            }
            writeByteCode(instance, OP_END_FUNC, node.body.nodes[identifier_index + 2].end);

            size_t* name_indexs = malloc(sizeof(size_t) * arity); 
            DataType* types = malloc(sizeof(DataType) * arity);
            for (int i = 0; i < arity; i++) {
                NodeList list = node.body.nodes[identifier_index + 1].body.nodes[i].body;
                name_indexs[i] = ast->tokens.tokens[list.nodes[list.count - 1].start].carry;
                types[i] = list.count == 1 ? TYPE_VAR : ast->tokens.tokens[list.nodes[0].start].carry;
            }          

            Function func;
            init_Function(&func);
            func.name = name;
            func.name_index = name_index;
            func.argv = name_indexs;
            func.argt = types;
            func.arity = node.body.nodes[identifier_index + 1].body.count;
            func.instance = instance;
            func.return_type = data_type;

            func.captured_count = 0;

            func.captured = NULL;
            func.captured_indices = NULL;

            write_FunctionList(&vm->functions, func);
            break;
        }
        
        case NODE_MASTER_CLASS_BODY: {
            if (scope != NULL) { 
                PRINT_ERROR(E_MUST_BE_GLOBAL, node.start - 1);
            }

            char* name = getTokenString(ast->tokens.tokens[node.body.nodes[0].start]);
            // check if function is already in the function list
            for (int i = 0; i < vm->functions.count; i++) {
                if (strcmp(vm->functions.funcs[i].name, name) == 0) {
                    PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, node.body.nodes[0].start);
                }
            }

            size_t name_index = ast->tokens.tokens[node.body.nodes[0].start].carry;
            CodeInstance* instance = malloc(sizeof(CodeInstance));
            initCodeInstance(instance);
            instance->ast = ast;
            ScopeData* new_scope = malloc(sizeof(ScopeData));
            initScopeData(new_scope);
            new_scope->is_class = true;

            int arity = node.body.nodes[1].body.count;
            int* hash_map = malloc(sizeof(int) * arity);
            for (int i = 0; i < arity; i++) {
                Node arg = node.body.nodes[1].body.nodes[i];
                int carry = ast->tokens.tokens[arg.body.nodes[arg.body.count - 1].start].carry;
                for (int j = 0; j < i; j++) {
                    if (hash_map[j] == carry) {
                        PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, arg.body.nodes[arg.body.count - 1].start);
                    }
                }
                hash_map[i] = carry;
                if (carry <= 1) {
                    PRINT_ERROR(E_INVALID_IDENTIFIER, arg.body.nodes[arg.body.count - 1].start);
                }
                pushScopeData(new_scope, carry);
            }
            
            free(hash_map);
            new_scope->return_type = D_NULL;

            int self_index = 1;

            // add self to the scope
            pushScopeData(new_scope, self_index);

            writeByteCode(instance, OP_PUSH_NULL, node.start);
            writeInstruction(instance, node.body.nodes[2].start - 1, OP_BUILD_OBJECT, DOSATO_SPLIT_SHORT(0));
            writeByteCode(instance, OP_MARK_CONSTANT, node.start);
            writeInstruction(instance, node.body.nodes[2].start - 1, OP_STORE_FAST, DOSATO_SPLIT_SHORT(arity));
            writeByteCode(instance, OP_POP, node.start);

            compileNode(vm, instance, node.body.nodes[2], ast, new_scope);

            freeScopeData(new_scope);
            free(new_scope);

            writeInstruction(instance, node.body.nodes[2].end, OP_RETURN, DOSATO_SPLIT_SHORT(arity));
            writeByteCode(instance, OP_END_FUNC, node.body.nodes[2].end);

            size_t* name_indexs = malloc(sizeof(size_t) * arity); 
            DataType* types = malloc(sizeof(DataType) * arity);
            for (int i = 0; i < arity; i++) {
                NodeList list = node.body.nodes[1].body.nodes[i].body;
                name_indexs[i] = ast->tokens.tokens[list.nodes[list.count - 1].start].carry;
                types[i] = list.count == 1 ? TYPE_VAR : ast->tokens.tokens[list.nodes[0].start].carry;
            }          

            Function func;
            init_Function(&func);
            func.name = name;
            func.name_index = name_index;
            func.argv = name_indexs;
            func.argt = types;
            func.arity = node.body.nodes[1].body.count;
            func.instance = instance;
            func.return_type = TYPE_OBJECT;

            func.captured_count = 0;

            func.captured = NULL;
            func.captured_indices = NULL;

            func.is_class = true;

            write_FunctionList(&vm->functions, func);

            break;
        }

        case NODE_ELSE_BODY:
        case NODE_THEN_BODY:
        case NODE_MASTER_DO_BODY: {
            // empty body does nothing
            if (node.body.count == 0) {
                break;
            }

            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            // pop the return value
            if (node.body.nodes[0].type == NODE_FUNCTION_CALL) writeByteCode(ci, OP_POP, node.body.nodes[0].end);
            break;
        }

        case NODE_MASTER_INCLUDE_BODY: {
            if (scope != NULL) {
                PRINT_ERROR(E_MUST_BE_GLOBAL, node.start - 1);
            }

            if (node.body.nodes[0].type != NODE_STRING_LITERAL) {
                PRINT_ERROR(E_EXPECTED_STRING, node.body.nodes[0].start);
            }

            char* pathValue = AS_STRING(vm->constants.values[ast->tokens.tokens[node.body.nodes[0].start].carry]);
            char* path = malloc(strlen(pathValue) + 1);
            strcpy(path, pathValue);
            AST* included_ast = malloc(sizeof(AST));
            init_AST(included_ast);
            parseFile(included_ast, path, 0, vm);

            CodeInstance included_instance;
            initCodeInstance(&included_instance);

            included_instance.ast = included_ast;

            int index = vm->includes.count; // before compiling the included file, increment the include count (so we know if we have too many includes)
            write_CodeInstanceList(&vm->includes, included_instance);
            
            if (vm->includes.count > MAX_INCLUDES) {
                PRINT_ERROR(E_TOO_MANY_INCLUDES, node.start);
            }

            // set working directory to the included file's directory
            #ifdef _WIN32
            char* last_slash = strrchr(path, '\\');
            #else
            char* last_slash = strrchr(path, '/');
            #endif

            if (last_slash != NULL) {
                *last_slash = '\0';
                chdir(path);
            }

            compileNode(vm, &included_instance, included_ast->root, included_ast, NULL);

            // reset working directory
            if (last_slash != NULL) {
                *last_slash = '/';
                chdir(".."); // go back to the previous directory
            }

            // push OP_END_INCLUDE
            writeByteCode(&included_instance, OP_END_INCLUDE, node.start);

            writeInstruction(ci, node.start, OP_INCLUDE, DOSATO_SPLIT_SHORT(index));

            // update the include
            vm->includes.instances[index] = included_instance;

            break;
        }

        case NODE_MASTER_IMPORT_BODY: {
            if (scope != NULL) {
                PRINT_ERROR(E_MUST_BE_GLOBAL, node.start - 1);
            }

            if (node.body.nodes[0].type != NODE_STRING_LITERAL) {
                PRINT_ERROR(E_EXPECTED_STRING, node.body.nodes[0].start);
            }

            char* path = malloc(strlen(AS_STRING(vm->constants.values[ast->tokens.tokens[node.body.nodes[0].start].carry])) + 1);
            strcpy(path, AS_STRING(vm->constants.values[ast->tokens.tokens[node.body.nodes[0].start].carry]));
            
            DynamicLibrary lib = loadLib(path);
            
            // register functions
            for (int i = 0; i < lib.functions.count; i++) {
                DosatoFunctionMap func = lib.functions.functions[i];

                // check if function is already in the function list
                for (int j = 0; j < vm->functions.count; j++) {
                    if (strcmp(vm->functions.funcs[j].name, func.name) == 0) {
                        PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, node.body.nodes[0].start);
                    }
                }

                Function new_func;
                init_Function(&new_func);
                new_func.name = malloc(strlen(func.name) + 1);
                strcpy(new_func.name, func.name);

                // check if name is in name map
                size_t name_index = -1;
                for (int j = 0; j < vm->mappings.count; j++) {
                    if (strcmp(vm->mappings.names[j], new_func.name) == 0) {
                        name_index = j;
                        break;
                    }
                }
                // if not, add it
                if (name_index == -1) {
                    name_index = vm->mappings.count;
                    char* new_name = malloc(strlen(new_func.name) + 1);
                    strcpy(new_name, new_func.name);
                    write_NameMap(&vm->mappings, new_name);
                }
                new_func.name_index = name_index;
                
                new_func.func_ptr = func.function;
                new_func.is_compiled = true;

                write_FunctionList(&vm->functions, new_func);
            }

            free_DynamicLibrary(&lib);

            free(path);

            break;
        }

        case NODE_MASTER_CONST_BODY:
        case NODE_MASTER_MAKE_BODY: {

            int identifier_index = 0;
            DataType data_type = TYPE_VAR;
            if (node.body.nodes[0].type == NODE_TYPE) {
                if (ast->tokens.tokens[node.body.nodes[1].start].carry == 0) {
                    PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, node.body.nodes[1].start);
                }
                data_type = ast->tokens.tokens[node.body.nodes[0].start].carry;
                identifier_index = 1;
            }

            int operator_index = -1;
            for (int i = identifier_index; i < node.body.count; i++) {
                if (node.body.nodes[i].type == NODE_OPERATOR) {
                    operator_index = i;
                    break;
                }
            }

            if (operator_index == -1) {
                PRINT_ERROR(E_EXPECTED_ASSIGNMENT_OPERATOR_PURE, node.start);
            }

            if (scope != NULL) { // local scope (push null to the stack, used for the local variable)
                for (int i = identifier_index; i < operator_index; i++) {
                    writeByteCode(ci, OP_PUSH_NULL, node.body.nodes[i].start);
                }
            }

            bool is_tuple = false;
            int value_count = node.body.count - operator_index - 1;
            if (value_count == operator_index - identifier_index) {
                is_tuple = true;
            } else if (value_count != 1) {
                PRINT_ERROR(E_INVALID_AMOUNT_SET_EXPRESSION, node.start + identifier_index);
            }

            
            for (int i = node.body.count - 1; i >= operator_index + 1; i--) {
                compileNode(vm, ci, node.body.nodes[i], ast, scope);
                writeInstruction(ci, node.start, OP_TYPE_CAST, data_type); // cast to the correct type

                if (type == NODE_MASTER_CONST_BODY) {
                    writeByteCode(ci, OP_MARK_CONSTANT, node.start);
                }
            }


            if (scope == NULL) { // global scope
                for (int i = identifier_index; i < operator_index; i++) {
                    if (ast->tokens.tokens[node.body.nodes[i].start].carry == 0) {
                        PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, node.body.nodes[i].start);
                    }
                    writeInstruction(ci, node.body.nodes[i].start, OP_DEFINE, DOSATO_SPLIT_SHORT(ast->tokens.tokens[node.body.nodes[i].start].carry));

                    if (is_tuple) {
                        writeByteCode(ci, OP_POP, node.start);
                    }
                }
                if (!is_tuple) {
                    writeByteCode(ci, OP_POP, node.start);
                }
            } else {
                for (int i = identifier_index; i < operator_index; i++) {
                    if (inScope(scope, ast->tokens.tokens[node.body.nodes[i].start].carry)) {
                        PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, node.body.nodes[i].start);
                    }
                    writeInstruction(ci, node.body.nodes[i].start, OP_STORE_FAST, DOSATO_SPLIT_SHORT(scope->locals_count));
                    pushScopeData(scope, ast->tokens.tokens[node.body.nodes[i].start].carry);

                    if (is_tuple) {
                        writeByteCode(ci, OP_POP, node.start);
                    }
                }
                if (!is_tuple) {
                    writeByteCode(ci, OP_POP, node.start);
                }
            }
            
            break;
        }

        case NODE_MASTER_SET_BODY: {

            // if operator is not an assignment operator, then it is a binary operator
            OperatorType operator = OPERATOR_NULL;
            int operator_index = -1;
            bool tuple = false;
            if (node.body.nodes[node.body.count - 1].type == NODE_OPERATOR) {
                operator = ast->tokens.tokens[node.body.nodes[node.body.count - 1].start].carry;
                operator_index = node.body.count - 1;
            } else if (node.body.nodes[node.body.count - 2].type == NODE_OPERATOR) {
                operator = ast->tokens.tokens[node.body.nodes[node.body.count - 2].start].carry;
                operator_index = node.body.count - 2;
            } else if (node.body.nodes[node.body.count / 2].type == NODE_OPERATOR && node.body.count % 2 == 1) {
                operator = ast->tokens.tokens[node.body.nodes[node.body.count / 2].start].carry;
                operator_index = node.body.count / 2;
                tuple = true;
            } else {
                PRINT_ERROR(E_INVALID_AMOUNT_SET_EXPRESSION, node.start);
            }

            if (!tuple) {
                if (operator == OPERATOR_INCREMENT || operator == OPERATOR_DECREMENT) {
                    // do nothing, handled later
                } else if (operator != OPERATOR_ASSIGN) {
                    // compile the left side of the expression
                    for (int i = 0; i < operator_index; i++) {
                        Node left = node.body.nodes[i];
                        left.type = left.type == NODE_SUBSCRIPT_EXPRESSION ? NODE_TEMP_SUBSCRIPT_EXPRESSION : left.type;
                        compileNode(vm, ci, left, ast, scope);
                        // push the value to the stack
                        compileNode(vm, ci, node.body.nodes[operator_index + 1], ast, scope);

                        writeOperatorInstruction(ci, operator, node.body.nodes[operator_index].start);
                    }
                } else {
                    // push the value to the stack
                    compileNode(vm, ci, node.body.nodes[operator_index + 1], ast, scope);
                }
            } else {
                if (operator != OPERATOR_ASSIGN) {
                    PRINT_ERROR(E_EXPECTED_ASSIGNMENT_OPERATOR_PURE, node.start);
                }
                // push the values to the stack
                for (int i = operator_index; i < node.body.count; i++) {
                    compileNode(vm, ci, node.body.nodes[i], ast, scope);
                }
            }

            
            for (int i = operator_index - 1; i >= 0; i--) {
                Node left = node.body.nodes[i];

                if (left.type == NODE_IDENTIFIER) {
                    if (inScope(scope, ast->tokens.tokens[left.start].carry)) {
                        size_t index = getScopeIndex(scope, ast->tokens.tokens[left.start].carry);
                        OpCode op = operator == OPERATOR_INCREMENT ? OP_INCREMENT_FAST : operator == OPERATOR_DECREMENT ? OP_DECREMENT_FAST : OP_STORE_FAST;
                        writeInstruction(ci, left.start, op, DOSATO_SPLIT_SHORT(index));
                    } else {
                        OpCode op = operator == OPERATOR_INCREMENT ? OP_INCREMENT : operator == OPERATOR_DECREMENT ? OP_DECREMENT : OP_STORE;
                        writeInstruction(ci, left.start, op, DOSATO_SPLIT_SHORT(ast->tokens.tokens[left.start].carry));
                    }
                } else if (left.type == NODE_SUBSCRIPT_EXPRESSION) {
                    if (left.body.nodes[0].type != NODE_IDENTIFIER) {
                        compileNode(vm, ci, left.body.nodes[0], ast, scope); // the left side of the subscript (the object)
                    } else {
                        if (inScope(scope, ast->tokens.tokens[left.body.nodes[0].start].carry)) {
                            writeInstruction(ci, left.body.nodes[0].start, OP_LOAD_FAST, DOSATO_SPLIT_SHORT(getScopeIndex(scope, ast->tokens.tokens[left.body.nodes[0].start].carry)));
                        } else {
                            writeInstruction(ci, left.body.nodes[0].start, OP_LOAD, DOSATO_SPLIT_SHORT(ast->tokens.tokens[left.body.nodes[0].start].carry));
                        }
                    }
                    if (ast->tokens.tokens[left.body.nodes[1].start].carry == OPERATOR_ARROW && left.body.nodes[2].type == NODE_IDENTIFIER && ast->tokens.tokens[left.body.nodes[1].start + 1].type == TOKEN_IDENTIFIER) {
                        // add identifier as a constant string
                        char* val = getTokenString(ast->tokens.tokens[left.body.nodes[2].start]);
                        int id = -1;
                        if (hasName(&vm->constants_map, val)) {
                            id = getName(&vm->constants_map, val);
                            free(val);
                        } else {
                            id = addName(&vm->constants_map, val);
                            write_ValueArray(&vm->constants, BUILD_STRING(val, false));
                        }
                        writeInstruction(ci, left.body.nodes[2].start, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(id));
                    } else {
                        compileNode(vm, ci, left.body.nodes[2], ast, scope);
                    }

                    OperatorType operator_inner = ast->tokens.tokens[left.body.nodes[1].start].carry; // operator for the subscript

                    OpCode op = operator_inner == OPERATOR_HASH ? 
                        (operator == OPERATOR_INCREMENT ? OP_INCREMENT_SUBSCR : operator == OPERATOR_DECREMENT ? OP_DECREMENT_SUBSCR : OP_STORE_SUBSCR) 
                        : (operator == OPERATOR_INCREMENT ? OP_INCREMENT_OBJ : operator == OPERATOR_DECREMENT ? OP_DECREMENT_OBJ : OP_STORE_OBJ);
                        
                    writeByteCode(ci, op, left.start);
                }

                if ((operator != OPERATOR_ASSIGN && operator != OPERATOR_INCREMENT && operator != OPERATOR_DECREMENT) || tuple) {
                    writeByteCode(ci, OP_POP, node.start);
                }
            }

            if (operator == OPERATOR_ASSIGN && operator != OPERATOR_INCREMENT && operator != OPERATOR_DECREMENT && !tuple) {
                writeByteCode(ci, OP_POP, node.start);
            }
            
            break;
        }

        case NODE_SUBSCRIPT_EXPRESSION: {
            if (node.body.nodes[0].type != NODE_IDENTIFIER) {
                compileNode(vm, ci, node.body.nodes[0], ast, scope); // the left side of the subscript (the object)
            } else {
                if (inScope(scope, ast->tokens.tokens[node.body.nodes[0].start].carry)) {
                    writeInstruction(ci, node.body.nodes[0].start, OP_LOAD_FAST, DOSATO_SPLIT_SHORT(getScopeIndex(scope, ast->tokens.tokens[node.body.nodes[0].start].carry)));
                } else {
                    writeInstruction(ci, node.body.nodes[0].start, OP_LOAD, DOSATO_SPLIT_SHORT(ast->tokens.tokens[node.body.nodes[0].start].carry));
                }
            }
            
            if (node.body.count != 3) {
                PRINT_ERROR(E_INVALID_EXPRESSION, node.start);
            }

            OperatorType operator = ast->tokens.tokens[node.body.nodes[1].start].carry;

            if (ast->tokens.tokens[node.body.nodes[1].start].carry != OPERATOR_HASH && ast->tokens.tokens[node.body.nodes[1].start].carry != OPERATOR_ARROW) {
                PRINT_ERROR(E_EXPECTED_HASH_OPERATOR, node.body.nodes[1].start);
            }

            if ((ast->tokens.tokens[node.body.nodes[1].start].carry == OPERATOR_ARROW || ast->tokens.tokens[node.body.nodes[1].start].carry == OPERATOR_NULL_COALESCE_ACCESS) && node.body.nodes[2].type == NODE_IDENTIFIER && ast->tokens.tokens[node.body.nodes[1].start + 1].type == TOKEN_IDENTIFIER) {
                // add identifier as a constant string
                char* val = getTokenString(ast->tokens.tokens[node.body.nodes[2].start]);
                int id = -1;
                if (hasName(&vm->constants_map, val)) {
                    id = getName(&vm->constants_map, val);
                    free(val);
                } else {
                    id = addName(&vm->constants_map, val);
                    write_ValueArray(&vm->constants, BUILD_STRING(val, false));
                }
                writeInstruction(ci, node.body.nodes[2].start, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(id));
            } else {
                compileNode(vm, ci, node.body.nodes[2], ast, scope);
            }

            writeByteCode(ci, operator == OPERATOR_HASH ? OP_GETLIST : OP_GETOBJECT, node.body.nodes[1].start);
            break;
        }

        case NODE_FUNCTION_CALL: {
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

            if ((ast->tokens.tokens[node.body.nodes[1].start].carry == OPERATOR_ARROW || ast->tokens.tokens[node.body.nodes[1].start].carry == OPERATOR_NULL_COALESCE_ACCESS) && node.body.nodes[2].type == NODE_IDENTIFIER && ast->tokens.tokens[node.body.nodes[1].start + 1].type == TOKEN_IDENTIFIER) {
                // add identifier as a constant string
                char* val = getTokenString(ast->tokens.tokens[node.body.nodes[2].start]);
                int id = -1;
                if (hasName(&vm->constants_map, val)) {
                    id = getName(&vm->constants_map, val);
                    free(val);
                } else {
                    id = addName(&vm->constants_map, val);
                    write_ValueArray(&vm->constants, BUILD_STRING(val, false));
                }
                writeInstruction(ci, node.body.nodes[2].start, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(id));
            } else {
                compileNode(vm, ci, node.body.nodes[2], ast, scope);
            }

            int res = writeOperatorInstruction(ci, ast->tokens.tokens[node.body.nodes[1].start].carry, node.body.nodes[1].start);
            if (res == -1) {
                PRINT_ERROR(E_NON_BINARY_OPERATOR, node.body.nodes[1].start);
            }

            break;
        }

        case NODE_UNARY_EXPRESSION: {
            compileNode(vm, ci, node.body.nodes[1], ast, scope);
            int res = writeUnaryInstruction(ci, ast->tokens.tokens[node.body.nodes[0].start].carry, node.body.nodes[0].start);
            if (res == -1) {
                PRINT_ERROR(E_NON_UNARY_OPERATOR, node.body.nodes[0].start);
            }
            break;
        }

        case NODE_TYPE_CAST: {
            compileNode(vm, ci, node.body.nodes[1], ast, scope);
            writeInstruction(ci, node.start, OP_TYPE_CAST, ast->tokens.tokens[node.body.nodes[0].start].carry);
            break;
        }

        case NODE_TERNARY_EXPRESSION: {
            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            writeInstruction(ci, node.start, OP_TYPE_CAST, TYPE_BOOL);
            writeInstruction(ci, node.start, OP_JUMP_IF_FALSE, DOSATO_SPLIT_SHORT(0));
            int jump_index = ci->count - getOffset(OP_JUMP_IF_FALSE);

            compileNode(vm, ci, node.body.nodes[1], ast, scope);
            writeInstruction(ci, node.start, OP_JUMP, DOSATO_SPLIT_SHORT(0));
            int jump_index_end = ci->count - getOffset(OP_JUMP);

            // set jump false location
            ci->code[jump_index + 1] = ci->count & 0xFF;
            ci->code[jump_index + 2] = ci->count >> 8;

            compileNode(vm, ci, node.body.nodes[2], ast, scope);
            
            // set the final jump location
            ci->code[jump_index_end + 1] = ci->count & 0xFF;
            ci->code[jump_index_end + 2] = ci->count >> 8;
            
            break;
        }
        
        case NODE_STRING_LITERAL:
        case NODE_NUMBER_LITERAL: {
            writeInstruction(ci, node.start, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(ast->tokens.tokens[node.start].carry));
            break;
        }

        case NODE_TRUE: {
            writeByteCode(ci, OP_PUSH_TRUE, node.start);
            break;
        }

        case NODE_FALSE: {
            writeByteCode(ci, OP_PUSH_FALSE, node.start);
            break;
        }

        case NODE_NULL_KEYWORD: {
            writeByteCode(ci, OP_PUSH_NULL, node.start);
            break;
        }

        case NODE_INFINITY_KEYWORD: {
            writeByteCode(ci, OP_PUSH_INFINITY, node.start);
            break;
        }

        case NODE_NAN_KEYWORD: {
            writeByteCode(ci, OP_PUSH_NAN, node.start);
            break;
        }

        case NODE_TEMPLATE_LITERAL: {
            // the sum of all the parts
            for (int i = 0; i < node.body.count; i++) {
                if (node.body.nodes[i].type == NODE_TEMPLATE_STRING_PART) {
                    // add constant to the constants pool
                    char* val = getTokenString(ast->tokens.tokens[node.body.nodes[i].start]);
                    if (i == 0) {
                        char* new_val = malloc(strlen(val));
                        strcpy(new_val, val + 1); // remove the first quote
                        free(val);
                        val = new_val;
                    }

                    if (i == node.body.count - 1) {
                        val[strlen(val) - 1] = '\0'; // remove the last quote
                    }

                    int id = -1;
                    if (hasName(&vm->constants_map, val)) {
                        id = getName(&vm->constants_map, val);
                        free(val);
                    } else {
                        id = addName(&vm->constants_map, val);
                        write_ValueArray(&vm->constants, BUILD_STRING(val, false));
                    }

                    writeInstruction(ci, node.body.nodes[i].start, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(id));
                } else {
                    compileNode(vm, ci, node.body.nodes[i], ast, scope);
                }
            }
            for (int i = 0; i < node.body.count - 1; i++) {
                writeByteCode(ci, OP_BINARY_ADD, node.start);
            }
            break;
        }

        case NODE_IDENTIFIER: {
            if (ast->tokens.tokens[node.start].carry == 0) {
                writeInstruction(ci, node.start, OP_LOAD_UNDERSCORE); // load the underscore variable
                break;
            }

            if (scope == NULL) { // global scope
                writeInstruction(ci, node.start, OP_LOAD, DOSATO_SPLIT_SHORT(ast->tokens.tokens[node.start].carry)); // load the global variable
            } else {
                if (!inScope(scope, ast->tokens.tokens[node.start].carry)) {
                    writeInstruction(ci, node.start, OP_LOAD, DOSATO_SPLIT_SHORT(ast->tokens.tokens[node.start].carry)); // load the global variable
                    break;
                }
                size_t index = getScopeIndex(scope, ast->tokens.tokens[node.start].carry);
                if (index == -1) {
                    PRINT_ERROR(E_UNDEFINED_VARIABLE, node.start);
                }
                writeInstruction(ci, node.start, OP_LOAD_FAST, DOSATO_SPLIT_SHORT(index)); // load the local variable
            }
            break;
        }

        case NODE_ARRAY_EXPRESSION: {
            for (int i = 0; i < node.body.count; i++) {
                compileNode(vm, ci, node.body.nodes[i], ast, scope);
            }
            writeInstruction(ci, node.start, OP_BUILD_LIST, DOSATO_SPLIT_SHORT(node.body.count)); // cast to the correct type
            break;
        }

        case NODE_OBJECT_EXPRESSION: {
            for (int i = 0; i < node.body.count; i++) {
                compileNode(vm, ci, node.body.nodes[i], ast, scope);
            }
            writeInstruction(ci, node.start, OP_BUILD_OBJECT, DOSATO_SPLIT_SHORT(node.body.count)); // cast to the correct type
            break;
        }

        case NODE_OBJECT_ENTRY: {
            if (node.body.nodes[0].type == NODE_IDENTIFIER && ast->tokens.tokens[node.start].type == TOKEN_IDENTIFIER) {
                // add identifier as a constant string
                char* val = getTokenString(ast->tokens.tokens[node.body.nodes[0].start]);
                int id = -1;
                if (hasName(&vm->constants_map, val)) {
                    id = getName(&vm->constants_map, val);
                    free(val);
                } else {
                    id = addName(&vm->constants_map, val);
                    write_ValueArray(&vm->constants, BUILD_STRING(val, false));
                }
                writeInstruction(ci, node.body.nodes[0].start, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(id));
            } else {
                compileNode(vm, ci, node.body.nodes[0], ast, scope);
            }
            compileNode(vm, ci, node.body.nodes[1], ast, scope);
            break;
        }

        case NODE_LAMBDA_EXPRESSION: {
            DataType type = ast->tokens.tokens[node.body.nodes[0].start].carry;

            char* name = COPY_STRING("lambda");
            size_t name_index = -1;

            CodeInstance* instance = malloc(sizeof(CodeInstance));
            initCodeInstance(instance);
            instance->ast = ast;
            ScopeData* new_scope = malloc(sizeof(ScopeData));
            initScopeData(new_scope);

            int arity = node.body.nodes[1].body.count;
            int* hash_map = malloc(sizeof(int) * arity);
            for (int i = 0; i < arity; i++) {
                Node arg = node.body.nodes[1].body.nodes[i];
                int carry = ast->tokens.tokens[arg.body.nodes[arg.body.count - 1].start].carry;
                for (int j = 0; j < i; j++) {
                    if (hash_map[j] == carry) {
                        PRINT_ERROR(E_ALREADY_DEFINED_VARIABLE, arg.body.nodes[arg.body.count - 1].start);
                    }
                }
                hash_map[i] = carry;
                pushScopeData(new_scope, carry);
            }
            free(hash_map);
            new_scope->return_type = type;

            compileNode(vm, instance, node.body.nodes[2], ast, new_scope);

            size_t* capture_indexs = malloc(0);
            int capture_index_count = 0;

            // check for each LOAD, if it is a local variable, change it to LOAD_FAST and capture the index
            for (int i = 0; i < instance->count; i += getOffset(instance->code[i])) {
                if (instance->code[i] == OP_LOAD) {
                    size_t index = DOSATO_GET_ADDRESS_SHORT(instance->code, i + 1);
                    if (inScope(scope, index)) {
                        instance->code[i] = OP_TEMP; // temporary change to TEMP
                        instance->code[i + 1] = (capture_index_count + arity) & 0xFF;
                        instance->code[i + 2] = (capture_index_count + arity) >> 8;

                        capture_indexs = realloc(capture_indexs, sizeof(size_t) * (capture_index_count + 1));
                        int variable_index = getScopeIndex(scope, index);
                        capture_indexs[capture_index_count++] = variable_index;
                    }
                }
            }


            // each OP_LOAD_FAST must be offset by the capture_index_count and OP_TEMP must be changed to OP_LOAD_FAST
            for (int i = 0; i < instance->count; i += getOffset(instance->code[i])) {
                if (instance->code[i] == OP_LOAD_FAST || instance->code[i] == OP_STORE_FAST || instance->code[i] == OP_INCREMENT_FAST || instance->code[i] == OP_DECREMENT_FAST) {
                    size_t index = DOSATO_GET_ADDRESS_SHORT(instance->code, i + 1);
                    if (index < arity) {
                        continue;
                    }
                    instance->code[i + 1] = (index + capture_index_count) & 0xFF;
                    instance->code[i + 2] = (index + capture_index_count) >> 8;
                } else if (instance->code[i] == OP_RETURN) {
                    size_t index = DOSATO_GET_ADDRESS_SHORT(instance->code, i + 1);
                    instance->code[i + 1] = (index + capture_index_count) & 0xFF;
                    instance->code[i + 2] = (index + capture_index_count) >> 8;
                } else if (instance->code[i] == OP_TEMP) {
                    instance->code[i] = OP_LOAD_FAST; // change back without changing the index
                }
            }


            freeScopeData(new_scope);
            free(new_scope);

            for (int i = 0; i < arity + capture_index_count; i++) {
                writeByteCode(instance, OP_POP, node.end);
            }
            writeByteCode(instance, OP_END_FUNC, node.body.nodes[2].end);

            size_t* name_indexs = malloc(sizeof(size_t) * arity); 
            DataType* types = malloc(sizeof(DataType) * arity);
            for (int i = 0; i < arity; i++) {
                NodeList list = node.body.nodes[1].body.nodes[i].body;
                name_indexs[i] = ast->tokens.tokens[list.nodes[list.count - 1].start].carry;
                types[i] = list.count == 1 ? TYPE_VAR : ast->tokens.tokens[list.nodes[0].start].carry;
            }

            Function func;
            init_Function(&func);
            func.name = name;
            func.name_index = name_index;
            func.argv = name_indexs;
            func.argt = types;
            func.arity = node.body.nodes[1].body.count;
            func.instance = instance;
            func.return_type = type;

            func.captured_count = capture_index_count;
            func.captured_indices = capture_indexs;

            func.captured = NULL;

            write_FunctionList(&vm->functions, func);

            writeInstruction(ci, node.start, OP_LOAD_LAMBDA, DOSATO_SPLIT_SHORT(vm->functions.count - 1));

            break;
        }

        case NODE_IF_BODY: {
            // compile the condition
            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            writeInstruction(ci, node.start, OP_TYPE_CAST, TYPE_BOOL); // cast to the correct type
            writeInstruction(ci, node.start, OP_JUMP_IF_FALSE, DOSATO_SPLIT_SHORT(0)); // jump to the else block if the condition is false
            int jump_index = ci->count - getOffset(OP_JUMP_IF_FALSE); // index of the jump instruction

            // compile the body
            compileNode(vm, ci, node.body.nodes[1], ast, scope);

            // if there is an else block, jump to the end of the if block
            if (node.body.count == 3) {
                writeInstruction(ci, node.start, OP_JUMP, DOSATO_SPLIT_SHORT(0));
                int jump_index_end = ci->count - getOffset(OP_JUMP);
                // update the jump instruction
                ci->code[jump_index + 1] = ci->count & 0xFF;
                ci->code[jump_index + 2] = ci->count >> 8;
                // compile the else block
                compileNode(vm, ci, node.body.nodes[2], ast, scope);
                // update the jump instruction
                ci->code[jump_index_end + 1] = ci->count & 0xFF;
                ci->code[jump_index_end + 2] = ci->count >> 8;
            } else {
                // update the jump instruction
                ci->code[jump_index + 1] = ci->count & 0xFF;
                ci->code[jump_index + 2] = ci->count >> 8;
            }
            
            break;
        }

        case NODE_WHEN_BODY: {
            // compile the condition
            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            writeInstruction(ci, node.start, OP_TYPE_CAST, TYPE_BOOL); // cast to the correct type
            writeInstruction(ci, node.start, OP_JUMP_IF_FALSE, DOSATO_SPLIT_SHORT(0)); // jump to the end of the when block if the condition is false
            int jump_index = ci->count - getOffset(OP_JUMP_IF_FALSE); // index of the jump instruction
            // compile the body
            for (int i = 1; i < node.body.count; i++) {
                compileNode(vm, ci, node.body.nodes[i], ast, scope);
            }
            // manually edit the jump instruction
            ci->code[jump_index + 1] = ci->count & 0xFF;
            ci->code[jump_index + 2] = ci->count >> 8;
            return jump_index; // return the index of the jump instruction, so the ELSE_BODY can update it
        }

        case NODE_WHILE_BODY: {
            // compile the condition
            int condition_start = ci->count;
            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            writeInstruction(ci, node.start, OP_TYPE_CAST, TYPE_BOOL); // cast to the correct type
            writeInstruction(ci, node.start, OP_JUMP_IF_FALSE, DOSATO_SPLIT_SHORT(0)); // jump to the end of the while block if the condition is false
            int jump_index = ci->count - getOffset(OP_JUMP_IF_FALSE); // index of the jump instruction

            bool is_local = scope != NULL;

            // store the loop location data
            write_LocationList(&ci->loop_jump_locations, condition_start, is_local ? scope->locals_count : 0); // this is for the CONTINUE instruction, continue reevaluates the condition
            write_LocationList(&ci->loop_jump_locations, jump_index, is_local ? scope->locals_count : 0); // this is for the BREAK instruction, break jumps to the end of the while block without reevaluating the condition
            
            // compile the body
            for (int i = 1; i < node.body.count; i++) {
                compileNode(vm, ci, node.body.nodes[i], ast, scope);
            }
            // copy the condition
            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            writeInstruction(ci, node.start, OP_TYPE_CAST, TYPE_BOOL); // cast to the correct type
            writeInstruction(ci, node.start, OP_JUMP_IF_TRUE, DOSATO_SPLIT_SHORT(jump_index + getOffset(OP_JUMP_IF_FALSE))); // jump to the start of the while block if the condition is true


            // manually edit the jump instruction
            ci->code[jump_index + 1] = ci->count & 0xFF;
            ci->code[jump_index + 2] = ci->count >> 8;

            // pop the loop location data
            ci->loop_jump_locations.count -= 2;
            break;
        }

        case NODE_FOR_BODY: {
            // push the list to the stack
            compileNode(vm, ci, node.body.nodes[0], ast, scope);
            // push the index to the stack
            writeByteCode(ci, OP_PUSH_MINUS_ONE, node.start);

            bool is_local = scope != NULL;

            if (is_local) {
                pushScopeData(scope, -1);
                pushScopeData(scope, -1);
            }

            writeInstruction(ci, node.start, OP_FOR_ITER, DOSATO_SPLIT_SHORT(0)); // jump to the end of the for block if the list is empty
            int jump_index = ci->count - getOffset(OP_FOR_ITER); // index of the jump instruction

            // store the loop location data
            write_LocationList(&ci->loop_jump_locations, jump_index, is_local ? scope->locals_count : 2);

            // store to iterator
            if (ast->tokens.tokens[node.body.nodes[1].start].carry != 0) {
                if (inScope(scope, ast->tokens.tokens[node.body.nodes[1].start].carry)) {
                    writeInstruction(ci, node.body.nodes[1].start, OP_STORE_FAST, DOSATO_SPLIT_SHORT(getScopeIndex(scope, ast->tokens.tokens[node.body.nodes[1].start].carry)));
                } else {
                    writeInstruction(ci, node.body.nodes[1].start, OP_STORE, DOSATO_SPLIT_SHORT(ast->tokens.tokens[node.body.nodes[1].start].carry));
                }
                writeByteCode(ci, OP_POP, node.body.nodes[1].end);
            } else {
                // if underscore, don't store the value
                ci->code[jump_index] = OP_FOR_DISCARD; // change the jump instruction to discard the value
            }

            // if global scope, the 2 locals must be added to the scope
            if (!is_local) {
                ScopeData new_scope;
                initScopeData(&new_scope);
                scope = &new_scope;
                pushScopeData(scope, -1);
                pushScopeData(scope, -1);
            }
            // compile the body
            for (int i = 2; i < node.body.count; i++) {
                compileNode(vm, ci, node.body.nodes[i], ast, scope);
            }
            // back to global scope
            if (!is_local) {
                freeScopeData(scope);
                scope = NULL;
            }

            // jump back to the start of the for block
            writeInstruction(ci, node.start, OP_JUMP, DOSATO_SPLIT_SHORT(jump_index));
            // manually edit the jump instruction
            ci->code[jump_index + 1] = ci->count & 0xFF;
            ci->code[jump_index + 2] = ci->count >> 8;

            if (is_local) {
                scope->locals_count -= 2;
            }

            // pop the loop location data
            ci->loop_jump_locations.count--;
            break;
        }
        
        case NODE_MASTER_SWITCH_BODY: {
            // compile the condition
            compileNode(vm, ci, node.body.nodes[0], ast, scope);

            // for each case, compile a jump instruction
            Node switch_body = node.body.nodes[1];
            int default_index = -1;
            int** jump_locations = malloc(sizeof(int*) * (switch_body.body.count));
            for (int i = 0; i < switch_body.body.count; i++) {
                // compile the conditions
                Node case_node = switch_body.body.nodes[i];
                jump_locations[i] = malloc(sizeof(int) * (case_node.body.count - 1));
                for (int j = 0; j < case_node.body.count - 1; j++) {
                    if (case_node.body.nodes[j].type == NODE_OTHER) {
                        default_index = default_index == -1 ? i : default_index;
                        jump_locations[i][j] = -1;
                    } else {
                        compileNode(vm, ci, case_node.body.nodes[j], ast, scope);
                        writeInstruction(ci, case_node.start, OP_JUMP_IF_MATCH, DOSATO_SPLIT_SHORT(0)); // jump to the next case if the condition is false
                        jump_locations[i][j] = ci->count - getOffset(OP_JUMP_IF_MATCH); // index of the jump instruction
                    }
                }
            }

            int default_jump_index = -1;
            if (default_index != -1) {
                // if theres a default case, write a jump instruction for it
                writeByteCode(ci, OP_POP, node.start);
                writeInstruction(ci, node.start, OP_JUMP, DOSATO_SPLIT_SHORT(0));
                default_jump_index = ci->count - getOffset(OP_JUMP); // index of the jump instruction
            }

            // jump to end of the switch block if no case is matched
            writeByteCode(ci, OP_POP, node.start);
            writeInstruction(ci, node.start, OP_JUMP, DOSATO_SPLIT_SHORT(0));
            int jump_end_index = ci->count - getOffset(OP_JUMP); // index of the jump instruction

            bool is_local = scope != NULL;
            // store the loop location data
            write_LocationList(&ci->loop_jump_locations, jump_end_index, is_local ? scope->locals_count : 0);

            // compile the bodies
            for (int i = 0; i < switch_body.body.count; i++) {
                Node case_node = switch_body.body.nodes[i];
                // upgrade the jump instructions
                for (int j = 0; j < case_node.body.count - 1; j++) {
                    if (jump_locations[i][j] != -1) {
                        ci->code[jump_locations[i][j] + 1] = ci->count & 0xFF;
                        ci->code[jump_locations[i][j] + 2] = ci->count >> 8;
                    }
                }

                // if the case is the default case, update the default jump instruction
                if (i == default_index) {
                    ci->code[default_jump_index + 1] = ci->count & 0xFF;
                    ci->code[default_jump_index + 2] = ci->count >> 8;
                }

                // last node is the body
                compileNode(vm, ci, case_node.body.nodes[case_node.body.count - 1], ast, scope);
            }

            // pop the loop location data
            ci->loop_jump_locations.count--;

            // manually edit the jump instruction
            ci->code[jump_end_index + 1] = ci->count & 0xFF;
            ci->code[jump_end_index + 2] = ci->count >> 8;  

            // free the jump locations
            for (int i = 0; i < switch_body.body.count; i++) {
                free(jump_locations[i]);
            }
            free(jump_locations);

            break;
        }

        
        case NODE_MASTER_BREAK_BODY:
        case NODE_MASTER_CONTINUE_BODY: {
            if (ci->loop_jump_locations.count == 0) {
                PRINT_ERROR(type == NODE_MASTER_BREAK_BODY ? E_BREAK_OUTSIDE_LOOP : E_CONTINUE_OUTSIDE_LOOP, node.start - 1);
            }
            size_t top_jump_index = ci->loop_jump_locations.locations[ci->loop_jump_locations.count - 1];
            if (ci->code[top_jump_index] == OP_JUMP_IF_FALSE && type == NODE_MASTER_CONTINUE_BODY) {
                top_jump_index = ci->loop_jump_locations.locations[ci->loop_jump_locations.count - 2]; // get the condition start index instead of the jump index
            }

            // top_stack_amount is the amount of locals in the loop, and the same for both the break and continue instruction
            size_t top_stack_amount = ci->loop_jump_locations.stack_count[ci->loop_jump_locations.count - 1];
            bool is_local = scope != NULL;
            writeInstruction(ci, node.start, type == NODE_MASTER_BREAK_BODY ? OP_BREAK : OP_CONTINUE, DOSATO_SPLIT_SHORT(top_jump_index), DOSATO_SPLIT_SHORT((is_local ? scope->locals_count - top_stack_amount : 0)));
            break;
        }

        case NODE_CATCH_BODY: {
            // compile the body
            writeInstruction(ci, node.start, OP_JUMP_IF_EXCEPTION, DOSATO_SPLIT_SHORT(0)); // jump to the end of the catch block if there is no exception
            int jump_index = ci->count - getOffset(OP_JUMP_IF_EXCEPTION); // index of the jump instruction
            for (int i = 1; i < node.body.count; i++) {
                compileNode(vm, ci, node.body.nodes[i], ast, scope);
            }

            // jump to the end of the catch block
            writeInstruction(ci, node.start, OP_JUMP, DOSATO_SPLIT_SHORT(0));
            int jump_end_index = ci->count - getOffset(OP_JUMP); // index of the jump instruction

            // manually edit the jump instruction
            ci->code[jump_index + 1] = ci->count & 0xFF;
            ci->code[jump_index + 2] = ci->count >> 8;

            // catch statement
            compileNode(vm, ci, node.body.nodes[0], ast, scope);

            // jump to the end of the catch block
            writeInstruction(ci, node.start, OP_JUMP, DOSATO_SPLIT_SHORT(0));
            int jump_end_index2 = ci->count - getOffset(OP_JUMP); // index of the jump instruction

            // manually edit the jump instruction
            ci->code[jump_end_index + 1] = ci->count & 0xFF;
            ci->code[jump_end_index + 2] = ci->count >> 8;

            writeByteCode(ci, OP_CLEAR_EXCEPTION, node.start);

            // manually edit the jump instruction
            ci->code[jump_end_index2 + 1] = ci->count & 0xFF;
            ci->code[jump_end_index2 + 2] = ci->count >> 8;
            break;
        }
        
    }


    // don't know why, but if the count is 0, the program will crash, so add a NOP instruction if the count is 0
    if (ci->count == 0) {
        writeByteCode(ci, OP_NOP, 0);
    }

    return 0;
}

#undef PRINT_ERROR

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
        case OPERATOR_ROOT_ASSIGN: {
            writeByteCode(ci, OP_BINARY_ROOT_REVERSE, token_index);
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
        case OPERATOR_NULL_COALESCE_ACCESS: {
            writeByteCode(ci, OP_GETOBJECT_SAFE, token_index);
            break;
        }

        case OPERATOR_NULL_COALESCE_ASSIGN:
        case OPERATOR_NULL_COALESCE: {
            writeByteCode(ci, OP_BINARY_NULL_COALESCE, token_index);
            break;
        }

        case OPERATOR_RANGE_UP: {
            writeByteCode(ci, OP_BINARY_RANGE_UP, token_index);
            break;
        }
        case OPERATOR_RANGE_DOWN: {
            writeByteCode(ci, OP_BINARY_RANGE_DOWN, token_index);
            break;
        }
        case OPERATOR_RANGE_UP_INCLUSIVE: {
            writeByteCode(ci, OP_BINARY_RANGE_UP_INCLUSIVE, token_index);
            break;
        }
        case OPERATOR_RANGE_DOWN_INCLUSIVE: {
            writeByteCode(ci, OP_BINARY_RANGE_DOWN_INCLUSIVE, token_index);
            break;
        }

        case OPERATOR_PIPE: {
            writeInstruction(ci, token_index, OP_CALL, 1);
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
        case OPERATOR_MULTIPLY: {
            writeByteCode(ci, OP_COPY, token_index);
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
    scope->is_class = false;
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