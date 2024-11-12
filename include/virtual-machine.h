#ifndef dosato_virtual_machine_h
#define dosato_virtual_machine_h

#include "common.h"
#include "code_instance.h"
#include "value.h"

#define GC_MIN_THRESHOLD 1024

typedef struct {
    CodeInstance* instance;
    char* name;
    size_t name_index;
    DataType return_type;
    size_t* argv;
    DataType* argt;
    size_t arity;
    uint8_t* ip;

    ValueArray* captured;
    size_t* captured_indices;
    size_t captured_count;

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
    NameMap constants_map;

    ErrorJumps error_jumps;

    CodeInstanceList includes;

    DosatoObject** allocated_objects;
    size_t allocated_objects_count;
    size_t allocated_objects_capacity;
    
} VirtualMachine;

extern VirtualMachine* main_vm;

#include "ast.h"

void initVirtualMachine(VirtualMachine* vm);
void freeVirtualMachine(VirtualMachine* vm);

void pushValue(ValueArray* array, Value value);

int runVirtualMachine(VirtualMachine* vm, int debug, bool is_main);

void init_FunctionList(FunctionList* list);
void write_FunctionList(FunctionList* list, Function func);
void free_FunctionList(FunctionList* list);

void destroy_Function(Function* func);
void destroy_FunctionList(FunctionList* list);

void init_Function(Function* func);

Value callExternalFunction(Function* function, ValueArray args, bool debug);

DosatoObject* buildDosatoObject(void* body, DataType type, bool sweep, void* vm);
void markObjects (VirtualMachine* vm);
void sweepObjects (VirtualMachine* vm);
void markValue(Value* value);
#define BUILD_STRING(value, trigger_sweep) (Value){ TYPE_STRING, .as.objectValue = buildDosatoObject(value, TYPE_STRING, trigger_sweep, main_vm), .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_ARRAY(value, trigger_sweep) (Value){ TYPE_ARRAY, .as.objectValue = buildDosatoObject(value, TYPE_ARRAY, trigger_sweep, main_vm), .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_OBJECT(value, trigger_sweep) (Value){ TYPE_OBJECT, .as.objectValue = buildDosatoObject(value, TYPE_OBJECT, trigger_sweep, main_vm), .defined = false, .is_variable_type = false, .is_constant = false }
#define BUILD_FUNCTION(value, trigger_sweep) (Value){ TYPE_FUNCTION, .as.objectValue = buildDosatoObject(value, TYPE_FUNCTION, trigger_sweep, main_vm), .defined = false, .is_variable_type = false, .is_constant = false }

#define NEXT_BYTE() (*vm->ip++)
#define NEXT_SHORT() ((*vm->ip++) | (*vm->ip++ << 8))
#define POP_VALUE() (vm->stack.values[--vm->stack.count])
#define PEEK_VALUE() (vm->stack.values[vm->stack.count - 1])
#define PEEK_VALUE_TWO() (vm->stack.values[vm->stack.count - 2])
#define POP_STACK() (vm->stack_frames.stack[--vm->stack_frames.count])
#define PUSH_STACK(frame) write_StackFrames(&vm->stack_frames, frame)
#define PEEK_STACK() (vm->stack_frames.stack[vm->stack_frames.count - 1])

#define RECURSION_LIMIT 1000

#define COPY_STRING(str) strcpy(malloc(strlen(str) + 1), str)

#endif // dosato_virtual_machine_h