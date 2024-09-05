#ifndef dosato_lexer_h
#define dosato_lexer_h

#include "common.h"
#include "token.h"
#include "virtual-machine.h"

int tokenise (TokenList* list, char* full_code, const int code_length, VirtualMachine* vm, const char* file_name);

void sortTokens (TokenList* list);

void trimComments (TokenList* list);


#endif // dosato_lexer_h