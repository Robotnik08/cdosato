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
    for (int i = 0; i < obj->size; i++) {
        if (!obj->entries[i].is_used) continue; // no hash
        ValueObjectHashEntry entry = obj->entries[i];
        while (true) {
            write_ValueArray(new_array, entry.keyValue);

            NEXT_ENTRY(entry);
        }
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
    for (int i = 0; i < obj->size; i++) {
        if (!obj->entries[i].is_used) continue; // no hash
        ValueObjectHashEntry entry = obj->entries[i];
        while (true) {
            write_ValueArray(new_array, entry.value);

            NEXT_ENTRY(entry);
        }
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
    for (int i = 0; i < obj->size; i++) {
        if (!obj->entries[i].is_used) continue; // no hash
        ValueObjectHashEntry entry = obj->entries[i];
        while (true) {
            ValueArray* array_entry = malloc(sizeof(ValueArray));
            init_ValueArray(array_entry);
            write_ValueArray(array_entry, entry.keyValue);
            write_ValueArray(array_entry, entry.value);
            write_ValueArray(new_array, BUILD_ARRAY(array_entry, false));
            
            NEXT_ENTRY(entry);
        }
    }

    return BUILD_ARRAY(new_array, true);
}

Value object_getHash(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    uint64_t hash = hashValue(GET_ARG(args, 0));

    return BUILD_ULONG(hash);
}