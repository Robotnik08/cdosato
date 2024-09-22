#include "../include/common.h"

#include "../include/parser.h"
#include "../include/node.h"
#include "../include/token.h"

#define PRINT_ERROR(index, code) do { \
    printError(source, tokens.tokens[index].start - source, file_name, code); \
} while (0)

#define ENCASED_IN_BRACKETS(start, end, b_type) \
    (getEndOfBlock(tokens, (start)) == (end) && \
     CHECK_BRACKET_TYPE(tokens.tokens[(start)].carry, b_type))


#define SKIP_BLOCK(i) \
    do { \
        if (tokens.tokens[(i)].type == TOKEN_PARENTHESIS_OPEN) {  \
            int endofblock = getEndOfBlock(tokens, (i)); \
            if (endofblock == -1) { \
                PRINT_ERROR((i), E_MISSING_CLOSING_PARENTHESIS); \
            } else { \
                (i) = endofblock; \
            } \
        } \
    } while (0)

Node parse (const char *source, size_t length, const int start, const int end, TokenList tokens, NodeType type, const char* file_name) {
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
                        PRINT_ERROR(end - 1, E_MISSING_SEPERATOR);
                    }
                    if (endofline == -2) {
                        PRINT_ERROR(i, E_MISSING_CLOSING_PARENTHESIS);
                    }

                    write_NodeList(&root.body, parse(source, length, i + 1, endofline, tokens, NODE_MASTER_DO + tokens.tokens[i].carry, file_name)); // NODE_MASTER + carry = the correct master node type
                    i = endofline;
                } else if (tokens.tokens[i].type != TOKEN_SEPARATOR) {
                    PRINT_ERROR(i, E_EXPECTED_MASTER);
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
            NodeList temp_body; // used to temporarily store the body, so we can reorganize it later
            init_NodeList(&temp_body);
            for (int i = start; i < end; i++) {
                SKIP_BLOCK(i);
                if (tokens.tokens[i].type == TOKEN_EXT) {
                    if (ext_type == EXT_NULL) {
                        ext_type = tokens.tokens[i].carry;
                        ext_start = i + 1;
                        if (!body_parsed) {
                            write_NodeList(&temp_body, parse(source, length, start, i, tokens, NODE_MASTER_DO_BODY + tokens.tokens[start - 1].carry, file_name));
                            body_parsed = true;
                        }
                    } else if (ext_type == EXT_IF && tokens.tokens[i].carry == EXT_THEN) {
                        continue;
                    } else {
                        write_NodeList(&temp_body, parse(source, length, ext_start, i, tokens, NODE_WHEN_BODY + ext_type, file_name));
                        ext_type = tokens.tokens[i].carry;
                        ext_start = i + 1;
                    }
                }
            }
            if (!body_parsed) {
                write_NodeList(&temp_body, parse(source, length, start, end, tokens, NODE_MASTER_DO_BODY + tokens.tokens[start - 1].carry, file_name));
            } else {
                write_NodeList(&temp_body, parse(source, length, ext_start, end, tokens, NODE_WHEN_BODY + ext_type, file_name));
            }

            // reorganize the body
            
            // Encapsulate all the else bodies in the if body
            NodeList new_temp_body;
            init_NodeList(&new_temp_body);
            for (int i = 0; i < temp_body.count; i++) {
                int if_index = i;
                if (temp_body.nodes[i].type == NODE_IF_BODY) {
                    Node* if_node = &temp_body.nodes[if_index];
                    if (i + 1 < temp_body.count) {
                        while (
                            temp_body.nodes[i + 1].type == NODE_ELSE_BODY || 
                            (temp_body.nodes[i + 1].type == NODE_ELSE_BODY && temp_body.nodes[i + 2].type == NODE_IF_BODY) || 
                            (temp_body.nodes[i + 1].type == NODE_IF_BODY && temp_body.nodes[i].type == NODE_ELSE_BODY)
                        ) {
                            write_NodeList(&if_node->body, temp_body.nodes[i + 1]);
                            if_node = &if_node->body.nodes[if_node->body.count - 1];
                            i++;
                            if (i + 1 == temp_body.count) {
                                break;
                            }
                        }
                    }
                }
                write_NodeList(&new_temp_body, temp_body.nodes[if_index]);
            }

            free_NodeList(&temp_body);
            temp_body = new_temp_body;


            // if WHEN, WHILE, FOR, CATCH they should encapsulate everything before them (in their body)
            NodeList full_body;
            init_NodeList(&full_body);
            write_NodeList(&full_body, temp_body.nodes[0]);
            for (int i = 1; i < temp_body.count; i++) {
                if (temp_body.nodes[i].type == NODE_WHEN_BODY || temp_body.nodes[i].type == NODE_WHILE_BODY || temp_body.nodes[i].type == NODE_FOR_BODY || temp_body.nodes[i].type == NODE_CATCH_BODY) {
                    for (int j = 0; j < full_body.count; j++) {
                        write_NodeList(&temp_body.nodes[i].body, full_body.nodes[j]);
                    }
                    free_NodeList(&full_body);
                }
                write_NodeList(&full_body, temp_body.nodes[i]);
            }
            free_NodeList(&temp_body);
            root.body = full_body;

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
                    PRINT_ERROR(start, E_MISSING_CLOSING_PARENTHESIS);
                }
                if (endofblock + 1 != end) {
                    PRINT_ERROR(endofblock + 1, E_UNEXPECTED_TOKEN);
                }
                write_NodeList(&root.body, parse(source, length, start + 1, endofblock, tokens, NODE_BLOCK, file_name));
            } else if (tokens.tokens[start].type == TOKEN_IDENTIFIER) {
                write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_FUNCTION_CALL, file_name));
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
                write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_TYPE, file_name));
                
                if (tokens.tokens[i].type != TOKEN_IDENTIFIER) {
                    PRINT_ERROR(i, E_EXPECTED_IDENTIFIER);
                }
                write_NodeList(&root.body, parse(source, length, i, i + 1, tokens, NODE_IDENTIFIER, file_name));
                if (tokens.tokens[i + 1].type != TOKEN_OPERATOR || tokens.tokens[i + 1].carry != OPERATOR_ASSIGN) {
                    PRINT_ERROR(i + 1, E_EXPECTED_ASSIGNMENT_OPERATOR_PURE);
                }
                write_NodeList(&root.body, parse(source, length, i + 2, end, tokens, NODE_EXPRESSION, file_name));
            } else {
                PRINT_ERROR(start, E_EXPECTED_TYPE_INDENTIFIER);
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
                    PRINT_ERROR(j, E_EXPECTED_ASSIGNMENT_OPERATOR);
                }
            }
            if (j == start) {
                PRINT_ERROR(start, E_EXPECTED_IDENTIFIER);
            }
            write_NodeList(&root.body, parse(source, length, start, j - 1, tokens, NODE_SUBSCRIPT_EXPRESSION, file_name));
            write_NodeList(&root.body, parse(source, length, j - 1, j, tokens, NODE_OPERATOR, file_name));
            if (tokens.tokens[j - 1].carry == OPERATOR_INCREMENT || tokens.tokens[j - 1].carry == OPERATOR_DECREMENT) {
                if (j != end) {
                    PRINT_ERROR(j, E_UNEXPECTED_TOKEN);
                }
                break;
            }
            write_NodeList(&root.body, parse(source, length, j, end, tokens, NODE_EXPRESSION, file_name));
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
                write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_TYPE, file_name));
                
                // next the function name
                if (tokens.tokens[i].type != TOKEN_IDENTIFIER) {
                    PRINT_ERROR(i, E_EXPECTED_IDENTIFIER);
                }
                write_NodeList(&root.body, parse(source, length, i, i + 1, tokens, NODE_IDENTIFIER, file_name));

                // parameters
                if (tokens.tokens[i + 1].type != TOKEN_PARENTHESIS_OPEN || !CHECK_BRACKET_TYPE(tokens.tokens[i + 1].carry, BRACKET_ROUND)) {
                    PRINT_ERROR(i + 1, E_EXPECTED_BRACKET_ROUND);
                }
                // get end of parameters
                int j = getEndOfBlock(tokens, i + 1);
                if (j == -1) {
                    PRINT_ERROR(i + 1, E_MISSING_CLOSING_PARENTHESIS);
                }
                write_NodeList(&root.body, parse(source, length, i + 2, j, tokens, NODE_FUNCTION_DEFINITION_PARAMETERS, file_name));

                // function body
                if (tokens.tokens[j + 1].type != TOKEN_PARENTHESIS_OPEN || !CHECK_BRACKET_TYPE(tokens.tokens[j + 1].carry, BRACKET_CURLY)) {
                    PRINT_ERROR(j + 1, E_EXPECTED_BRACKET_CURLY);
                }
                // get end of block
                int k = getEndOfBlock(tokens, j + 1);
                if (k == -1) {
                    PRINT_ERROR(j + 1, E_MISSING_CLOSING_PARENTHESIS);
                }
                write_NodeList(&root.body, parse(source, length, j + 2, k, tokens, NODE_BLOCK, file_name));
                if (k + 1 != end) {
                    PRINT_ERROR(k + 1, E_UNEXPECTED_TOKEN);
                }
            } else {
                PRINT_ERROR(start, E_EXPECTED_TYPE_INDENTIFIER);
            }
            break;
        }

        case NODE_MASTER_INCLUDE_BODY: {
            // expression
            write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_EXPRESSION, file_name));
            break;
        }

        case NODE_MASTER_IMPORT_BODY: {
            // identifier
            if (tokens.tokens[start].type != TOKEN_STRING) {
                PRINT_ERROR(start, E_EXPECTED_STRING);
            }
            if (start + 1 != end) {
                PRINT_ERROR(start + 1, E_UNEXPECTED_TOKEN);
            }
            write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_STRING_LITERAL, file_name));
            break;
        }

        case NODE_MASTER_RETURN_BODY: {
            // expression
            if (start == end) {
                break; // no return value
            }
            write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_EXPRESSION, file_name));
            break;
        }

        case NODE_MASTER_BREAK_BODY:
        case NODE_MASTER_CONTINUE_BODY: {
            if (start != end) {
                PRINT_ERROR(start, E_UNEXPECTED_TOKEN);
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
                    write_NodeList(&root.body, parse(source, length, j, i, tokens, NODE_FUNCTION_DEFINITION_ARGUMENT, file_name));
                    j = i + 1;
                }
            }
            write_NodeList(&root.body, parse(source, length, j, end, tokens, NODE_FUNCTION_DEFINITION_ARGUMENT, file_name));

            break;
        }

        case NODE_FUNCTION_DEFINITION_ARGUMENT: {
            if (start == end) {
                PRINT_ERROR(start, E_EXPECTED_TYPE_INDENTIFIER);
            }
            // type
            int i = start;
            while (tokens.tokens[i++].type == TOKEN_VAR_TYPE) {}
            i--;
            write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_TYPE, file_name));
            // identifier
            if (tokens.tokens[i].type != TOKEN_IDENTIFIER) {
                PRINT_ERROR(i, E_EXPECTED_IDENTIFIER);
            }
            write_NodeList(&root.body, parse(source, length, i, i + 1, tokens, NODE_IDENTIFIER, file_name));
            if (i + 1 != end) {
                PRINT_ERROR(i + 1, E_UNEXPECTED_TOKEN);
            }
            break;
        }

        case NODE_FUNCTION_CALL: {
            // get the function expression
            int i = start;
            for (int j = start; j < end; j++) {
                if (tokens.tokens[j].type == TOKEN_PARENTHESIS_OPEN) {
                    i = j;
                    break;
                }
            }
            if (i == end - 1) {
                PRINT_ERROR(i, E_EXPECTED_BRACKET_ROUND);
            }
            if (i == start) {
                PRINT_ERROR(start, E_EXPECTED_IDENTIFIER);
            }
            write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_EXPRESSION, file_name));
            // get the arguments
            if (tokens.tokens[i].type != TOKEN_PARENTHESIS_OPEN || !CHECK_BRACKET_TYPE(tokens.tokens[i].carry, BRACKET_ROUND)) {
                PRINT_ERROR(i, E_EXPECTED_BRACKET_ROUND);
            }
            int j = getEndOfBlock(tokens, i);
            if (j == -1) {
                PRINT_ERROR(i, E_MISSING_CLOSING_PARENTHESIS);
            }
            write_NodeList(&root.body, parse(source, length, i + 1, j, tokens, NODE_ARGUMENTS, file_name));
            if (j + 1 != end) {
                PRINT_ERROR(j + 1, E_UNEXPECTED_TOKEN);
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
                    write_NodeList(&root.body, parse(source, length, j, i, tokens, NODE_EXPRESSION, file_name));
                    j = i + 1;
                }
            }
            write_NodeList(&root.body, parse(source, length, j, end, tokens, NODE_EXPRESSION, file_name));
            break;
        }

        case NODE_IF_BODY: {
            // expression
            int then_loc = -1;
            for (int i = start; i < end; i++) {
                SKIP_BLOCK(i);
                if (tokens.tokens[i].type == TOKEN_EXT && tokens.tokens[i].carry == EXT_THEN) {
                    then_loc = i;
                    break;
                }
            }
            if (then_loc == -1) {
                PRINT_ERROR(end - 1, E_EXPECTED_THEN);
            }

            write_NodeList(&root.body, parse(source, length, start, then_loc, tokens, NODE_EXPRESSION, file_name));
            write_NodeList(&root.body, parse(source, length, then_loc + 1, end, tokens, NODE_THEN_BODY, file_name));
            break;
        }


        case NODE_WHEN_BODY:
        case NODE_WHILE_BODY: {
            // expression
            write_NodeList(&root.body, parse(source, length, start, end, tokens, NODE_EXPRESSION, file_name));
            break;
        }

        case NODE_FOR_BODY: {
            // special type of FOR expression
            // get rid of parentheses
            int new_start = start;
            int new_end = end - 1;
            while (ENCASED_IN_BRACKETS(new_start, new_end, BRACKET_ROUND)) {
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
                PRINT_ERROR(i, E_EXPECTED_IDENTIFIER);
            }
            write_NodeList(&root.body, parse(source, length, new_start, i, tokens, NODE_EXPRESSION, file_name));
            // after the arrow must be an identifier
            if (tokens.tokens[i + 1].type != TOKEN_IDENTIFIER) {
                PRINT_ERROR(i + 1, E_EXPECTED_IDENTIFIER);
            }
            write_NodeList(&root.body, parse(source, length, i + 1, i + 2, tokens, NODE_IDENTIFIER, file_name));
            if (i + 1 != new_end) {
                PRINT_ERROR(i + 2, E_UNEXPECTED_TOKEN);
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
                    write_NodeList(&root.body, parse(source, length, j, i, tokens, NODE_OBJECT_ENTRY, file_name));
                    j = i + 1;
                }
            }
            write_NodeList(&root.body, parse(source, length, j, end, tokens, NODE_OBJECT_ENTRY, file_name));
            break;
        }

        case NODE_OBJECT_ENTRY: {
            // get the first : operator
            int i = start;
            for (i = start; i < end; i++) {
                SKIP_BLOCK(i);
                if (tokens.tokens[i].type == TOKEN_OPERATOR && tokens.tokens[i].carry == OPERATOR_COLON) {
                    break;
                }
            }

            if (i == start) {
                PRINT_ERROR(i, E_EXPECTED_IDENTIFIER);
            }
            if (i == end - 1) {
                PRINT_ERROR(i, E_EXPECTED_COLON_OPERATOR);
            }

            
            write_NodeList(&root.body, parse(source, length, start, i, tokens, NODE_EXPRESSION, file_name));
            write_NodeList(&root.body, parse(source, length, i + 1, end, tokens, NODE_EXPRESSION, file_name));
            break;
        }

        case NODE_SUBSCRIPT_EXPRESSION:
        case NODE_EXPRESSION: {
            int new_start = start;
            int new_end = end;
            while (ENCASED_IN_BRACKETS(new_start, new_end - 1, BRACKET_ROUND)) {
                new_start++;
                new_end--;
            }

            root.start = new_start;
            root.end = new_end;


            if (new_start == new_end) {
                PRINT_ERROR(new_start, E_EMPTY_EXPRESSION);
            }

            if (new_start == new_end - 1) {
                // single expression
                if (tokens.tokens[new_start].type == TOKEN_IDENTIFIER) {
                    root.type = NODE_IDENTIFIER;
                } else if (tokens.tokens[new_start].type == TOKEN_NUMBER) {
                    root.type = NODE_NUMBER_LITERAL;
                } else if (tokens.tokens[new_start].type == TOKEN_STRING) {
                    root.type = NODE_STRING_LITERAL;
                } else if (tokens.tokens[new_start].type == TOKEN_BOOLEAN) {
                    root.type = tokens.tokens[new_start].carry == 0 ? NODE_FALSE : NODE_TRUE;
                } else if (tokens.tokens[new_start].type == TOKEN_NULL) {
                    root.type = NODE_NULL_KEYWORD;
                } else {
                    PRINT_ERROR(new_start, E_UNEXPECTED_TOKEN);
                }
                break;
            }

            // split on the operators
            int op_loc = new_start - 1;
            int highest = -1;
            int highest_index = -1;

            int precedence_values[] = OPERATOR_PRECEDENCE;

            bool func_call = false;
            bool type_cast = false;
            bool is_unary = false;
            bool is_turnary = false;
            for (int i = new_start; i < new_end; i++) {
                if (i > new_start) {
                    if (tokens.tokens[i].type == TOKEN_PARENTHESIS_OPEN && CHECK_BRACKET_TYPE(tokens.tokens[i].carry, BRACKET_ROUND)) {
                        if (op_loc == i - 1) {
                            SKIP_BLOCK(i);
                            continue; // it's a normal bracket expression
                        }
                        int endofblock = getEndOfBlock(tokens, i);
                        if (endofblock == -1) {
                            PRINT_ERROR(i, E_MISSING_CLOSING_PARENTHESIS);
                        }
                        if (1 >= highest) {
                            is_unary = false;
                            is_turnary = false;
                            highest = 1;
                            highest_index = i;
                            func_call = true;
                            type_cast = false;
                        }
                    }
                    // type cast
                    if (tokens.tokens[i-1].type == TOKEN_PARENTHESIS_CLOSED && CHECK_BRACKET_TYPE(tokens.tokens[i-1].carry, BRACKET_ROUND)) {
                        if (op_loc == i + 1){
                            continue; // it's a normal bracket expression
                        }
                        int startofblock = getStartOfBlock(tokens, i-1);
                        if (startofblock == -1) {
                            PRINT_ERROR(i, E_MISSING_OPENING_PARENTHESIS);
                        }

                        // check if it's all type tokens in the block
                        bool all_type = true;
                        for (int j = startofblock + 1; j < i - 1; j++) {
                            if (tokens.tokens[j].type != TOKEN_VAR_TYPE) {
                                all_type = false;
                                break;
                            }
                        }
                        all_type = all_type && startofblock + 1 != i - 1;

                        if (2 >= highest && all_type) {
                            if (is_unary) {
                                continue;
                            }
                            is_unary = false;
                            is_turnary = false;
                            highest = 2;
                            highest_index = type_cast ? highest_index : startofblock;
                            func_call = false;
                            type_cast = true;
                        }
                    }
                }
                SKIP_BLOCK(i);
                if (tokens.tokens[i].type == TOKEN_OPERATOR) {
                    op_loc = i;
                    int precedence = precedence_values[tokens.tokens[i].carry];

                    bool temp_unary = false;
                    if (i == new_start || tokens.tokens[i - 1].type == TOKEN_OPERATOR || tokens.tokens[i - 1].type == TOKEN_PARENTHESIS_OPEN || (tokens.tokens[i - 1].type == TOKEN_PARENTHESIS_CLOSED && tokens.tokens[i - 2].type == TOKEN_VAR_TYPE && tokens.tokens[i - 3].type == TOKEN_PARENTHESIS_OPEN)) {
                        if (tokens.tokens[i].type == TOKEN_OPERATOR && isUnaryOperator(tokens.tokens[i].carry)) {
                            precedence = 2; // unary operator precedence
                            temp_unary = true;
                        } else {
                            PRINT_ERROR(i, E_NON_UNARY_OPERATOR);
                        }
                    }

                    // ternary operator
                    bool temp_turnary = false;
                    if (tokens.tokens[i].carry == OPERATOR_QUESTION) {
                        // check if theres a colon
                        bool colon = false;
                        int j = i + 1;
                        int q_count = 0;
                        for (j = i + 1; j < new_end; j++) {
                            SKIP_BLOCK(j);
                            if (tokens.tokens[j].type == TOKEN_OPERATOR && tokens.tokens[j].carry == OPERATOR_QUESTION) {
                                q_count++;
                            }

                            if (tokens.tokens[j].type == TOKEN_OPERATOR && tokens.tokens[j].carry == OPERATOR_COLON) {
                                if (!q_count--) {
                                    colon = true;
                                    break;
                                }
                            }
                        }

                        if (!colon) {
                            PRINT_ERROR(i, E_INCOMPLETE_TERNARY_OPERATOR);
                        }

                        i = j;
                        temp_turnary = true;
                    }

                    if (precedence >= highest) {
                        if (type_cast) {
                            continue; // type cast has the highest precedence
                        }
                        highest = precedence;
                        highest_index = is_unary && temp_unary ? highest_index : op_loc; // when it's a unary operator, the highest index is the previous one
                        is_unary = temp_unary;
                        is_turnary = temp_turnary;
                        func_call = false;
                        type_cast = false;
                    }
                    op_loc = i;
                }
            }

            if (func_call) {
                write_NodeList(&root.body, parse(source, length, new_start, getEndOfBlock(tokens, highest_index) + 1, tokens, NODE_FUNCTION_CALL, file_name));
                break;
            }
            if (type_cast) {
                write_NodeList(&root.body, parse(source, length, highest_index, new_end, tokens, NODE_TYPE_CAST, file_name));
                break;
            }
            if (highest == -1) {
                // no operators

                if (ENCASED_IN_BRACKETS(new_start, new_end - 1, BRACKET_SQUARE)) {
                    write_NodeList(&root.body, parse(source, length, new_start + 1, new_end - 1, tokens, NODE_ARRAY_EXPRESSION, file_name));
                } else if (ENCASED_IN_BRACKETS(new_start, new_end - 1, BRACKET_CURLY)) {
                    write_NodeList(&root.body, parse(source, length, new_start + 1, new_end - 1, tokens, NODE_OBJECT_EXPRESSION, file_name));
                } else {
                    PRINT_ERROR(new_start, E_UNEXPECTED_TOKEN);
                }
            } else {
                if (is_unary) {
                    // unary operator
                    write_NodeList(&root.body, parse(source, length, highest_index, new_end, tokens, NODE_UNARY_EXPRESSION, file_name));
                } else if (highest_index == new_end - 1) {
                    // error if the last token is an operator
                    PRINT_ERROR(highest_index, E_UNEXPECTED_TOKEN);
                } else if (is_turnary) {
                    // ternary operator
                    write_NodeList(&root.body, parse(source, length, new_start, highest_index, tokens, NODE_EXPRESSION, file_name)); // condition

                    int j = highest_index + 1;
                    int q_count = 0;
                    for (; j < new_end; j++) {
                        SKIP_BLOCK(j);
                        if (tokens.tokens[j].type == TOKEN_OPERATOR && tokens.tokens[j].carry == OPERATOR_QUESTION) {
                            q_count++;
                        }

                        if (tokens.tokens[j].type == TOKEN_OPERATOR && tokens.tokens[j].carry == OPERATOR_COLON) {
                            if (!q_count--) {
                                break;
                            }
                        }
                    }

                    write_NodeList(&root.body, parse(source, length, highest_index + 1, j, tokens, NODE_EXPRESSION, file_name)); // true_expression
                    write_NodeList(&root.body, parse(source, length, j + 1, new_end, tokens, NODE_EXPRESSION, file_name)); // false_expression

                    root.type = NODE_TERNARY_EXPRESSION;
                } else {
                    write_NodeList(&root.body, parse(source, length, new_start, highest_index, tokens, type, file_name));

                    if (tokens.tokens[highest_index].type != TOKEN_OPERATOR) {
                        PRINT_ERROR(highest_index, E_UNEXPECTED_TOKEN);
                    }
                    OperatorType op = tokens.tokens[highest_index].carry;
                    if (isAssignmentOperator(op) || op == OPERATOR_NOT || op == OPERATOR_NOT_BITWISE || op == OPERATOR_COMMA || op == OPERATOR_QUESTION || op == OPERATOR_COLON) {
                        PRINT_ERROR(highest_index, E_NON_BINARY_OPERATOR);
                    }

                    if (type == NODE_SUBSCRIPT_EXPRESSION && op != OPERATOR_HASH && op != OPERATOR_ARROW) {
                        PRINT_ERROR(highest_index, E_EXPECTED_HASH_OPERATOR);
                    }

                    write_NodeList(&root.body, parse(source, length, highest_index, highest_index + 1, tokens, NODE_OPERATOR, file_name));
                    write_NodeList(&root.body, parse(source, length, highest_index + 1, new_end, tokens, NODE_EXPRESSION, file_name));
                }
            }
            
            break;
        }

        case NODE_UNARY_EXPRESSION: {
            if (start == end) {
                PRINT_ERROR(start, E_UNEXPECTED_TOKEN);
            }
            if (tokens.tokens[start].type != TOKEN_OPERATOR) {
                PRINT_ERROR(start, E_UNEXPECTED_TOKEN);
            }

            if (!isUnaryOperator(tokens.tokens[start].carry)) {
                PRINT_ERROR(start, E_NON_UNARY_OPERATOR);
            }
            
            write_NodeList(&root.body, parse(source, length, start, start + 1, tokens, NODE_OPERATOR, file_name));
            write_NodeList(&root.body, parse(source, length, start + 1, end, tokens, NODE_EXPRESSION, file_name));
            break;
        }

        case NODE_TYPE_CAST: {
            if (start == end) {
                PRINT_ERROR(start, E_UNEXPECTED_TOKEN);
            }

            if (tokens.tokens[start].type != TOKEN_PARENTHESIS_OPEN || !CHECK_BRACKET_TYPE(tokens.tokens[start].carry, BRACKET_ROUND)) {
                PRINT_ERROR(start, E_EXPECTED_BRACKET_ROUND);
            }

            int i = getEndOfBlock(tokens, start);
            if (i == -1) {
                PRINT_ERROR(start, E_MISSING_CLOSING_PARENTHESIS);
            }

            write_NodeList(&root.body, parse(source, length, start + 1, i, tokens, NODE_TYPE, file_name));
            write_NodeList(&root.body, parse(source, length, i + 1, end, tokens, NODE_EXPRESSION, file_name));
            break;
        }

        case NODE_TYPE: {
            if (start == end) {
                PRINT_ERROR(start, E_EXPECTED_TYPE_INDENTIFIER);
            }
            if (end - start > 1) {
                PRINT_ERROR(start, E_INVALID_TYPE);
            }
            break;
        }


        default: {
            break; // do nothing
        }
    }

    return root;
}
#undef PRINT_ERROR