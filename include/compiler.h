#ifndef dosato_compiler_h
#define dosato_compiler_h

#include "common.h"

#include "ast.h"
#include "virtual-machine.h"

void compile(VirtualMachine* vm, AST ast);

#endif // dosato_compiler_h