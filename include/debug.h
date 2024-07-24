#ifndef dosato_debug_h
#define dosato_debug_h

#include "common.h"
#include "code_instance.h"

void printInstruction(uint8_t* code, size_t offset, int line);
void disassembleCode(CodeInstance* instance, const char* name);

#endif // dosato_debug_h