#include "../../include/standard_libraries/std_array.h"

int compareValues (const void* a, const void* b) {
    Value a_value = *(Value*)a;
    Value b_value = *(Value*)b;

    // if either value is a string, array, object, or function, return 0 (cannot compare)
    if (a_value.type == TYPE_STRING || b_value.type == TYPE_STRING ||
        a_value.type == TYPE_ARRAY || b_value.type == TYPE_ARRAY ||
        a_value.type == TYPE_OBJECT || b_value.type == TYPE_OBJECT ||
        a_value.type == TYPE_FUNCTION || b_value.type == TYPE_FUNCTION) {
        return 0;
    }

    // if either value is double, cast both to double and compare
    if (a_value.type == TYPE_DOUBLE || b_value.type == TYPE_DOUBLE) {
        castValue(&a_value, TYPE_DOUBLE);
        castValue(&b_value, TYPE_DOUBLE);

        double diff = AS_DOUBLE(a_value) - AS_DOUBLE(b_value);
        if (diff < 0) return -1;
        if (diff > 0) return 1;
        return 0;
    }
    // if both values are integers, cast them to long and compare
    else if (ISINTTYPE(a_value.type) && ISINTTYPE(b_value.type)) {
        castValue(&a_value, TYPE_LONG);
        castValue(&b_value, TYPE_LONG);

        if (AS_LONG(a_value) < AS_LONG(b_value)) return -1;
        if (AS_LONG(a_value) > AS_LONG(b_value)) return 1;
        return 0;
    }
    // if types are incompatible, return 0
    else {
        return 0;
    }
}

Value dosato_quick_sort (ValueArray* array, int left, int right, size_t function) {
    if (left < right) {
        Value pivot_value = dosato_partition(array, left, right, function);
        if (pivot_value.type == TYPE_EXCEPTION || pivot_value.type == TYPE_HLT) {
            return pivot_value;
        }
        int pivot = AS_LONG(pivot_value);
        Value val1 = dosato_quick_sort(array, left, pivot - 1, function);
        if (val1.type == TYPE_EXCEPTION || val1.type == TYPE_HLT) {
            return val1;
        }
        Value val2 = dosato_quick_sort(array, pivot + 1, right, function);
        if (val2.type == TYPE_EXCEPTION || val2.type == TYPE_HLT) {
            return val2;
        }
    }
    return UNDEFINED_VALUE;
}

Value dosato_partition (ValueArray* array, int left, int right, size_t function) {
    Value pivot = array->values[right];
    int i = left - 1;

    for (int j = left; j < right; j++) {
        ValueArray args;
        init_ValueArray(&args);
        write_ValueArray(&args, array->values[j]);
        write_ValueArray(&args, pivot);
        Value result = callExternalFunction(function, args, false);
        free_ValueArray(&args);
        if (result.type == TYPE_EXCEPTION || result.type == TYPE_HLT) {
            return result;
        }

        if (!ISINTTYPE(result.type) && !ISFLOATTYPE(result.type)) {
            return BUILD_EXCEPTION(E_EXPECTED_NUMBER);
        }
        
        if (ISINTTYPE(result.type)) {
            castValue(&result, TYPE_LONG);
            if (AS_LONG(result) < 0) {
                i++;
                Value temp = array->values[i];
                array->values[i] = array->values[j];
                array->values[j] = temp;
            }
        }

        if (ISFLOATTYPE(result.type)) {
            castValue(&result, TYPE_DOUBLE);
            if (AS_DOUBLE(result) < 0) {
                i++;
                Value temp = array->values[i];
                array->values[i] = array->values[j];
                array->values[j] = temp;
            }
        }
        
    }

    Value temp = array->values[i + 1];
    array->values[i + 1] = array->values[right];
    array->values[right] = temp;

    return BUILD_LONG(i + 1);
}

