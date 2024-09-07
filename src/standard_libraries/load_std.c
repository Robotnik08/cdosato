#include "../../include/standard_libraries/load_std.h"

int loadStandardLibrary(VirtualMachine* vm) {
    // load the standard library
    DosatoFunctionMap functions[] = {
        {"SAY", io_say},
        {"SAYLN", io_sayln},
        {"LISTEN", io_listen}
    };

    for (int i = 0; i < sizeof(functions) / sizeof(DosatoFunctionMap); i++) {
        Function func;
        init_Function(&func);
        func.name = functions[i].name;
        func.func_ptr = functions[i].function;
        func.is_compiled = true;

        // check if name is in name map
        size_t name_index = -1;
        for (int j = 0; j < vm->mappings.count; j++) {
            if (strcmp(vm->mappings.names[j], func.name) == 0) {
                name_index = j;
                break;
            }
        }
        // if not, add it
        if (name_index == -1) {
            name_index = vm->mappings.count;
            char* name = strdup(func.name);
            write_NameMap(&vm->mappings, name);
        }
        func.name_index = name_index;

        write_FunctionList(&vm->functions, func);
    }
}