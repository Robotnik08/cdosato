#ifndef dosato_virtual_machine_h
#define dosato_virtual_machine_h

#include "common.h"
#include "code_instance.h"
#include "value.h"

typedef struct {
    CodeInstance* instance;
    ValueArray stack;
    uint8_t* ip;
} VirtualMachine;

void initVirtualMachine(VirtualMachine* vm);
void freeVirtualMachine(VirtualMachine* vm);

void pushValue(ValueArray* array, Value value);

int runVirtualMachine(VirtualMachine* vm);


#define NEXT_BYTE() (*vm->ip++)
#define NEXT_SHORT() (*vm->ip++) | (*vm->ip++ << 8)
#define POP_VALUE() (vm->stack.values[--vm->stack.count])

#endif // dosato_virtual_machine_h