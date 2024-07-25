#include "../include/common.h"

#include "../include/strtools.h"


int isNumeric (char c) {
    return c >= '0' && c <= '9';
}

int isAlpha (char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isAlphaNumeric (char c) {
    return isNumeric(c) || isAlpha(c) || c == '_';
}

int isAlphaNameric (char c) {
    return isAlpha(c) || c == '_';
}

int isFloateric (char c) {
    return isNumeric(c) || c == '.' || c == 'F';
}

int isTrueEmpty (char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\0';
}

int isEmpty (char c) {
    return c = ' ' || c == '\t' || c == '\n' || c == '\r';
}

char* itos (long long int i) {
    char* str = malloc(sizeof(char) * 30);
    sprintf(str, "%lld", i);
    return str;
}

char* uitos (unsigned long long int i) {
    char* str = malloc(sizeof(char) * 30);
    sprintf(str, "%llu", i);
    return str;
}

char* ftos (double f) {
    char* str = malloc(sizeof(char) * 30);
    sprintf(str, "%.17g", f);
    return str;
}

char** strspl(const char* input, const char* separator) {
    int count = 1;
    const char* temp = input;
    // Count the number of times the separator appears in the input string
    while ((temp = strstr(temp, separator))) {
        count++;
        temp += strlen(separator);
    }
    // Allocate memory for the substrings
    char** substrings = (char**)malloc((count + 1) * sizeof(char*));

    const char* start = input;
    int i = 0;

    // Loop through the input string, splitting it by the separator
    while (1) {
        const char* end = strstr(start, separator);
        if (end == NULL) {
            substrings[i] = (char*)malloc(strlen(start) + 1);
            strcpy(substrings[i], start);
            break;
        }

        int length = end - start;
        substrings[i] = (char*)malloc(length + 1);
        strncpy(substrings[i], start, length);
        substrings[i][length] = '\0';

        start = end + strlen(separator);
        i++;
    }
    // Add a null terminator to the end of the array
    substrings[count] = NULL;

    return substrings;
}

int strrep(char *in, const char *selector, const char *replacement) {
    int in_len = strlen(in);
    int selector_len = strlen(selector);

    int amount = 0;

    char *result = (char *)malloc(in_len * 1024);

    result[0] = '\0';

    for (int i = 0; i < in_len; ) {
        if (strncmp(in + i, selector, selector_len) == 0) {
            strcat(result, replacement);
            i += selector_len;
            amount++;
        } else {
            strncat(result, in + i, 1);
            i++;
        }
    }

    strcpy(in, result);

    free(result);

    return amount;
}

char* getWord(const char* text, int start) {
    // Find the length of the word (number of alphanumeric characters)
    int wordLength = 0;
    for (int i = start; i < strlen(text); i++) {
        if (!isAlphaNumeric(text[i])) {
            break;
        }
        wordLength++;
    }

    // Allocate memory for the word (including space for the null terminator)
    char* word = (char*)malloc((wordLength + 1) * sizeof(char));

    // Copy the characters of the word from the input text
    for (int i = 0; i < wordLength; i++) {
        word[i] = text[start + i];
    }

    // Add the null terminator to make it a valid C string
    word[wordLength] = '\0';
    return word;
}

int getLine (const char* text, int pos) {
    int line = 1;
    for (int i = 0; i < pos; i++) {
        if (text[i] == '\n') {
            line++;
        }
    }
    return line;
}

int getLineCol (const char* text, int pos) {
    int col = 1;
    for (int i = 0; i < pos; i++) {
        if (text[i] == '\n') {
            col = 0;
        }
        col++;
    }
    return col;
}

char* removeLastAndFirstChar (const char* str, int amount) {
    char* new_str = malloc(sizeof(char) * (strlen(str) - amount*2 + 1));
    for (int i = 0; i < strlen(str) - amount*2; i++) {
        new_str[i] = str[i + amount];
    }
    new_str[strlen(str) - amount-1] = '\0';

    return new_str;
}

int strsur (const char* str, const char character) {
    return str[0] == character && str[strlen(str) - 1] == character;
}

int strchl (const char* str, const char character) {
    int count = 0;
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == character) {
            count++;
        }
    }
    return count;
}