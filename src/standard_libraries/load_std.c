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
        {"STRINGLENGTH", string_length},
        {"SUBSTRING", string_substr},
        {"STRINGINDEXOF", string_indexof},
        {"STRINGLASTINDEXOF", string_lastindexof},
        {"STARTSWITH", string_startswith},
        {"ENDSWITH", string_endswith},
        {"REPLACE", string_replace},
        {"TRIM", string_trim},
        {"STRINGREVERSE", string_reverse},
        {"STRINGCONTAINS", string_contains},
        {"STRINGREMOVE", string_remove},
        {"STRINGINSERT", string_insert},
        {"STRINGTOINT", string_atoi},
        {"STRINGTODOUBLE", string_atod},
        {"STRINGCOUNT", string_count},
        {"JOIN", string_join},
        {"FORMAT", string_format},

        {"SORT", array_sort},
        {"PUSH", array_push},
        {"POP", array_pop},
        {"SHIFT", array_shift},
        {"UNSHIFT", array_unshift},
        {"SLICE", array_slice},
        {"SPLICE", array_splice},
        {"INDEXOF", array_index_of},
        {"LASTINDEXOF", array_last_index_of},
        {"REVERSE", array_reverse},
        {"FILL", array_fill},
        {"RANGE", array_range},
        {"RANGEF", array_rangef},
        {"COUNTER", array_counter},
        {"SETCOUNTER", array_set_counter},

        {"TYPEOF", type_typeof},
        {"ISNULL", type_isnull},
        {"TYPEINTOF", type_typeof_int},

        // no caps functions below (refrence the same functions)
        {"say", io_say},
        {"sayln", io_sayln},
        {"listen", io_listen},
        {"read", io_read_file},
        {"write", io_write_file},
        {"append", io_append_file},
        {"delete", io_delete_file},
        {"exists", io_file_exists},
        {"move", io_move_file},
        {"copy", io_copy_file},

        {"clear", io_clear},
        {"end", io_end},
        {"pause", io_pause},
        {"system", io_system},

        {"srandom", io_seed_random},
        {"random", io_random_double},
        {"randomint", io_random_int},
        {"randomrange", io_random_range},
        {"randombool", io_random_bool},

        {"time", io_time},
        {"sleep", io_sleep},
        {"datetime", io_datetime},
        {"clock", io_clock},

        {"abs", math_abs},
        {"round", math_round},
        {"floor", math_floor},
        {"ceil", math_ceil},
        {"pow", math_pow},
        {"sqrt", math_sqrt},
        {"min", math_min},
        {"max", math_max},
        {"log", math_log},
        {"log10", math_log10},
        {"log2", math_log2},
        {"exp", math_exp},
        {"sin", math_sin},
        {"cos", math_cos},
        {"tan", math_tan},
        {"asin", math_asin},
        {"acos", math_acos},
        {"atan", math_atan},
        {"atan2", math_atan2},
        {"clamp", math_clamp},

        {"split", string_split},
        {"lower", string_lower},
        {"upper", string_upper},
        {"stringlength", string_length},
        {"substring", string_substr},
        {"stringindexof", string_indexof},
        {"stringlastindexof", string_lastindexof},
        {"startswith", string_startswith},
        {"endswith", string_endswith},
        {"replace", string_replace},
        {"trim", string_trim},
        {"stringreverse", string_reverse},
        {"stringcontains", string_contains},
        {"stringremove", string_remove},
        {"stringinsert", string_insert},
        {"stringtoint", string_atoi},
        {"stringtodouble", string_atod},
        {"stringcount", string_count},
        {"join", string_join},
        {"format", string_format},

        {"sort", array_sort},
        {"push", array_push},
        {"pop", array_pop},
        {"shift", array_shift},
        {"unshift", array_unshift},
        {"slice", array_slice},
        {"splice", array_splice},
        {"indexof", array_index_of},
        {"lastindexof", array_last_index_of},
        {"reverse", array_reverse},
        {"fill", array_fill},
        {"range", array_range},
        {"rangef", array_rangef},
        {"counter", array_counter},
        {"setcounter", array_set_counter},

        {"typeof", type_typeof},
        {"isnull", type_isnull},
        {"typeintof", type_typeof_int}
    };

    for (int i = 0; i < sizeof(functions) / sizeof(DosatoFunctionMap); i++) {
        Function func;
        init_Function(&func);
        char* name = malloc(strlen(functions[i].name) + 1);
        strcpy(name, functions[i].name);
        func.name = name;
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

    return 0;
}

