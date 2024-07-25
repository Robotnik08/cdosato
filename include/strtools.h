#ifndef dosato_strtools_h
#define dosato_strtools_h

#include "common.h"

/**
 * @brief Check if a character is numeric
 * @param c The character to check
*/
int isNumeric (char c);
/**
 * @brief Check if a character is alphabetic
 * @param c The character to check
*/
int isAlpha (char c);
/**
 * @brief Check if a character is a space, new line, or tab, or even a \0
 * @param c The character to check
*/
int isTrueEmpty (char c);
/**
 * @brief Check if a character is empty
 * @param c The character to check
*/
int isEmpty (char c);
/**
 * @brief Check if a character is alphanumeric
 * @param c The character to check
*/
int isAlphaNumeric (char c);
/**
 * @brief Check if a character is Alpha or has an underscore
 * @param c The character to check
*/
int isAlphaNameric (char c);
/**
 * @brief Check if a character is a number or a period
 * @param c The character to check
*/
int isFloateric (char c);
/**
 * @brief Split a string into an array of strings, make sure to free the result
 * @param input The string to split
 * @param separator The separator to split by
*/
char** strspl(const char* input, const char* separator);
/**
 * @brief Replace all instances of a string with another string
 * @param in The string to replace in
 * @param selector The string to replace
 * @param replacement The string to replace with
 * @return The amount of times the string was replaced
*/
int strrep(char *in, const char *selector, const char *replacement);
/**
 * @brief Get the Next Word in a String
 * @param text The text to get the word from
 * @param start The index to start at
 * @return The next word in the string (don't forget to free it)
*/
char* getWord (const char* text, int start);
/**
 * @brief Get the line number of a character in a string
 * @param text The text to get the line number from
 * @param pos The index of the character to get the line number of
*/
int getLine (const char* text, int pos);
/**
 * @brief Get the column number of a character in a string
 * @param text The text to get the column number from
 * @param pos The index of the character to get the column number of
*/
int getLineCol (const char* text, int pos);

/**
 * @brief Remove the first and last character of a string
 * @param test The string to remove the first and last character of
 * @param amount The amount of characters to remove
 * @return The new string (don't forget to free it)
*/
char* removeLastAndFirstChar(const char* str, int amount);

/**
 * @brief Check if a string starts and ends with a character
 * @param str The string to check
 * @param character The character to check
 * @warning The buffer is 1024 x the size of the string, so don't use this for large strings or large replacements
*/
int strsur (const char* str, const char character);

/**
 * @brief Count the amount of times a character appears in a string
 * @param str The string to check
 * @param character The character to check
 * @return The amount of times the character appears in the string
*/
int strchl (const char* str, const char character);

#endif