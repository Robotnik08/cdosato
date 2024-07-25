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


int tokenise (TokenList* list, char* full_code, const int code_length) {
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
    for (int i = 0; i < code_length; i++) {
        if (!quotationtype) {
            if ((full_code[i] == '"' || full_code[i] == '\'') && escapeCount % 2 == 0) {
                quotationtype = full_code[i];
                start = i;
                continue;
            }
            if (full_code[i] == '/' && full_code[i + 1 < code_length ? i + 1 : 0] == '/') {
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
        } else {
            if (full_code[i] == quotationtype && escapeCount % 2 == 0) {
                end = i;
                DOSATO_ADD_TOKEN(list, TOKEN_STRING, full_code + start, end - start, 0);
                start = end + 1;
                quotationtype = '\0';
            }
        }

        // Check for escape character
        if (full_code[i] == '\\') {
            escapeCount++;
        } else {
            escapeCount = 0;
        }
    }
    
    REFRESH_LIST()
    
    // get master tokens
    const char* mastertokens[] = MASTER_KEYWORDS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(mastertokens)/sizeof(char*); j++) {
            // check if the previous char is not alphanumeric
            if (i > 0) {
                if (isAlphaNumeric(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(mastertokens[j]) < code_length) {
                    if (isAlphaNameric(full_code[i + strlen(mastertokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
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
                if (isAlphaNumeric(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(var_typetokens[j]) < code_length) {
                    if (isAlphaNameric(full_code[i + strlen(var_typetokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
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
                if (isAlphaNumeric(full_code[i-1])) break;
                // check if the next char is not alphanameric
                if (i + strlen(extension_tokens[j]) < code_length) {
                    if (isAlphaNameric(full_code[i + strlen(extension_tokens[j])])) continue; // not a break, since the next type could differ in length
                }
            }
            char* next_word = getWord(full_code, i);
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
                DOSATO_ADD_TOKEN(list, TOKEN_PARENTHESIS, full_code + i, 0, getBracketType(full_code[i]) | bracketTier);
                bracketTypeHiarcy[bracketTier - 1] = j;
                break;
            }
        }
        // check for closing brackets
        for (int j = 0; j < sizeof(brackettokens)/sizeof(char*); j++) {
            if (full_code[i] == brackettokens[j][1]) {
                DOSATO_ADD_TOKEN(list, TOKEN_PARENTHESIS, full_code + i, 0, getBracketType(full_code[i]) | (bracketTypeHiarcy[bracketTier - 1] == j ? bracketTier : -1));
                if (bracketTypeHiarcy[bracketTier - 1] == j) {
                    bracketTypeHiarcy[bracketTier - 1] = -1;
                    bracketTier--;
                }
                break;
            }
        }
    }

    REFRESH_LIST()

    // get separator tokens
    const char separatortokens[] = SEPARATORS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        for (int j = 0; j < sizeof(separatortokens); j++) {
            if (full_code[i] == separatortokens[j]) {
                DOSATO_ADD_TOKEN(list, TOKEN_SEPARATOR, full_code + i, 0, j);
                break;
            }
        }
    }
    
    REFRESH_LIST()

    // getNumber tokens
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        if (isFloateric(full_code[i])) {
            if (!(isNumeric(full_code[i]) || full_code[i] == '.') || isAlphaNameric(full_code[i-1]) || isAlphaNameric(full_code[i+1])) {
                if (isAlphaNameric(full_code[i-1]) && full_code[i] == '.') return 4;
                for (int k = i; k < code_length && isFloateric(full_code[i]); k++) {
                    i = k;
                }
                continue;
            }
            bool foundDot = false;
            int start = i;
            int end = i;

            bool invalid = false;
            for (int j = i; j < code_length; j++) {
                if (isAlphaNameric(full_code[j-1]) && !isFloateric(full_code[j])) {
                    invalid = true;
                    for (int k = j; k < code_length && isFloateric(full_code[k]); k++) {
                        end = k;
                    }
                    return E_INVALID_LITERAL;
                }
                if (full_code[j] == '.') {
                    if (foundDot) {
                        invalid = true;
                        for (int k = j; k < code_length && isFloateric(full_code[k]); k++) {
                            end = k;
                        }
                        return E_INVALID_LITERAL;
                    }
                    foundDot = true;
                    if (!isNumeric(full_code[j+1])) {
                        invalid = true;
                        for (int k = j; k < code_length && isFloateric(full_code[k]); k++) {
                            end = k;
                        }
                        return E_INVALID_LITERAL;
                    }
                }
                if (isFloateric(full_code[j])) {
                    if (full_code[j] == 'F') {
                        if (isAlphaNumeric(full_code[j+1])) {
                            invalid = true;
                            for (int k = j; k < code_length && isFloateric(full_code[k]); k++) {
                                end = k;
                            }
                            return E_INVALID_LITERAL;
                        }
                        if (isNumeric(full_code[j-1])) {
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
                DOSATO_ADD_TOKEN(list, TOKEN_NUMBER, full_code + start, end - start, 0);
            }
            i = end;
        }
    }
    
    REFRESH_LIST()

    // get operator tokens
    const char* operatortokens[] = OPERATORS;
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        bool foundHuge = false;
        for (int j = 0; j < sizeof(operatortokens)/sizeof(char*); j++) {
            if (i + 2 >= code_length) break;
            char hugeoperator[4] = { full_code[i], full_code[i+1 < code_length ? i+1 : 0], full_code[i+2 < code_length ? i+2 : 0], '\0' };
            if (!strcmp(hugeoperator, operatortokens[j])) {
                DOSATO_ADD_TOKEN(list, TOKEN_OPERATOR, full_code + i, strlen(operatortokens[j]), j);
                i += 2;
                foundHuge = true;
                break;
            }
        }
        if (foundHuge) continue;
        bool foundBig = false;
        for (int j = 0; j < sizeof(operatortokens)/sizeof(char*); j++) {
            if (i + 1 >= code_length) break;
            char bigoperator[3] = { full_code[i], full_code[i+1 < code_length ? i+1 : 0], '\0' };
            if (!strcmp(bigoperator, operatortokens[j])) {
                DOSATO_ADD_TOKEN(list, TOKEN_OPERATOR, full_code + i, strlen(operatortokens[j]), j);
                i++;
                foundBig = true;
                break;
            }
        }
        if (foundBig) continue;
        for (int j = 0; j < sizeof(operatortokens)/sizeof(char*); j++) {
            if (full_code[i] == operatortokens[j][0]) {
                DOSATO_ADD_TOKEN(list, TOKEN_OPERATOR, full_code + i, strlen(operatortokens[j]), j);
                break;
            }
        }
    }

    REFRESH_LIST()
    
    // get identifier tokens (variables, functions, etc.)
    for (int i = 0; i < code_length; i++) {
        SKIP_TOKEN()
        if (isAlphaNumeric(full_code[i])) {
            char* word = getWord(full_code, i);
            if (strlen(word) > 0) {
                DOSATO_ADD_TOKEN(list, TOKEN_IDENTIFIER, full_code + i, strlen(word) - 1, 0);
                i += strlen(word) - 1;
            }
            free(word);
        }
    }

    // finalise tokens
    REFRESH_LIST()
    trimComments(list); // comments will be ignored from now on

    #undef SKIP_TOKEN
    #undef REFRESH_LIST

    return 0;
}
