#ifndef dosato_compiler_h
#define dosato_compiler_h

#include "common.h"

#include "ast.h"
#include "virtual-machine.h"

void compile(VirtualMachine* vm, AST ast);
void compileNode (VirtualMachine* vm, Node node, TokenList tokens);

#endif // dosato_compiler_h