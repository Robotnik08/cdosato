#ifndef dosato_virtual_machine_h
#define dosato_virtual_machine_h

#include "common.h"
#include "code_instance.h"
#include "value.h"

typedef struct {
    CodeInstance* instance;
    ValueArray constants;
    ValueArray globals;
    
    uint8_t* ip;

    StackFrames stack_frames;
    ValueArray stack;
    
    NameMap mappings;
} VirtualMachine;
#include "ast.h"

void initVirtualMachine(VirtualMachine* vm);
void freeVirtualMachine(VirtualMachine* vm);

void pushValue(ValueArray* array, Value value);

int runVirtualMachine(VirtualMachine* vm, int debug, AST ast);


#define NEXT_BYTE() (*vm->ip++)
#define NEXT_SHORT() (*vm->ip++) | (*vm->ip++ << 8)
#define POP_VALUE() (vm->stack.values[--vm->stack.count])
#define POP_STACK() (vm->stack_frames.stack[--vm->stack_frames.count])

#endif // dosato_virtual_machine_h