int loadConstants (VirtualMachine* vm, char** argv, int argc) {

    #define ADD_CONST(name, value) do { \
        size_t name_index = -1; \
        for (int j = 0; j < vm->mappings.count; j++) { \
            if (strcmp(vm->mappings.names[j], name) == 0) { \
                name_index = j; \
                break; \
            } \
        } \
        if (name_index == -1) { \
            name_index = vm->mappings.count; \
            addName(&vm->mappings, name); \
            write_ValueArray(&vm->globals, value); \
        } else { \
            vm->globals.values[name_index] = value; \
        } \
    } while(0)


    // add the standard library constants

    Value maxlong = (Value){ TYPE_LONG, .as.longValue = 9223372036854775807LL, .defined = true, .is_variable_type = false, .is_constant = true };
    Value minlong = (Value){ TYPE_LONG, .as.longValue = -9223372036854775807LL - 1, .defined = true, .is_variable_type = false, .is_constant = true };
    Value maxint = (Value){ TYPE_LONG, .as.longValue = 2147483647, .defined = true, .is_variable_type = false, .is_constant = true };
    Value minint = (Value){ TYPE_LONG, .as.longValue = -2147483648, .defined = true, .is_variable_type = false, .is_constant = true };
    Value maxshort = (Value){ TYPE_LONG, .as.longValue = 32767, .defined = true, .is_variable_type = false, .is_constant = true };
    Value minshort = (Value){ TYPE_LONG, .as.longValue = -32768, .defined = true, .is_variable_type = false, .is_constant = true };
    Value maxchar = (Value){ TYPE_LONG, .as.longValue = 127, .defined = true, .is_variable_type = false, .is_constant = true };
    Value minchar = (Value){ TYPE_LONG, .as.longValue = -128, .defined = true, .is_variable_type = false, .is_constant = true };
    Value pi = (Value){ TYPE_DOUBLE, .as.doubleValue = 3.14159265358979323846, .defined = true, .is_variable_type = false, .is_constant = true };
    Value e = (Value){ TYPE_DOUBLE, .as.doubleValue = 2.71828182845904523536, .defined = true, .is_variable_type = false, .is_constant = true };

    Value argc_ = (Value){ TYPE_LONG, .as.longValue = argc, .defined = true, .is_variable_type = false, .is_constant = true };

    ValueArray* list = malloc(sizeof(ValueArray));
    init_ValueArray(list);
    for (int i = 0; i < argc; i++) {
        Value arg = BUILD_STRING(COPY_STRING(argv[i]), false);
        write_ValueArray(list, arg);
    }

    Value argv_ = BUILD_ARRAY(list, false);
    argv_.is_constant = true;
    argv_.defined = true;



    
    ADD_CONST("MAXLONG", maxlong);
    ADD_CONST("MINLONG", minlong);
    ADD_CONST("MAXINT", maxint);
    ADD_CONST("MININT", minint);
    ADD_CONST("MAXSHORT", maxshort);
    ADD_CONST("MINSHORT", minshort);
    ADD_CONST("MAXBYTE", maxchar);
    ADD_CONST("MINBYTE", minchar);
    ADD_CONST("MATH_PI", pi);
    ADD_CONST("MATH_E", e);

    ADD_CONST("ARGC", argc_);
    ADD_CONST("ARGV", argv_);

    // add lower case constants
    ADD_CONST("maxlong", maxlong);
    ADD_CONST("minlong", minlong);
    ADD_CONST("maxint", maxint);
    ADD_CONST("minint", minint);
    ADD_CONST("maxshort", maxshort);
    ADD_CONST("minshort", minshort);
    ADD_CONST("maxbyte", maxchar);
    ADD_CONST("minbyte", minchar);
    ADD_CONST("math_pi", pi);
    ADD_CONST("math_e", e);

    ADD_CONST("argc", argc_);
    ADD_CONST("argv", argv_);

    #undef constMapAdd

    return 0;
}