#include "../../include/standard_libraries/load_std.h"

int loadStandardLibrary(VirtualMachine* vm) {
    // load the standard library
    DosatoFunctionMap functions[] = {
        {"SAY", io_say},
        {"SAYLN", io_sayln},
        {"LISTEN", io_listen},
        {"READ", io_read_file},
        {"WRITE", io_write_file},
        {"APPEND", io_append_file},
        {"DELETE", io_delete_file},
        {"EXISTS", io_file_exists},
        {"MOVE", io_move_file},
        {"COPY", io_copy_file},

        {"CLEAR", io_clear},
        {"END", io_end},
        {"PAUSE", io_pause},
        {"SYSTEM", io_system},

        {"SRANDOM", io_seed_random},
        {"RANDOM", io_random_double},
        {"RANDOMINT", io_random_int},
        {"RANDOMRANGE", io_random_range},
        {"RANDOMBOOL", io_random_bool},

        {"TIME", io_time},
        {"SLEEP", io_sleep},
        {"DATETIME", io_datetime},
        {"CLOCK", io_clock},

        {"ABS", math_abs},
        {"ROUND", math_round},
        {"FLOOR", math_floor},
        {"CEIL", math_ceil},
        {"POW", math_pow},
        {"SQRT", math_sqrt},
        {"MIN", math_min},
        {"MAX", math_max},
        {"LOG", math_log},
        {"LOG10", math_log10},
        {"LOG2", math_log2},
        {"EXP", math_exp},
        {"SIN", math_sin},
        {"COS", math_cos},
        {"TAN", math_tan},
        {"ASIN", math_asin},
        {"ACOS", math_acos},
        {"ATAN", math_atan},
        {"ATAN2", math_atan2},
        {"CLAMP", math_clamp},

        {"SPLIT", string_split},
        {"LOWER", string_lower},
        {"UPPER", string_upper},
        {"LENGTH", string_length},
        {"SUBSTR", string_substr},
        {"INDEXOF", string_indexof},
        {"LASTINDEXOF", string_lastindexof},
        {"STARTSWITH", string_startswith},
        {"ENDSWITH", string_endswith},
        {"REPLACE", string_replace},
        {"TRIM", string_trim},
        {"REVERSE", string_reverse},
        {"CONTAINS", string_contains},
        {"REMOVE", string_remove},
        {"INSERT", string_insert},
        {"STRINGTOINT", string_atoi},
        {"STRINGTODOUBLE", string_atod},
        {"COUNT", string_count},

        {"SORT", array_sort}
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
            addName(&vm->mappings, func.name);
        }
        func.name_index = name_index;

        write_FunctionList(&vm->functions, func);
    }
}