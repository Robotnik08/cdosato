#include "../../include/standard_libraries/std_object.h"

Value object_keys(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }
    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_OBJECT) {
        return BUILD_EXCEPTION(E_NOT_AN_OBJECT);
    }

    ValueObject* obj = AS_OBJECT(arg);
    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = 0; i < obj->count; i++) {
        write_ValueArray(new_array, obj->keys[i]);
    }

    return BUILD_ARRAY(new_array, true);
}

Value object_values(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }
    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_OBJECT) {
        return BUILD_EXCEPTION(E_NOT_AN_OBJECT);
    }

    ValueObject* obj = AS_OBJECT(arg);
    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = 0; i < obj->count; i++) {
        write_ValueArray(new_array, obj->values[i]);
    }

    return BUILD_ARRAY(new_array, true);
}

Value object_entries(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }
    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_OBJECT) {
        return BUILD_EXCEPTION(E_NOT_AN_OBJECT);
    }

    ValueObject* obj = AS_OBJECT(arg);
    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = 0; i < obj->count; i++) {
        ValueArray* entry = malloc(sizeof(ValueArray));
        init_ValueArray(entry);
        write_ValueArray(entry, obj->keys[i]);
        write_ValueArray(entry, obj->values[i]);
        write_ValueArray(new_array, BUILD_ARRAY(entry, false));
    }

    return BUILD_ARRAY(new_array, true);
}