#include "../include/common.h"

#include "../include/lexer.h"

void sortTokens (TokenList* list) {
    Token* tokens = list->tokens;
    int tokenCount = list->count;
    for (int i = 0; i < tokenCount; i++) {
        for (int j = 0; j < tokenCount - i - 1; j++) {
            if (tokens[j].start > tokens[j + 1].start) {
                Token temp = tokens[j];
                tokens[j] = tokens[j + 1];
                tokens[j + 1] = temp;
            }
        }
    }
}

void trimComments (TokenList* list) {
    Token* tokens = list->tokens;
    int tokenCount = list->count;
    for (int i = 0; i < tokenCount; i++) {
        if (tokens[i].type == TOKEN_COMMENT) {
            for (int j = i; j < tokenCount; j++) {
                tokens[j] = tokens[j + 1];
            }
            tokenCount--;
            i--;
        }
    }
    list->count = tokenCount;
}

#define MAX_TEMPLETE_RECURSION 255

int tokenise (TokenList* list, char* full_code, const int code_length, VirtualMachine* vm, const char* file_name) {
    int tokenCount = list->count;

    #define SKIP_TOKEN() \
        for (int t = 0; t < tokenCount; t++) { \
            if (i == list->tokens[t].start - full_code) { \
                i += list->tokens[t].length; \
            } \
        }

    #define REFRESH_LIST() \
        tokenCount = list->count; \
        sortTokens(list);

    // get comment and string tokens
    int start = 0;
    int end = 0;
    char quotationtype = '\0';
    int escapeCount = 0;
    int stringCount = 0;

    bool in_template = false;
    int template_start_positions[MAX_TEMPLETE_RECURSION];
    int template_bracket_depths[MAX_TEMPLETE_RECURSION];
    int template_ids[MAX_TEMPLETE_RECURSION];
    int template_id = 0;
    int template_start_count = 0;
    int bracket_depth = 0;

    for (int i = 0; i < code_length; i++) {
        // handle inside template block
        if (template_start_count > 0) {
            // closing template string with backtick
            if (full_code[i] == '`' && escapeCount % 2 == 0) {
                if (in_template && template_start_count > 0) {
                    end = i;
                    DOSATO_ADD_TOKEN(list, TOKEN_TEMPLATE_END, full_code + template_start_positions[template_start_count - 1], end - template_start_positions[template_start_count - 1], template_ids[template_start_count - 1]);
                    template_start_count--;
                    in_template = false;  // nested template check
                    start = end + 1;
                    continue;
                }
            }

            // handling `{}` expression block
            if (full_code[i] == '{' && escapeCount % 2 == 0 && in_template) {
                in_template = false;  // entering template expression
                template_bracket_depths[template_start_count - 1] = bracket_depth++;

                // emit token for string part before `{`
                DOSATO_ADD_TOKEN(list, TOKEN_TEMPLATE, full_code + template_start_positions[template_start_count - 1], i - template_start_positions[template_start_count - 1] - 1, template_ids[template_start_count - 1]);

                start = i + 1;  // mark the start of `{`
                continue;
            }

            // handle curly braces for template expression depth
            if (full_code[i] == '{' && escapeCount % 2 == 0 && !quotationtype && !in_template) {
                bracket_depth++;
            }

            if (full_code[i] == '}' && escapeCount % 2 == 0 && !quotationtype && !in_template) {
                if (--bracket_depth == template_bracket_depths[template_start_count - 1]) {
                    in_template = true;  // end of `{}` block
                    start = i + 1;  // resume after closing brace
                    template_start_positions[template_start_count - 1] = start;
                }
            }
        }

        if (!quotationtype && !in_template) {
            if ((full_code[i] == '"' || full_code[i] == '\'') && escapeCount % 2 == 0) {
                quotationtype = full_code[i];
                start = i;
                continue;
            }
            if (full_code[i] == '`' && escapeCount % 2 == 0) {
                in_template = true;
                if (template_start_count >= MAX_TEMPLETE_RECURSION) {
                    printError(full_code, start, file_name, E_TEMPLATE_RECURSION_LIMIT, 1);
                }
                template_bracket_depths[template_start_count] = bracket_depth;
                template_ids[template_start_count] = template_id++;
                template_start_positions[template_start_count++] = i;
                start = i;
                continue;
            }
            if (full_code[i] == '/' && full_code[i + 1] == '/') {
                // kandle single-line comment
                int foundEnd = 0;
                start = i;
                for (int j = i; j < code_length; j++) {
                    if (full_code[j] == '\n') {
                        end = j - 1;
                        DOSATO_ADD_TOKEN(list, TOKEN_COMMENT, full_code + start, end - start, 0);
                        start = end + 1;
                        i = j;
                        foundEnd = 1;
                        break;
                    }
                }
                if (!foundEnd) {
                    end = code_length - 1;
                    DOSATO_ADD_TOKEN(list, TOKEN_COMMENT, full_code + start, end - start, 0);
                    start = end + 1;
                    i = code_length;
                }
            }

            if (full_code[i] == '/' && full_code[i + 1] == '*') {
                // handle block comment
                int foundEnd = 0;
                start = i;
                for (int j = i; j < code_length; j++) {
                    if (full_code[j] == '*' && full_code[j + 1] == '/') {
                        end = j + 1;
                        DOSATO_ADD_TOKEN(list, TOKEN_COMMENT, full_code + start, end - start, 0);
                        start = end + 1;
                        i = j + 1;
                        foundEnd = 1;
                        break;
                    }
                }
                if (!foundEnd) {
                    end = code_length - 1;
                    DOSATO_ADD_TOKEN(list, TOKEN_COMMENT, full_code + start, end - start, 0);
                    start = end + 1;
                    i = code_length;
                }
            }
        } else {
            // handle string literals
            if (full_code[i] == quotationtype && escapeCount % 2 == 0) {
                end = i;
                int id = 0;
                char* lit = malloc(end - start + 2);
                memcpy(lit, full_code + start, end - start + 2);
                lit[end - start + 1] = '\0';

                if (lit[0] == '\'') {
                    if (lit[2] != '\'' && !(lit[3] == '\'' && lit[1] == '\\') ) {
                        printError(full_code, start, file_name, E_INVALID_CHAR_LITERAL, 1);
                    } else if (lit[3] == '\'' && lit[1] == '\\') {
                        if (lit[2] != 'n' && lit[2] != 't' && lit[2] != 'r' && lit[2] != 'b' && lit[2] != 'f' && lit[2] != '0' && lit[2] != '\\' && lit[2] != '\'' && lit[2] != '"') {
                            printError(full_code, start, file_name, E_INVALID_CHAR_LITERAL, 1);
                        }
                    }
                }

                if (hasName(&vm->constants_map, lit)) {
                    id = getName(&vm->constants_map, lit);
                } else {
                    id = addName(&vm->constants_map, lit);
                    stringCount++;
                }
                DOSATO_ADD_TOKEN(list, TOKEN_STRING, full_code + start, end - start, id);
                start = end + 1;
                quotationtype = '\0';
                free(lit);
            }
        }

        // Check for escape character
        if (full_code[i] == '\\') {
            escapeCount++;
        } else {
            escapeCount = 0;
        }
    }

    if (quotationtype) {
        printError(full_code, start, file_name, E_UNCLOSED_STRING_LITERAL, 1);
    }

    if (template_start_count > 0) {
        // Handle unclosed template literals
        printError(full_code, start, file_name, E_UNCLOSED_STRING_LITERAL, 1);
    }

    
    REFRESH_LIST()
    
    // get master tokens
    const char* mastertokens[] = MASTER_KEYWORDS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(mastertokens)/sizeof(char*); j++) {
            // check if the previous char is not alphanumeric
            if (i > 0) {
                if (IS_ALPHANUMERIC(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(mastertokens[j]) < code_length) {
                    if (IS_ALPHANAMERIC(full_code[i + strlen(mastertokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
            toUpper(next_word);
            if (!strcmp(next_word, mastertokens[j])) {
                free(next_word);
                DOSATO_ADD_TOKEN(list, TOKEN_MASTER_KEYWORD, full_code + i, strlen(mastertokens[j]) - 1, j);
                i += strlen(mastertokens[j]) - 1;
                break;
            } else {
                free(next_word);
            }
        }
    }
    
    REFRESH_LIST()

    // get var_type tokens
    const char* var_typetokens[] = VAR_TYPES;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(var_typetokens)/sizeof(char*); j++) {
            // check if the previous char is not alphanumeric
            if (i > 0) {
                if (IS_ALPHANUMERIC(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(var_typetokens[j]) < code_length) {
                    if (IS_ALPHANAMERIC(full_code[i + strlen(var_typetokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
            toUpper(next_word);
            if (!strcmp(next_word, var_typetokens[j])) {
                free(next_word);
                DOSATO_ADD_TOKEN(list, TOKEN_VAR_TYPE, full_code + i, strlen(var_typetokens[j]) - 1, j);
                i += strlen(var_typetokens[j]) - 1;
                break;
            } else {
                free(next_word);
            }
        }
    }
    
    REFRESH_LIST()

    // get extension tokens
    const char* extension_tokens[] = EXTENSION_KEYWORDS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(extension_tokens)/sizeof(char*); j++) {
            // check if the previous char is not alphanumeric
            if (i > 0) {
                if (IS_ALPHANUMERIC(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(extension_tokens[j]) < code_length) {
                    if (IS_ALPHANAMERIC(full_code[i + strlen(extension_tokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
            toUpper(next_word);
            if (!strcmp(next_word, extension_tokens[j])) {
                free(next_word);
                DOSATO_ADD_TOKEN(list, TOKEN_EXT, full_code + i, strlen(extension_tokens[j]) - 1, j);
                i += strlen(extension_tokens[j]) - 1;
                break;
            } else {
                free(next_word);
            }
        }
    }
    
    REFRESH_LIST()
    
    // get bracket tokens
    const char* brackettokens[] = BRACKETS;
    int bracketTier = 0;
    int bracketTypeHiarcy[code_length];
    for (int k = 0; k < code_length; k++) bracketTypeHiarcy[k] = -1;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        // check for opening brackets
        for (int j = 0; j < sizeof(brackettokens)/sizeof(char*); j++) {
            if (full_code[i] == brackettokens[j][0]) {
                bracketTier++;
                DOSATO_ADD_TOKEN(list, TOKEN_PARENTHESIS_OPEN, full_code + i, 0, getBracketType(full_code[i]) | bracketTier);
                bracketTypeHiarcy[bracketTier - 1] = j;
                break;
            }
        }
        // check for closing brackets
        for (int j = 0; j < sizeof(brackettokens)/sizeof(char*); j++) {
            if (full_code[i] == brackettokens[j][1]) {
                DOSATO_ADD_TOKEN(list, TOKEN_PARENTHESIS_CLOSED, full_code + i, 0, getBracketType(full_code[i]) | (bracketTypeHiarcy[bracketTier - 1] == j ? bracketTier : -1));
                if (bracketTypeHiarcy[bracketTier - 1] == j) {
                    bracketTypeHiarcy[bracketTier - 1] = -1;
                    bracketTier--;
                }
                break;
            }
        }
    }
    
    REFRESH_LIST()

    int numberCount = 0;

    // get hex number tokens

    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        if (full_code[i] == '0' && (full_code[i+1] == 'x' || full_code[i+1] == 'X')) {
            if (IS_ALPHANUMERIC(full_code[i-1]) || full_code[i-1] == '.' || (IS_ALPHANAMERIC(full_code[i+2]) && !IS_HEXADECIMAL(full_code[i+2]))) {
                continue;
            }
            int start = i;
            int end = i;
            for (int j = i + 2; j < code_length; j++) {
                if (!IS_HEXADECIMAL(full_code[j])) {
                    end = j - 1;
                    break;
                }
                end = j;
            }
            int id = 0;
            char* lit = malloc(end - start + 2);
            memcpy(lit, full_code + start, end - start + 2);
            lit[end - start + 1] = '\0';
            if (hasName(&vm->constants_map, lit)) {
                id = getName(&vm->constants_map, lit);
            } else {
                id = addName(&vm->constants_map, lit);
                numberCount++;
            }
            DOSATO_ADD_TOKEN(list, TOKEN_NUMBER_HEX, full_code + start, end - start, id);
            free(lit);
            i = end;
        }
    }

    REFRESH_LIST()

    // get octal number tokens
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        if (full_code[i] == '0' && (IS_OCTAL(full_code[i+1]) || full_code[i+1] == 'o' || full_code[i+1] == 'O')) {
            if (IS_ALPHANUMERIC(full_code[i-1]) || full_code[i-1] == '.' || IS_ALPHANAMERIC(full_code[i+2])) {
                continue;
            }
            int start = i;
            int end = i;
            bool invalid = false;
            for (int j = i + 2; j < code_length; j++) {
                if (!IS_OCTAL(full_code[j])) {
                    end = j - 1;
                    if (full_code[j] == '8' || full_code[j] == '9') {
                        invalid = true;
                    }
                    break;
                }
                end = j;
            }
            if (invalid) {
                continue;
            }
            int id = 0;
            char* lit = malloc(end - start + 2);
            memcpy(lit, full_code + start, end - start + 2);
            lit[end - start + 1] = '\0';
            if (hasName(&vm->constants_map, lit)) {
                id = getName(&vm->constants_map, lit);
            } else {
                id = addName(&vm->constants_map, lit);
                numberCount++;
            }
            DOSATO_ADD_TOKEN(list, TOKEN_NUMBER_OCTAL, full_code + start, end - start, id);
            free(lit);
            i = end;
        }
    }

    REFRESH_LIST()

    // get binary number tokens
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        if (full_code[i] == '0' && (full_code[i+1] == 'b' || full_code[i+1] == 'B')) {
            if (IS_ALPHANUMERIC(full_code[i-1]) || full_code[i-1] == '.' || IS_ALPHANAMERIC(full_code[i+2])) {
                continue;
            }
            int start = i;
            int end = i;
            for (int j = i + 2; j < code_length; j++) {
                if (!IS_BINARY(full_code[j])) {
                    end = j - 1;
                    break;
                }
                end = j;
            }
            int id = 0;
            char* lit = malloc(end - start + 2);
            memcpy(lit, full_code + start, end - start + 2);
            lit[end - start + 1] = '\0';
            if (hasName(&vm->constants_map, lit)) {
                id = getName(&vm->constants_map, lit);
            } else {
                id = addName(&vm->constants_map, lit);
                numberCount++;
            }
            DOSATO_ADD_TOKEN(list, TOKEN_NUMBER_BINARY, full_code + start, end - start, id);
            free(lit);
            i = end;
        }
    }
    
    REFRESH_LIST()

    // get number tokens
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        if (IS_FLOATERIC(full_code[i])) {
            if (!(IS_NUMERIC(full_code[i]) || full_code[i] == '.') || IS_ALPHANAMERIC(full_code[i-1]) || IS_ALPHANAMERIC(full_code[i+1])) {
                if (IS_ALPHANAMERIC(full_code[i-1]) && full_code[i] == '.') printError(full_code, i, file_name, E_INVALID_NUMBER_LITERAL, 1);
                for (int k = i; k < code_length && IS_FLOATERIC(full_code[i]); k++) {
                    i = k;
                }
                continue;
            }
            bool foundDot = false;
            int start = i;
            int end = i;

            bool invalid = false;
            for (int j = i; j < code_length; j++) {
                if (IS_ALPHANAMERIC(full_code[j-1]) && !IS_FLOATERIC(full_code[j])) {
                    invalid = true;
                    for (int k = j; k < code_length && IS_FLOATERIC(full_code[k]); k++) {
                        end = k;
                    }
                    printError(full_code, start, file_name, E_INVALID_NUMBER_LITERAL, 1);
                }
                if (full_code[j] == '.') {
                    if (foundDot) {
                        invalid = true;
                        for (int k = j; k < code_length && IS_FLOATERIC(full_code[k]); k++) {
                            end = k;
                        }
                        printError(full_code, start, file_name, E_INVALID_NUMBER_LITERAL, 1);
                    }
                    foundDot = true;
                    if (!IS_NUMERIC(full_code[j+1])) {
                        invalid = true;
                        for (int k = j; k < code_length && IS_FLOATERIC(full_code[k]); k++) {
                            end = k;
                        }
                        printError(full_code, start, file_name, E_INVALID_NUMBER_LITERAL, 1);
                    }
                }
                if (IS_FLOATERIC(full_code[j])) {
                    if (full_code[j] == 'F') {
                        if (IS_ALPHANUMERIC(full_code[j+1])) {
                            invalid = true;
                            for (int k = j; k < code_length && IS_FLOATERIC(full_code[k]); k++) {
                                end = k;
                            }
                            printError(full_code, start, file_name, E_INVALID_NUMBER_LITERAL, 1);
                        }
                        if (IS_NUMERIC(full_code[j-1])) {
                            end = j;
                            break;
                        }
                        continue;
                    }
                    end = j;
                } else {
                    break;
                }
            }
            if (!invalid) {
                int id = 0;
                char* lit = malloc(end - start + 2);
                memcpy(lit, full_code + start, end - start + 2);
                lit[end - start + 1] = '\0';
                if (hasName(&vm->constants_map, lit)) {
                    id = getName(&vm->constants_map, lit);
                } else {
                    id = addName(&vm->constants_map, lit);
                    numberCount++;
                }
                DOSATO_ADD_TOKEN(list, TOKEN_NUMBER, full_code + start, end - start, id);
                free(lit);
            }
            i = end;
        }
    }
    
    REFRESH_LIST()

    // get operator tokens
    const char* operatortokens[] = OPERATORS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(operatortokens)/sizeof(char*); j++) {
            if (i + 2 >= code_length) break;
            char hugeoperator[4] = { full_code[i], full_code[i+1 < code_length ? i+1 : 0], full_code[i+2 < code_length ? i+2 : 0], '\0' };
            if (!strcmp(hugeoperator, operatortokens[j])) {
                DOSATO_ADD_TOKEN(list, TOKEN_OPERATOR, full_code + i, strlen(operatortokens[j]) - 1, j);
                i += 3;
                break;
            }
        }
        for (int j = 0; j < sizeof(operatortokens)/sizeof(char*); j++) {
            if (i + 1 >= code_length) break;
            char bigoperator[3] = { full_code[i], full_code[i+1 < code_length ? i+1 : 0], '\0' };
            if (!strcmp(bigoperator, operatortokens[j])) {
                DOSATO_ADD_TOKEN(list, TOKEN_OPERATOR, full_code + i, strlen(operatortokens[j]) - 1, j);
                i += 2;
                break;
            }
        }
        for (int j = 0; j < sizeof(operatortokens)/sizeof(char*); j++) {
            if (full_code[i] == operatortokens[j][0]) {
                DOSATO_ADD_TOKEN(list, TOKEN_OPERATOR, full_code + i, strlen(operatortokens[j]) - 1, j);
                break;
            }
        }
    }

    REFRESH_LIST()

    // get TRUE and FALSE tokens
    const char* booltokens[] = BOOLEAN_KEYWORDS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(booltokens)/sizeof(char*); j++) {
            // check if the previous char is not alphanumeric
            if (i > 0) {
                if (IS_ALPHANUMERIC(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(booltokens[j]) < code_length) {
                    if (IS_ALPHANAMERIC(full_code[i + strlen(booltokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
            toUpper(next_word);
            if (!strcmp(next_word, booltokens[j])) {
                free(next_word);
                DOSATO_ADD_TOKEN(list, TOKEN_BOOLEAN, full_code + i, strlen(booltokens[j]) - 1, j);
                i += strlen(booltokens[j]) - 1;
                break;
            } else {
                free(next_word);
            }
        }
    }

    REFRESH_LIST()

    //get NULL tokens
    const char* nulltokens[] = NULL_KEYWORDS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(nulltokens)/sizeof(char*); j++) {
            // check if the previous char is not alphanumeric
            if (i > 0) {
                if (IS_ALPHANUMERIC(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(nulltokens[j]) < code_length) {
                    if (IS_ALPHANAMERIC(full_code[i + strlen(nulltokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
            toUpper(next_word);
            if (!strcmp(next_word, nulltokens[j])) {
                free(next_word);
                DOSATO_ADD_TOKEN(list, TOKEN_NULL_KEYWORD, full_code + i, strlen(nulltokens[j]) - 1, j);
                i += strlen(nulltokens[j]) - 1;
                break;
            } else {
                free(next_word);
            }
        }
    }

    REFRESH_LIST()

    // get INFINITY tokens
    const char* infinitytokens[] = INFINITY_KEYWORDS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(infinitytokens)/sizeof(char*); j++) {
            // check if the previous char is not alphanumeric
            if (i > 0) {
                if (IS_ALPHANUMERIC(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(infinitytokens[j]) < code_length) {
                    if (IS_ALPHANAMERIC(full_code[i + strlen(infinitytokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
            toUpper(next_word);
            if (!strcmp(next_word, infinitytokens[j])) {
                free(next_word);
                DOSATO_ADD_TOKEN(list, TOKEN_INFINITY_KEYWORD, full_code + i, strlen(infinitytokens[j]) - 1, j);
                i += strlen(infinitytokens[j]) - 1;
                break;
            } else {
                free(next_word);
            }
        }
    }

    REFRESH_LIST()

    // get NAN tokens
    const char* nantokens[] = NAN_KEYWORDS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(nantokens)/sizeof(char*); j++) {
            // check if the previous char is not alphanumeric
            if (i > 0) {
                if (IS_ALPHANUMERIC(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(nantokens[j]) < code_length) {
                    if (IS_ALPHANAMERIC(full_code[i + strlen(nantokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
            toUpper(next_word);
            if (!strcmp(next_word, nantokens[j])) {
                free(next_word);
                DOSATO_ADD_TOKEN(list, TOKEN_NAN_KEYWORD, full_code + i, strlen(nantokens[j]) - 1, j);
                i += strlen(nantokens[j]) - 1;
                break;
            } else {
                free(next_word);
            }
        }
    }

    REFRESH_LIST()

    // get reserved keywords
    const char* reservedtokens[] = RESERVED_KEYWORDS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(reservedtokens)/sizeof(char*); j++) {
            // check if the previous char is not alphanumeric
            if (i > 0) {
                if (IS_ALPHANUMERIC(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(reservedtokens[j]) < code_length) {
                    if (IS_ALPHANAMERIC(full_code[i + strlen(reservedtokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
            toUpper(next_word);
            if (!strcmp(next_word, reservedtokens[j])) {
                free(next_word);
                DOSATO_ADD_TOKEN(list, TOKEN_RESERVED_KEYWORD, full_code + i, strlen(reservedtokens[j]) - 1, j);
                i += strlen(reservedtokens[j]) - 1;
                break;
            } else {
                free(next_word);
            }
        }
    }

    REFRESH_LIST()
    
    // add the _ to the map
    if (!hasName(&vm->mappings, "_")) {
        addName(&vm->mappings, "_");
    }
    
    // get identifier tokens (variables, functions, etc.)
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        if (IS_ALPHANUMERIC(full_code[i])) {
            char* word = getWord(full_code, i);
            if (strlen(word) > 0) {
                int id = 0;
                if (hasName(&vm->mappings, word)) {
                    id = getName(&vm->mappings, word);
                } else {
                    id = addName(&vm->mappings, word);
                }
                DOSATO_ADD_TOKEN(list, TOKEN_IDENTIFIER, full_code + i, strlen(word) - 1, id);
                i += strlen(word) - 1;
            }
            free(word);
        }
    }
    
    REFRESH_LIST()

    // check if theres any invalid tokens
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        if (!IS_SPACE(full_code[i])) {
            printError(full_code, i, file_name, E_UNEXPECTED_TOKEN, 1);
        }
    }

    // finalise tokens
    REFRESH_LIST()
    trimComments(list); // comments will be ignored from now on
    // parse the constant mappings
    for (int i = 0; i < stringCount; i++) {
        // check which quotes are used
        char* lit = vm->constants_map.names[i + vm->constants_map.count - stringCount - numberCount];
        char quote = lit[0];
        if (quote == '\'') {
            char val = lit[1];

            if (lit[1] == '\\') {
                // parse escape sequences

                switch (lit[2]) {
                    case 'n':
                        val = '\n';
                        break;
                    case 't':
                        val = '\t';
                        break;
                    case 'r':
                        val = '\r';
                        break;
                    case '0':
                        val = '\0';
                        break;
                    case 'b':
                        val = '\b';
                        break;
                    case 'f':
                        val = '\f';
                        break;
                    case '\\':
                        val = '\\';
                        break;
                    case '\'':
                        val = '\'';
                        break;
                    case '"':
                        val = '"';
                        break;
                    default:
                        val = lit[2]; // if it's not an escape sequence, just use the character
                        break;
                }
            }

            write_ValueArray(&vm->constants, BUILD_CHAR(val));
        }
        if (quote == '"') {
            char* val = malloc(strlen(lit) - 1);
            memcpy(val, lit + 1, strlen(lit) - 2);
            val[strlen(lit) - 2] = '\0';

            // parse escape sequences
            for (int j = 0; j < strlen(val); j++) {
                if (val[j] == '\\') {
                    switch (val[j + 1]) {
                        case 'n':
                            val[j] = '\n';
                            break;
                        case 't':
                            val[j] = '\t';
                            break;
                        case 'r':
                            val[j] = '\r';
                            break;
                        case '0':
                            val[j] = '\0';
                            break;
                        case 'b':
                            val[j] = '\b';
                            break;
                        case 'f':
                            val[j] = '\f';
                            break;
                        case '\\':
                            val[j] = '\\';
                            break;
                        case '\'':
                            val[j] = '\'';
                            break;
                        case '"':
                            val[j] = '"';
                            break;
                        default:
                            val[j] = val[j + 1]; // if it's not an escape sequence, just use the character
                            break;
                    }
                    for (int k = j + 1; k < strlen(val); k++) {
                        val[k] = val[k + 1];
                    }
                } else if (val[j] == '\n') {
                    val[j] = ' ';
                    break;
                }
            }

            write_ValueArray(&vm->constants, BUILD_STRING(val, false));
        }
    }
    for (int i = 0; i < numberCount; i++) {
        char* lit = vm->constants_map.names[i + vm->constants_map.count - numberCount];
        bool isInt = true;

        // Check for hexadecimal
        if (strlen(lit) > 2 && lit[0] == '0' && (lit[1] == 'x' || lit[1] == 'X')) {
            write_ValueArray(&vm->constants, BUILD_ULONG(strtoull(lit + 2, NULL, 16)));
            isInt = false;
        }
        // Check for octal
        else if (strlen(lit) > 1 && lit[0] == '0' && (IS_OCTAL(lit[1]) || lit[1] == 'o' || lit[1] == 'O')) {
            bool invalid = false;
            for (int j = (IS_OCTAL(lit[1]) ? 1 : 2); j < strlen(lit); j++) {
                if (lit[j] == '8' || lit[j] == '9') {
                    invalid = true;
                    break;
                }
            }
            if (!invalid) {
                write_ValueArray(&vm->constants, BUILD_ULONG(strtoull(lit + (IS_OCTAL(lit[1]) ? 1 : 2), NULL, 8)));
                isInt = false;
            }
        }
        // Check for binary
        else if (strlen(lit) > 2 && lit[0] == '0' && (lit[1] == 'b' || lit[1] == 'B')) {
            write_ValueArray(&vm->constants, BUILD_ULONG(strtoull(lit + 2, NULL, 2)));
            isInt = false;
        }
        // Check for float or double
        else {
            for (int j = 0; j < strlen(lit); j++) {
                if (lit[j] == '.' || lit[j] == 'F') {
                    if (lit[strlen(lit) - 1] == 'F') {
                        write_ValueArray(&vm->constants, BUILD_FLOAT(atof(lit)));
                    } else {
                        write_ValueArray(&vm->constants, BUILD_DOUBLE(atof(lit)));
                    }
                    isInt = false;
                    break;
                }
            }
        }
        // Default to unsigned long if it's an integer
        if (isInt) {
            write_ValueArray(&vm->constants, BUILD_ULONG(strtoull(lit, NULL, 10)));
        }
    }

    #undef SKIP_TOKEN
    #undef REFRESH_LIST

    return 0;
}