Value array_sort(ValueArray args, bool debug) {
    if (args.count > 2 || args.count == 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    if (args.count == 2) {
        Value arg2 = GET_ARG(args, 1);
        if (arg2.type != TYPE_FUNCTION) {
            return BUILD_EXCEPTION(E_NOT_A_FUNCTION);
        }
        Value res = dosato_quick_sort(AS_ARRAY(arg), 0, AS_ARRAY(arg)->count - 1, AS_LONG(arg2));

        if (res.type == TYPE_EXCEPTION || res.type == TYPE_HLT) {
            return res;
        }
        return arg;
    }

    qsort(AS_ARRAY(arg)->values, AS_ARRAY(arg)->count, sizeof(Value), compareValues);

    return arg;
}

Value array_push (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    Value value = GET_ARG(args, 1);
    write_ValueArray(AS_ARRAY(arg), value);

    return arg;
}

Value array_pop (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    if ((AS_ARRAY(arg))->count == 0) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    return AS_ARRAY(arg)->values[AS_ARRAY(arg)->count-- - 1]; // return the popped value
}

Value array_shift (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    if ((AS_ARRAY(arg))->count == 0) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    ValueArray* obj = AS_ARRAY(arg);

    Value value = obj->values[0];
    
    obj->count--;
    for (int i = 0; i < obj->count; i++) {
        obj->values[i] = obj->values[i + 1];
    }

    return value;
}

Value array_unshift (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    Value value = GET_ARG(args, 1);
    ValueArray* obj = AS_ARRAY(arg);
    write_ValueArray(obj, UNDEFINED_VALUE); // make space for the new value

    for (int i = obj->count; i > 0; i--) {
        obj->values[i] = obj->values[i - 1];
    }

    obj->values[0] = value;

    return arg;
}

Value array_slice (ValueArray args, bool debug) {
    if (args.count < 2 || args.count > 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    Value start = GET_ARG(args, 1);
    CAST_SAFE(start, TYPE_LONG);

    Value end;
    if (args.count == 3) {
        end = GET_ARG(args, 2);
        CAST_SAFE(end, TYPE_LONG);
    } else {
        end = BUILD_LONG((AS_ARRAY(arg))->count);
    }

    if (AS_LONG(start) < 0 || AS_LONG(start) >= (AS_ARRAY(arg))->count || AS_LONG(end) < 0 || AS_LONG(end) > (AS_ARRAY(arg))->count) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    ValueArray* obj = AS_ARRAY(arg);
    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = AS_LONG(start); i < AS_LONG(end); i++) {
        write_ValueArray(new_array, obj->values[i]);
    }

    return BUILD_ARRAY(new_array, true);
}

Value array_splice (ValueArray args, bool debug) {
    if (args.count < 2 || args.count > 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    Value start = GET_ARG(args, 1);
    CAST_SAFE(start, TYPE_LONG);

    Value delete_count;
    if (args.count > 2) {
        delete_count = GET_ARG(args, 2);
        CAST_SAFE(delete_count, TYPE_LONG);
    } else {
        delete_count = BUILD_LONG((AS_ARRAY(arg))->count - AS_LONG(start));
    }

    ValueArray* obj = AS_ARRAY(arg);
    if (AS_LONG(start) < 0 || AS_LONG(start) >= obj->count || AS_LONG(delete_count) < 0 || AS_LONG(start) + AS_LONG(delete_count) > obj->count) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = 0; i < AS_LONG(start); i++) {
        write_ValueArray(new_array, obj->values[i]);
    }
    for (int i = AS_LONG(start) + AS_LONG(delete_count); i < obj->count; i++) {
        write_ValueArray(new_array, obj->values[i]);
    }

    return BUILD_ARRAY(new_array, true);
}

Value array_index_of (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    Value value = GET_ARG(args, 1);

    ValueArray* obj = AS_ARRAY(arg);
    for (int i = 0; i < obj->count; i++) {
        if (valueEquals(&obj->values[i], &value)) {
            return BUILD_LONG(i);
        }
    }

    return BUILD_LONG(-1);
}

Value array_last_index_of(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    Value value = GET_ARG(args, 1);

    ValueArray* obj = AS_ARRAY(arg);
    for (int i = obj->count - 1; i >= 0; i--) {
        if (valueEquals(&obj->values[i], &value)) {
            return BUILD_LONG(i);
        }
    }

    return BUILD_LONG(-1);
}

Value array_reverse (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    ValueArray* obj = AS_ARRAY(arg);
    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = obj->count - 1; i >= 0; i--) {
        write_ValueArray(new_array, obj->values[i]);
    }

    return BUILD_ARRAY(new_array, true);
}

Value array_fill (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value value = GET_ARG(args, 0);
    Value count = GET_ARG(args, 1);

    int cast_error = castValue(&count, TYPE_LONG);
    if (cast_error != E_NULL) {
        return BUILD_EXCEPTION(cast_error);
    }

    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = 0; i < AS_LONG(count); i++) {
        write_ValueArray(new_array, value);
    }

    return BUILD_ARRAY(new_array, true);
}


Value array_range (ValueArray args, bool debug) {
    if (args.count < 1 || args.count > 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value start = GET_ARG(args, 0);
    CAST_SAFE(start, TYPE_LONG);

    Value end;
    if (args.count > 1) {
        end = GET_ARG(args, 1);
        CAST_SAFE(end, TYPE_LONG);
    } else {
        end = BUILD_LONG(AS_LONG(start));
        start = BUILD_LONG(0);
    }

    Value step;
    if (args.count > 2) {
        step = GET_ARG(args, 2);
        CAST_SAFE(step, TYPE_LONG);
    } else {
        step = BUILD_LONG(1);
    }

    if (AS_LONG(step) == 0) {
        return BUILD_EXCEPTION(E_CANNOT_BE_ZERO);
    }

    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = AS_LONG(start); (AS_LONG(step) > 0 && i < AS_LONG(end)) || (AS_LONG(step) < 0 && i > AS_LONG(end)); i += AS_LONG(step)) {
        write_ValueArray(new_array, BUILD_LONG(i));
    }

    return BUILD_ARRAY(new_array, true);
}

Value array_rangef (ValueArray args, bool debug) {
    if (args.count < 1 || args.count > 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value start = GET_ARG(args, 0);
    CAST_SAFE(start, TYPE_DOUBLE);

    Value end;
    if (args.count > 1) {
        end = GET_ARG(args, 1);
        CAST_SAFE(end, TYPE_DOUBLE);
    } else {
        end = BUILD_DOUBLE(AS_DOUBLE(start));
        start = BUILD_DOUBLE(0);
    }

    Value step;
    if (args.count > 2) {
        step = GET_ARG(args, 2);
        CAST_SAFE(step, TYPE_DOUBLE);
    } else {
        step = BUILD_DOUBLE(1);
    }

    if (AS_DOUBLE(step) == 0) {
        return BUILD_EXCEPTION(E_CANNOT_BE_ZERO);
    }

    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (double i = AS_DOUBLE(start); (AS_DOUBLE(step) > 0 && i < AS_DOUBLE(end)) || (AS_DOUBLE(step) < 0 && i > AS_DOUBLE(end)); i += AS_DOUBLE(step)) {
        write_ValueArray(new_array, BUILD_DOUBLE(i));
    }

    return BUILD_ARRAY(new_array, true);
}

long long int counter = 0;

Value array_counter(ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    return BUILD_LONG(counter++);
}

Value array_set_counter(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_LONG);
    
    counter = AS_LONG(arg);

    return BUILD_LONG(counter++);
}