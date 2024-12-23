#ifndef dosato_compiler_h
#define dosato_compiler_h

#include "common.h"

#include "ast.h"
#include "virtual-machine.h"

#define MAX_INCLUDES INT16_MAX >> 4

typedef struct {
    size_t* locals_lookup;
    size_t locals_count;
    size_t locals_capacity;
    DataType return_type;
    bool is_class;
} ScopeData;

void compile(VirtualMachine* vm, AST* ast);
int compileNode (VirtualMachine* vm, CodeInstance* ci, Node node, AST* ast, ScopeData* scope);

int writeOperatorInstruction (CodeInstance* ci, OperatorType operator, size_t token_index);
int writeUnaryInstruction (CodeInstance* ci, OperatorType operator, size_t token_index);

void initScopeData(ScopeData* scope);
void pushScopeData(ScopeData* scope, size_t index);
size_t popScopeData(ScopeData* scope);
void freeScopeData(ScopeData* scope);

bool inScope(ScopeData* scope, size_t index);
size_t getScopeIndex(ScopeData* scope, size_t index);

#endif // dosato_compiler_h