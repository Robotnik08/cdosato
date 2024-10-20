#include "../include/common.h"

#include "../include/strtools.h"


char* getWord(const char* text, int start) {
    // Find the length of the word (number of alphanumeric characters)
    int wordLength = 0;
    for (int i = start; i < strlen(text); i++) {
        if (!IS_ALPHANUMERIC(text[i])) {
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

int toUpper (char* str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 32; // 32 is the difference between the ASCII values of 'A' and 'a'
        }
    }
    return 0;
}

int toLower (char* str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32; // 32 is the difference between the ASCII values of 'A' and 'a'
        }
    }
    return 0;
}