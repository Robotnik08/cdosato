#include "../include/common.h"

#include "../include/memory.h"
#include "../include/token.h"
#include "../include/node.h"

DOSATO_LIST_FUNC_GEN(NodeList, Node, nodes)

void destroyNodeList (NodeList* list) {
    for (size_t i = 0; i < list->count; i++) {
        destroyNodeList(&list->nodes[i].body);
    }
    free_NodeList(list);
}

static const char* NODE_NAMES[] = {
    "NODE_PROGRAM",

    "NODE_MASTER_DO",
    "NODE_MASTER_MAKE",
    "NODE_MASTER_SET",
    "NODE_MASTER_DEFINE",
    "NODE_MASTER_INCLUDE",
    "NODE_MASTER_IMPORT",
    "NODE_MASTER_RETURN",
    "NODE_MASTER_BREAK",
    "NODE_MASTER_CONTINUE",
    "NODE_MASTER_SWITCH",
    "NODE_MASTER_CONST",

    "NODE_MASTER_DO_BODY",
    "NODE_MASTER_MAKE_BODY",
    "NODE_MASTER_SET_BODY",
    "NODE_MASTER_DEFINE_BODY",
    "NODE_MASTER_INCLUDE_BODY",
    "NODE_MASTER_IMPORT_BODY",
    "NODE_MASTER_RETURN_BODY",
    "NODE_MASTER_BREAK_BODY",
    "NODE_MASTER_CONTINUE_BODY",
    "NODE_MASTER_SWITCH_BODY",
    "NODE_MASTER_CONST_BODY",

    "NODE_WHEN_BODY",
    "NODE_WHILE_BODY",
    "NODE_ELSE_BODY",
    "NODE_CATCH_BODY",
    "NODE_THEN_BODY",
    "NODE_FOR_BODY",
    "NODE_IF_BODY",

    "NODE_BLOCK",
    "NODE_EXPRESSION",
    "NODE_SUBSCRIPT_EXPRESSION",
    "NODE_TEMP_SUBSCRIPT_EXPRESSION",
    "NODE_UNARY_EXPRESSION",
    "NODE_ARRAY_EXPRESSION",
    "NODE_OBJECT_EXPRESSION",
    "NODE_TERNARY_EXPRESSION",
    "NODE_FUNCTION_CALL",
    "NODE_TYPE",
    "NODE_OPERATOR",
    "NODE_IDENTIFIER",
    "NODE_TRUE",
    "NODE_FALSE",
    "NODE_NULL_KEYWORD",
    "NODE_NUMBER_LITERAL",
    "NODE_STRING_LITERAL",
    "NODE_FUNCTION_DEFINITION_PARAMETERS",
    "NODE_FUNCTION_DEFINITION_ARGUMENT",
    "NODE_ARGUMENTS",
    "NODE_OBJECT_ENTRY",
    "NODE_TYPE_CAST",
    "NODE_SWITCH_BODY",
    "NODE_SWITCH_CASE",
    "NODE_OTHER",
    "NODE_TEMPLATE_LITERAL",
    "NODE_TEMPLATE_STRING_PART"
};

void printNode (Node node, int depth) {
    putchar(node.body.count == 0 ? '-' : '+');
    printf("Node: s:%d e:%d Type:%s\n", node.start, node.end - 1, NODE_NAMES[node.type]);
    for (size_t i = 0; i < node.body.count; i++) {
        for (int i = 0; i < depth; i++) {
            printf("  ");
        }
        printf("  ");
        printNode(node.body.nodes[i], depth + 1);
    }
}

int getEndOfLine (TokenList tokens, int start) {
    for (int i = start; i < tokens.count; i++) {
        if (tokens.tokens[i].type == TOKEN_PARENTHESIS_OPEN) {
            int end = getEndOfBlock(tokens, i);
            if (end == -1) {
                return -1; // invalid (missing closing parenthesis)
            }
            i = end;
        } else if (tokens.tokens[i].type == TOKEN_PARENTHESIS_CLOSED || tokens.tokens[i].type == TOKEN_MASTER_KEYWORD) {
            return i;
        }
    }
    return tokens.count;
}

int getEndOfBlock (TokenList tokens, int start) {
    if (tokens.tokens[start].type != TOKEN_PARENTHESIS_OPEN) {
        return -1; // invalid
    }
    int targetcarry = tokens.tokens[start].carry;
    for (int i = start + 1; i < tokens.count; i++) {
        if (tokens.tokens[i].type == TOKEN_PARENTHESIS_CLOSED) {
            if (tokens.tokens[i].carry == targetcarry) {
                return i;
            }
        }
    }
    return -1; // invalid
}

int getStartOfBlock (TokenList tokens, int start) {
    if (tokens.tokens[start].type != TOKEN_PARENTHESIS_CLOSED) {
        return -1; // invalid
    }
    int targetcarry = tokens.tokens[start].carry;
    for (int i = start - 1; i >= 0; i--) {
        if (tokens.tokens[i].type == TOKEN_PARENTHESIS_OPEN) {
            if (tokens.tokens[i].carry == targetcarry) {
                return i;
            }
        }
    }
    return -1; // invalid
}