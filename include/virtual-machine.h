#ifndef dosato_virtual_machine_h
#define dosato_virtual_machine_h

#include "common.h"
#include "code_instance.h"
#include "value.h"

typedef struct {
    CodeInstance* instance;
    char* name;
    size_t name_index;
    DataType return_type;
    size_t* argv;
    DataType* argt;
    size_t arity;
    uint8_t* ip;

    bool is_compiled; // compiled or imported by external library
    void* func_ptr;
} Function;

typedef struct {
    Function* funcs;
    size_t count;
    size_t capacity;
} FunctionList;

typedef struct {
    CodeInstance* instance;
    ValueArray constants;
    ValueArray globals;
    
    uint8_t* ip;

    FunctionList functions;

    StackFrames stack_frames;
    ValueArray stack;
    
    NameMap mappings;

    ErrorJumps error_jumps;
} VirtualMachine;

#include "ast.h"

void initVirtualMachine(VirtualMachine* vm);
void freeVirtualMachine(VirtualMachine* vm);

void pushValue(ValueArray* array, Value value);

int runVirtualMachine(VirtualMachine* vm, int debug, AST ast);

void init_FunctionList(FunctionList* list);
void write_FunctionList(FunctionList* list, Function func);
void free_FunctionList(FunctionList* list);

void destroy_Function(Function* func);
void destroy_FunctionList(FunctionList* list);

void init_Function(Function* func);


#define NEXT_BYTE() (*vm->ip++)
#define NEXT_SHORT() (*vm->ip++) | (*vm->ip++ << 8)
#define POP_VALUE() (vm->stack.values[--vm->stack.count])
#define PEEK_VALUE() (vm->stack.values[vm->stack.count - 1])
#define PEEK_VALUE_TWO() (vm->stack.values[vm->stack.count - 2])
#define POP_STACK() (vm->stack_frames.stack[--vm->stack_frames.count])
#define PUSH_STACK(frame) write_StackFrames(&vm->stack_frames, frame)
#define PEEK_STACK() (vm->stack_frames.stack[vm->stack_frames.count - 1])

#define RECURSION_LIMIT 1000

#endif // dosato_virtual_machine_h