#include "../include/common.h"

#include "../include/parser.h"
#include "../include/node.h"
#include "../include/token.h"

#define ERROR(index, code) do { \
    printError(source, tokens.tokens[index].start - source, code); \
} while (0)

#define SKIP_BLOCK(i) \
    do { \
        if (tokens.tokens[(i)].type == TOKEN_PARENTHESIS_OPEN) {  \
            int endofblock = getEndOfBlock(tokens, (i)); \
            if (endofblock == -1) { \
                ERROR((i), E_MISSING_CLOSING_PARENTHESIS); \
            } else { \
                (i) = endofblock; \
            } \
        } \
    } while (0)

Node parse (const char *source, size_t length, const int start, const int end, TokenList tokens, NodeType type) {
    Node root;
    root.start = start;
    root.end = end;
    root.type = type;
    init_NodeList(&root.body);

    switch (type) {
        case NODE_BLOCK:
        case NODE_PROGRAM: {
            for (int i = start; i < end; i++) {
                if (tokens.tokens[i].type == TOKEN_MASTER_KEYWORD) {
                    int endofline = getEndOfLine(tokens, i);

                    if (endofline == -1) {
                        ERROR(end - 1, E_MISSING_SEPERATOR);
                    }
                    if (endofline == -2) {
                        ERROR(i, E_MISSING_CLOSING_PARENTHESIS);
                    }

                    write_NodeList(&root.body, parse(source, length, i + 1, endofline, tokens, NODE_MASTER_DO + tokens.tokens[i].carry)); // NODE_MASTER + carry = the correct master node type
                    i = endofline;
                } else if (tokens.tokens[i].type != TOKEN_SEPARATOR) {
                    ERROR(i, E_EXPECTED_MASTER);
                }
            }
            break;
        }

        case NODE_MASTER_DO:
        case NODE_MASTER_MAKE:
        case NODE_MASTER_SET:
        case NODE_MASTER_DEFINE:
        case NODE_MASTER_INCLUDE:
        case NODE_MASTER_IMPORT:
        case NODE_MASTER_RETURN:
        case NODE_MASTER_BREAK:
        case NODE_MASTER_CONTINUE: {
            bool body_parsed = false;
            ExtensionKeywordType ext_type = EXT_NULL;
            int ext_start = start;
            for (int i = start; i < end; i++) {
                SKIP_BLOCK(i);
                if (tokens.tokens[i].type == TOKEN_EXT) {
                    if (ext_type == EXT_NULL) {
                        ext_type = tokens.tokens[i].carry;
                        ext_start = i + 1;
                        if (!body_parsed) {
                            write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_MASTER_DO_BODY + tokens.tokens[start - 1].carry));
                            body_parsed = true;
                        }
                    } else {
                        write_NodeList(&root.body, parse(source, length, ext_start, i - 1, tokens, NODE_WHEN_BODY + ext_type));
                        ext_type = tokens.tokens[i].carry;
                        ext_start = i + 1;
                    }
                }
            }
            if (!body_parsed) {
                write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_MASTER_DO_BODY + tokens.tokens[start - 1].carry));
            } else {
                write_NodeList(&root.body, parse(source, length, ext_start, end, tokens, NODE_WHEN_BODY + ext_type));
            }
            break;
        }

        case NODE_MASTER_DO_BODY: {
            // either a function call or a block
            if (tokens.tokens[start].type == TOKEN_PARENTHESIS_OPEN && CHECK_BRACKET_TYPE(tokens.tokens[start].carry, BRACKET_CURLY)) {
                int endofblock = getEndOfBlock(tokens, start);
                if (endofblock == -1) {
                    ERROR(start, E_MISSING_CLOSING_PARENTHESIS);
                }
                write_NodeList(&root.body, parse(source, length, start + 1, endofblock, tokens, NODE_BLOCK));
            } else if (tokens.tokens[start].type == TOKEN_IDENTIFIER) {
                write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_FUNCTION_CALL));
            }
            break;
        }

        case NODE_MASTER_MAKE_BODY: {
            // making a variable
            if (tokens.tokens[start].type == TOKEN_VAR_TYPE) {
                // parse the TYPE
                int i = start;
                while (tokens.tokens[i++].type == TOKEN_VAR_TYPE) {}
                i--;
                write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_TYPE));
                
                if (tokens.tokens[i].type != TOKEN_IDENTIFIER) {
                    ERROR(i, E_EXPECTED_IDENTIFIER);
                }
                write_NodeList(&root.body, parse(source, length, i, i + 1, tokens, NODE_INDENTIFIER));
                if (tokens.tokens[i + 1].type != TOKEN_OPERATOR || tokens.tokens[i + 1].carry != OPERATOR_ASSIGN) {
                    ERROR(i + 1, E_EXPECTED_ASSIGNMENT_OPERATOR_PURE);
                }
                write_NodeList(&root.body, parse(source, length, i + 2, end, tokens, NODE_EXPRESSION));
            } else {
                ERROR(start, E_EXPECTED_TYPE_INDENTIFIER);
            }
            break;
        }

        case NODE_MASTER_SET_BODY: {
            // setting a variable
            int j = start;
            // get the variable (everything before the first operator assigmnet operator)
            for (j = start; j < end; j++) {
                SKIP_BLOCK(j);
                if (tokens.tokens[j].type == TOKEN_OPERATOR && isAssignmentOperator(tokens.tokens[j].carry)) {
                    j++;
                    break;
                }
                if (j == end - 1) {
                    ERROR(j, E_EXPECTED_ASSIGNMENT_OPERATOR);
                }
            }
            if (j == start) {
                ERROR(start, E_EXPECTED_IDENTIFIER);
            }
            write_NodeList(&root.body, parse(source, length, start, j - 1, tokens, NODE_EXPRESSION));
            write_NodeList(&root.body, parse(source, length, j - 1, j, tokens, NODE_OPERATOR));
            write_NodeList(&root.body, parse(source, length, j, end, tokens, NODE_EXPRESSION));
            break;
        }

        case NODE_MASTER_DEFINE_BODY: {
            // defining a function
            
            // first must be a return type
            if (tokens.tokens[start].type == TOKEN_VAR_TYPE) {
                // parse the TYPE
                int i = start;
                while (tokens.tokens[i++].type == TOKEN_VAR_TYPE) {}
                i--;
                write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_TYPE));
                
                // next the function name
                if (tokens.tokens[i].type != TOKEN_IDENTIFIER) {
                    ERROR(i, E_EXPECTED_IDENTIFIER);
                }
                write_NodeList(&root.body, parse(source, length, i, i + 1, tokens, NODE_INDENTIFIER));

                // parameters
                if (tokens.tokens[i + 1].type != TOKEN_PARENTHESIS_OPEN || !CHECK_BRACKET_TYPE(tokens.tokens[i + 1].carry, BRACKET_ROUND)) {
                    ERROR(i + 1, E_EXPECTED_BRACKET_ROUND);
                }
                // get end of parameters
                int j = getEndOfBlock(tokens, i + 1);
                if (j == -1) {
                    ERROR(i + 1, E_MISSING_CLOSING_PARENTHESIS);
                }
                write_NodeList(&root.body, parse(source, length, i + 2, j, tokens, NODE_FUNCTION_DEFINITION_PARAMETERS));

                // function body
                if (tokens.tokens[j + 1].type != TOKEN_PARENTHESIS_OPEN || !CHECK_BRACKET_TYPE(tokens.tokens[j + 1].carry, BRACKET_CURLY)) {
                    ERROR(j + 1, E_EXPECTED_BRACKET_CURLY);
                }
                // get end of block
                int k = getEndOfBlock(tokens, j + 1);
                if (k == -1) {
                    ERROR(j + 1, E_MISSING_CLOSING_PARENTHESIS);
                }
                write_NodeList(&root.body, parse(source, length, j + 2, k, tokens, NODE_BLOCK));
                if (k + 1 != end) {
                    ERROR(k + 1, E_UNEXPECTED_TOKEN);
                }
            } else {
                ERROR(start, E_EXPECTED_TYPE_INDENTIFIER);
            }
            break;
        }

        case NODE_MASTER_INCLUDE_BODY: {
            // expression
            write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_EXPRESSION));
            break;
        }

        case NODE_MASTER_IMPORT_BODY: {
            // identifier
            if (tokens.tokens[start].type != TOKEN_IDENTIFIER) {
                ERROR(start, E_EXPECTED_IDENTIFIER);
            }
            if (start + 1 != end) {
                ERROR(start + 1, E_UNEXPECTED_TOKEN);
            }
            write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_INDENTIFIER));
            break;
        }

        case NODE_MASTER_RETURN_BODY: {
            // expression
            write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_EXPRESSION));
            break;
        }

        case NODE_MASTER_BREAK_BODY:
        case NODE_MASTER_CONTINUE_BODY: {
            if (start != end) {
                ERROR(start, E_UNEXPECTED_TOKEN);
            }
            break;
        }


        default: {
            break; // do nothing
        }
    }

    return root;
}
#undef ERROR