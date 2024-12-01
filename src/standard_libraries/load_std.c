#include "../../include/standard_libraries/load_std.h"

int loadStandardLibrary(VirtualMachine* vm) {
    // load the standard library
    DosatoFunctionMap functions[] = {
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
        {"createDirectory", io_create_directory},
        {"deleteDirectory", io_delete_directory},
        {"directoryExists", io_directory_exists},
        {"getFiles", io_get_files_directory},

        {"clear", io_clear},
        {"end", io_end},
        {"pause", io_pause},
        {"system", io_system},

        {"seedRandom", io_seed_random},
        {"random", io_random_double},
        {"randomInt", io_random_int},
        {"randomRange", io_random_range},
        {"randomBool", io_random_bool},

        {"time", io_time},
        {"sleep", io_sleep},
        {"dateTime", io_datetime},
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

        {"stringSplit", string_split},
        {"stringLower", string_lower},
        {"stringUpper", string_upper},
        {"stringLength", string_length},
        {"substring", string_substr},
        {"stringIndexOf", string_indexof},
        {"stringLastIndexOf", string_lastindexof},
        {"stringStartsWith", string_startswith},
        {"stringEndsWith", string_endswith},
        {"stringReplace", string_replace},
        {"stringTrim", string_trim},
        {"stringReverse", string_reverse},
        {"stringContains", string_contains},
        {"stringRemove", string_remove},
        {"stringInsert", string_insert},
        {"stringToInt", string_atoi},
        {"stringToDouble", string_atod},
        {"stringCount", string_count},
        {"stringJoin", string_join},
        {"stringFormat", string_format},
        {"toString", string_to_string},

        {"sort", array_sort},
        {"push", array_push},
        {"pop", array_pop},
        {"shift", array_shift},
        {"unshift", array_unshift},
        {"slice", array_slice},
        {"splice", array_splice},
        {"indexOf", array_index_of},
        {"lastIndexOf", array_last_index_of},
        {"reverse", array_reverse},
        {"fill", array_fill},
        {"range", array_range},
        {"rangef", array_rangef},
        {"counter", array_counter},
        {"setCounter", array_set_counter},
        {"map", array_map},
        {"reduce", array_reduce},
        {"some", array_some},
        {"filter", array_filter},
        {"every", array_every},
        {"count", array_count},
        {"sum", array_sum},

        {"typeOf", type_typeof},
        {"isNull", type_isnull},
        {"typeIntOf", type_typeof_int},
        {"isNaN", type_isnan},

        {"throw", type_throw},

        {"keys", object_keys},
        {"values", object_values},
        {"entries", object_entries},
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