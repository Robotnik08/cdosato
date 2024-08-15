#include "../include/common.h"

#include "../include/compiler.h"

void compile(VirtualMachine* vm, AST ast) { 

    // Compile the AST into byte code
    compileNode(vm, ast.root, ast.tokens);

    writeByteCode(vm->instance, OP_STOP, 0);
}

void compileNode (VirtualMachine* vm, Node node, TokenList tokens) {
    switch (node.type) {
        case NODE_PROGRAM:
        case NODE_BLOCK: {
            for (int i = 0; i < node.body.count; i++) {
                compileNode(vm, node.body.nodes[i], tokens);
            }
            break;
        }

        case NODE_MASTER_MAKE:
        case NODE_MASTER_DO: {
            compileNode(vm, node.body.nodes[0], tokens);
            break;
        }

        case NODE_MASTER_DO_BODY: {
            compileNode(vm, node.body.nodes[0], tokens);
            break;
        }

        case NODE_MASTER_MAKE_BODY: {
            // push the value to the stack
            compileNode(vm, node.body.nodes[2], tokens);
            if (tokens.tokens[node.body.nodes[0].start].carry != TYPE_VAR)
                writeInstruction(vm->instance, node.start, OP_TYPE_CAST, tokens.tokens[node.body.nodes[0].start].carry); // cast to the correct type (if not var)
            
            writeInstruction(vm->instance, node.body.nodes[1].start, OP_DEFINE_FAST, DOSATO_SPLIT_SHORT(tokens.tokens[node.body.nodes[1].start].carry)); // store the value in the local variable
            
            break;
        }


        case NODE_FUNCTION_CALL: {
            // push arguments
            for (int i = 0; i < node.body.nodes[1].body.count; i++) {
                compileNode(vm, node.body.nodes[1].body.nodes[i], tokens);
            }
            writeByteCode(vm->instance, OP_PRINT, node.start); // for now, a function call is just a print
            break;
        }


        case NODE_EXPRESSION: {
            if (node.body.count == 1) {
                compileNode(vm, node.body.nodes[0], tokens);
                break;
            }
            compileNode(vm, node.body.nodes[0], tokens);
            compileNode(vm, node.body.nodes[2], tokens);
            writeByteCode(vm->instance, OP_BINARY_ADD, node.start); // for now, an expression is just an add
            break;
        }
        
        case NODE_STRING_LITERAL:
        case NODE_NUMBER_LITERAL: {
            writeInstruction(vm->instance, node.start, OP_LOAD_CONSTANT, DOSATO_SPLIT_SHORT(tokens.tokens[node.start].carry)); // always load 1 for now
            break;
        }

        case NODE_INDENTIFIER: {
            writeInstruction(vm->instance, node.start, OP_LOAD_FAST, DOSATO_SPLIT_SHORT(tokens.tokens[node.start].carry)); // load the local variable
            break;
        }
    }
}