#include "../include/common.h"

#include "../include/parser.h"
#include "../include/node.h"
#include "../include/token.h"

#define ERROR(index, code) do { \
    printError(source, tokens.tokens[index].start - source, code); \
} while (0)

#define ENCASED_IN_BRACKETS(start, end, b_type) \
    (((start).carry == (end).carry) && \
     ((start).type == TOKEN_PARENTHESIS_OPEN) && \
     ((end).type == TOKEN_PARENTHESIS_CLOSED) && \
     CHECK_BRACKET_TYPE((start).carry, b_type))


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
                        write_NodeList(&root.body, parse(source, length, ext_start, i, tokens, NODE_WHEN_BODY + ext_type));
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
        case NODE_MASTER_DO_BODY: 
        case NODE_THEN_BODY: 
        case NODE_CATCH_BODY:
        case NODE_ELSE_BODY: {
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
            if (tokens.tokens[j - 1].carry == OPERATOR_INCREMENT || tokens.tokens[j - 1].carry == OPERATOR_DECREMENT) {
                if (j != end) {
                    ERROR(j, E_UNEXPECTED_TOKEN);
                }
                break;
            }
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


        case NODE_FUNCTION_DEFINITION_PARAMETERS: {
            if (start == end) {
                // no function parameters
                break;
            }
            // split on the commas
            int j = start;
            for (int i = start; i < end; i++) {
                if (tokens.tokens[i].type == TOKEN_OPERATOR && tokens.tokens[i].carry == OPERATOR_COMMA) {
                    write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_FUNCTION_DEFINITION_ARGUMENT));
                    j = i + 1;
                }
            }
            write_NodeList(&root.body, parse(source, length, j, end, tokens, NODE_FUNCTION_DEFINITION_ARGUMENT));

            break;
        }

        case NODE_FUNCTION_DEFINITION_ARGUMENT: {
            if (start == end) {
                ERROR(start, E_EXPECTED_TYPE_INDENTIFIER);
            }
            // type
            int i = start;
            while (tokens.tokens[i++].type == TOKEN_VAR_TYPE) {}
            i--;
            write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_TYPE));
            // identifier
            if (tokens.tokens[i].type != TOKEN_IDENTIFIER) {
                ERROR(i, E_EXPECTED_IDENTIFIER);
            }
            write_NodeList(&root.body, parse(source, length, i, i + 1, tokens, NODE_INDENTIFIER));
            if (i + 1 != end) {
                ERROR(i + 1, E_UNEXPECTED_TOKEN);
            }
            break;
        }

        case NODE_FUNCTION_CALL: {
            // get the function expression
            int i = start;
            for (i = start; i < end; i++) {
                if (tokens.tokens[i].type == TOKEN_PARENTHESIS_OPEN) {
                    break;
                }
            }
            if (i == end - 1) {
                ERROR(i, E_EXPECTED_BRACKET_ROUND);
            }
            if (i == start) {
                ERROR(start, E_EXPECTED_IDENTIFIER);
            }
            write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_EXPRESSION));
            // get the arguments
            if (tokens.tokens[i].type != TOKEN_PARENTHESIS_OPEN || !CHECK_BRACKET_TYPE(tokens.tokens[i].carry, BRACKET_ROUND)) {
                ERROR(i, E_EXPECTED_BRACKET_ROUND);
            }
            int j = getEndOfBlock(tokens, i);
            if (j == -1) {
                ERROR(i, E_MISSING_CLOSING_PARENTHESIS);
            }
            write_NodeList(&root.body, parse(source, length, i + 1, j, tokens, NODE_ARGUMENTS));
            if (j + 1 != end) {
                ERROR(j + 1, E_UNEXPECTED_TOKEN);
            }
            break;
        }
    	
        case NODE_ARRAY_EXPRESSION:
        case NODE_ARGUMENTS: {
            if (start == end) {
                // no arguments
                break;
            }
            // split on the commas
            int j = start;
            for (int i = start; i < end; i++) {
                SKIP_BLOCK(i);
                if (tokens.tokens[i].type == TOKEN_OPERATOR && tokens.tokens[i].carry == OPERATOR_COMMA) {
                    write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_EXPRESSION));
                    j = i + 1;
                }
            }
            write_NodeList(&root.body, parse(source, length, j, end, tokens, NODE_EXPRESSION));
            break;
        }

        case NODE_WHEN_BODY:
        case NODE_WHILE_BODY:
        case NODE_IF_BODY: {
            // expression
            write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_EXPRESSION));
            break;
        }

        case NODE_FOR_BODY: {
            // special type of FOR expression
            // get rid of parentheses
            int new_start = start;
            int new_end = end - 1;
            while (ENCASED_IN_BRACKETS(tokens.tokens[new_start], tokens.tokens[new_end], BRACKET_ROUND)) {
                new_start++;
                new_end--;
            }

            root.start = new_start;
            root.end = new_end;

            // get the first expression (everything before the => operator)
            int i = new_start;
            for (i = new_start; i < new_end; i++) {
                SKIP_BLOCK(i);
                if (tokens.tokens[i].type == TOKEN_OPERATOR && tokens.tokens[i].carry == OPERATOR_AS) {
                    break;
                }
            }
            if (i == new_start) {
                ERROR(i, E_EXPECTED_IDENTIFIER);
            }
            write_NodeList(&root.body, parse(source, length, new_start, i, tokens, NODE_EXPRESSION));
            // after the arrow must be an identifier
            if (tokens.tokens[i + 1].type != TOKEN_IDENTIFIER) {
                ERROR(i + 1, E_EXPECTED_IDENTIFIER);
            }
            write_NodeList(&root.body, parse(source, length, i + 1, i + 2, tokens, NODE_INDENTIFIER));
            if (i + 1 != new_end) {
                ERROR(i + 2, E_UNEXPECTED_TOKEN);
            }
            break;
        }

        case NODE_OBJECT_EXPRESSION: {
            if (start == end) {
                // empty object
                break;
            }

            // split on the commas
            int j = start;
            for (int i = start; i < end; i++) {
                SKIP_BLOCK(i);
                if (tokens.tokens[i].type == TOKEN_OPERATOR && tokens.tokens[i].carry == OPERATOR_COMMA) {
                    write_NodeList(&root.body, parse(source, length, j, i, tokens, NODE_OBJECT_ENTRY));
                    j = i + 1;
                }
            }
            write_NodeList(&root.body, parse(source, length, j, end, tokens, NODE_OBJECT_ENTRY));
            break;
        }

        case NODE_OBJECT_ENTRY: {
            // get the key
            if (tokens.tokens[start].type == TOKEN_IDENTIFIER) {
                write_NodeList(&root.body, parse(source, length, start, start + 1, tokens, NODE_INDENTIFIER));
            } else if (tokens.tokens[start].type == TOKEN_STRING) {
                write_NodeList(&root.body, parse(source, length, start, start + 1, tokens, NODE_STRING_LITERAL));
            } else {
                ERROR(start, E_EXPECTED_IDENTIFIER);
            }
            if (tokens.tokens[start + 1].type != TOKEN_OPERATOR || tokens.tokens[start + 1].carry != OPERATOR_COLON) {
                ERROR(start + 1, E_UNEXPECTED_TOKEN);
            }
            write_NodeList(&root.body, parse(source, length, start + 2, end, tokens, NODE_EXPRESSION));
            break;
        }


        case NODE_EXPRESSION: {
            int new_start = start;
            int new_end = end;
            while (ENCASED_IN_BRACKETS(tokens.tokens[new_start], tokens.tokens[new_end - 1], BRACKET_ROUND)) {
                new_start++;
                new_end--;
            }

            root.start = new_start;
            root.end = new_end;


            if (new_start == new_end) {
                ERROR(new_start, E_EMPTY_EXPRESSION);
            }

            if (new_start == new_end - 1) {
                // single expression
                if (tokens.tokens[new_start].type == TOKEN_IDENTIFIER) {
                    write_NodeList(&root.body, parse(source, length, new_start, new_end, tokens, NODE_INDENTIFIER));
                } else if (tokens.tokens[new_start].type == TOKEN_NUMBER) {
                    write_NodeList(&root.body, parse(source, length, new_start, new_end, tokens, NODE_NUMBER_LITERAL));
                } else if (tokens.tokens[new_start].type == TOKEN_STRING) {
                    write_NodeList(&root.body, parse(source, length, new_start, new_end, tokens, NODE_STRING_LITERAL));
                } else {
                    ERROR(new_start, E_UNEXPECTED_TOKEN);
                }
                break;
            }

            // split on the operators
            int j = new_start - 1;
            int highest = -1;
            int highest_index = -1;

            int precedence_values[] = OPERATOR_PRECEDENCE;

            bool func_call = false;
            for (int i = new_start; i < new_end; i++) {
                if (tokens.tokens[i].type == TOKEN_PARENTHESIS_OPEN && CHECK_BRACKET_TYPE(tokens.tokens[i].carry, BRACKET_ROUND)) {
                    if (j == i - 1) {
                        SKIP_BLOCK(i);
                        continue; // it's a normal bracket expression
                    }
                    int endofblock = getEndOfBlock(tokens, i);
                    if (endofblock == -1) {
                        ERROR(i, E_MISSING_CLOSING_PARENTHESIS);
                    }
                    if (1 >= highest) {
                        highest = 1;
                        highest_index = i;
                        func_call = true;
                    }
                }
                SKIP_BLOCK(i);
                if (tokens.tokens[i].type == TOKEN_OPERATOR) {
                    j = i;
                    if (precedence_values[tokens.tokens[i].carry] >= highest) {
                        highest = precedence_values[tokens.tokens[i].carry];
                        highest_index = i;
                        func_call = false;
                    }
                }
            }

            if (func_call) {
                write_NodeList(&root.body, parse(source, length, new_start, getEndOfBlock(tokens, highest_index) + 1, tokens, NODE_FUNCTION_CALL));
                break;
            }

            if (highest == -1) {
                // no operators

                if (ENCASED_IN_BRACKETS(tokens.tokens[new_start], tokens.tokens[new_end - 1], BRACKET_SQUARE)) {
                    write_NodeList(&root.body, parse(source, length, new_start + 1, new_end - 1, tokens, NODE_ARRAY_EXPRESSION));
                } else if (ENCASED_IN_BRACKETS(tokens.tokens[new_start], tokens.tokens[new_end - 1], BRACKET_CURLY)) {
                    write_NodeList(&root.body, parse(source, length, new_start + 1, new_end - 1, tokens, NODE_OBJECT_EXPRESSION));
                } else {
                    ERROR(new_start, E_UNEXPECTED_TOKEN);
                }
            } else {
                if (highest_index == new_start) {
                    // unary operator
                    write_NodeList(&root.body, parse(source, length, highest_index, end, tokens, NODE_UNARY_EXPRESSION));
                } else if (highest_index == new_end - 1) {
                    // error
                    ERROR(highest_index, E_UNEXPECTED_TOKEN);
                } else {
                    write_NodeList(&root.body, parse(source, length, new_start, highest_index, tokens, NODE_EXPRESSION));
                    write_NodeList(&root.body, parse(source, length, highest_index, highest_index + 1, tokens, NODE_OPERATOR));
                    write_NodeList(&root.body, parse(source, length, highest_index + 1, new_end, tokens, NODE_EXPRESSION));
                }
            }
            
            break;
        }

        case NODE_UNARY_EXPRESSION: {
            if (start == end) {
                ERROR(start, E_UNEXPECTED_TOKEN);
            }
            if (tokens.tokens[start].type != TOKEN_OPERATOR || !isUnaryOperator(tokens.tokens[start].carry)) {
                ERROR(start, E_NON_UNARY_OPERATOR);
            }
            
            write_NodeList(&root.body, parse(source, length, start, start + 1, tokens, NODE_OPERATOR));
            write_NodeList(&root.body, parse(source, length, start + 1, end, tokens, NODE_EXPRESSION));
            break;
        }


        default: {
            break; // do nothing
        }
    }

    return root;
}
#undef ERROR