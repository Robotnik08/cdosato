#ifndef dosato_strtools_h
#define dosato_strtools_h

#include "common.h"

#define IS_SPACE(c) (c == ' ' || c == '\n' || c == '\t' || c == '\0')
#define IS_NUMERIC(c) (c >= '0' && c <= '9')
#define IS_ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define IS_ALPHANUMERIC(c) (IS_NUMERIC(c) || IS_ALPHA(c) || c == '_')
#define IS_ALPHANAMERIC(c) (IS_ALPHA(c) || c == '_')
#define IS_HEXADECIMAL(c) (IS_NUMERIC(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
#define IS_OCTAL(c) (c >= '0' && c <= '7')
#define IS_BINARY(c) (c == '0' || c == '1')
#define IS_FLOATERIC(c) (IS_NUMERIC(c) || c == '.' || c == 'F')



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
 * @brief Convert a string to uppercase
 * @param str The string to convert
*/
int toUpper (char* str);

/**
 * @brief Convert a string to lowercase
 * @param str The string to convert
*/
int toLower (char* str);

#endif