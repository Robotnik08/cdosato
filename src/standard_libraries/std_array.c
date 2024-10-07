#include "../../include/standard_libraries/std_array.h"

int compareValues (const void* a, const void* b) {
    Value a_value = *(Value*)a;
    Value b_value = *(Value*)b;
    if (a_value.type == TYPE_STRING || b_value.type == TYPE_STRING || a_value.type == TYPE_ARRAY || b_value.type == TYPE_ARRAY || a_value.type == TYPE_OBJECT || b_value.type == TYPE_OBJECT || a_value.type == TYPE_FUNCTION || b_value.type == TYPE_FUNCTION) {
        return 0;
    }

    if (a_value.type == TYPE_DOUBLE || b_value.type == TYPE_DOUBLE) {
        // a float to double cast always succeeds
        castValue(&a_value, TYPE_DOUBLE);
        castValue(&b_value, TYPE_DOUBLE);

        return a_value.as.doubleValue - b_value.as.doubleValue;
    } else if (ISINTTYPE(a_value.type) && ISINTTYPE(b_value.type)) {
        // an int to int cast always succeeds
        castValue(&a_value, TYPE_LONG);
        castValue(&b_value, TYPE_LONG);

        return a_value.as.longValue - b_value.as.longValue;
    } else {
        // if the types are not compatible, return 0
        return 0;
    }
}

Value dosato_quick_sort (ValueArray* array, int left, int right, size_t function) {
    if (left < right) {
        Value pivot_value = dosato_partition(array, left, right, function);
        if (pivot_value.type == TYPE_EXCEPTION || pivot_value.type == TYPE_HLT) {
            return pivot_value;
        }
        int pivot = pivot_value.as.longValue;
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
            if (result.as.longValue < 0) {
                i++;
                Value temp = array->values[i];
                array->values[i] = array->values[j];
                array->values[j] = temp;
            }
        }

        if (ISFLOATTYPE(result.type)) {
            castValue(&result, TYPE_DOUBLE);
            if (result.as.doubleValue < 0) {
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

    return (Value){ TYPE_LONG, .as.longValue = i + 1 };
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
        Value res = dosato_quick_sort(AS_ARRAY(arg), 0, AS_ARRAY(arg)->count - 1, arg2.as.longValue);

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

    return AS_ARRAY(arg)->values[AS_ARRAY(arg)->count--]; // return the popped value
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

    obj->count--;
    for (int i = 0; i < obj->count; i++) {
        obj->values[i] = obj->values[i + 1];
    }

    return arg;
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
    int cast_error = castValue(&start, TYPE_LONG);
    if (cast_error != E_NULL) {
        return BUILD_EXCEPTION(cast_error);
    }

    Value end;
    if (args.count == 3) {
        end = GET_ARG(args, 2);
        cast_error = castValue(&end, TYPE_LONG);
        if (cast_error != E_NULL) {
            return BUILD_EXCEPTION(cast_error);
        }
    } else {
        end = (Value){ TYPE_LONG, .as.longValue = (AS_ARRAY(arg))->count };
    }

    if (start.as.longValue < 0 || start.as.longValue >= (AS_ARRAY(arg))->count || end.as.longValue < 0 || end.as.longValue > (AS_ARRAY(arg))->count) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    ValueArray* obj = AS_ARRAY(arg);
    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = start.as.longValue; i < end.as.longValue; i++) {
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
    int cast_error = castValue(&start, TYPE_LONG);
    if (cast_error != E_NULL) {
        return BUILD_EXCEPTION(cast_error);
    }

    Value delete_count;
    if (args.count > 2) {
        delete_count = GET_ARG(args, 2);
        cast_error = castValue(&delete_count, TYPE_LONG);
        if (cast_error != E_NULL) {
            return BUILD_EXCEPTION(cast_error);
        }
    } else {
        delete_count = (Value){ TYPE_LONG, .as.longValue = (AS_ARRAY(arg))->count - start.as.longValue };
    }

    ValueArray* obj = AS_ARRAY(arg);
    if (start.as.longValue < 0 || start.as.longValue >= obj->count || delete_count.as.longValue < 0 || start.as.longValue + delete_count.as.longValue > obj->count) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = 0; i < start.as.longValue; i++) {
        write_ValueArray(new_array, obj->values[i]);
    }
    for (int i = start.as.longValue + delete_count.as.longValue; i < obj->count; i++) {
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
            return (Value){ TYPE_LONG, .as.longValue = i };
        }
    }

    return (Value){ TYPE_LONG, .as.longValue = -1 };
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
            return (Value){ TYPE_LONG, .as.longValue = i };
        }
    }

    return (Value){ TYPE_LONG, .as.longValue = -1 };
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
    for (int i = 0; i < count.as.longValue; i++) {
        write_ValueArray(new_array, value);
    }

    return BUILD_ARRAY(new_array, true);
}


Value array_range (ValueArray args, bool debug) {
    if (args.count < 1 || args.count > 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value start = GET_ARG(args, 0);
    int cast_error = castValue(&start, TYPE_LONG);
    if (cast_error != E_NULL) {
        return BUILD_EXCEPTION(cast_error);
    }

    Value end;
    if (args.count > 1) {
        end = GET_ARG(args, 1);
        cast_error = castValue(&end, TYPE_LONG);
        if (cast_error != E_NULL) {
            return BUILD_EXCEPTION(cast_error);
        }
    } else {
        end = (Value){ TYPE_LONG, .as.longValue = start.as.longValue };
        start = (Value){ TYPE_LONG, .as.longValue = 0 };
    }

    Value step;
    if (args.count > 2) {
        step = GET_ARG(args, 2);
        cast_error = castValue(&step, TYPE_LONG);
        if (cast_error != E_NULL) {
            return BUILD_EXCEPTION(cast_error);
        }
    } else {
        step = (Value){ TYPE_LONG, .as.longValue = 1 };
    }

    if (step.as.longValue == 0) {
        return BUILD_EXCEPTION(E_CANNOT_BE_ZERO);
    }

    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = start.as.longValue; (step.as.longValue > 0 && i < end.as.longValue) || (step.as.longValue < 0 && i > end.as.longValue); i += step.as.longValue) {
        write_ValueArray(new_array, (Value){ TYPE_LONG, .as.longValue = i });
    }

    return BUILD_ARRAY(new_array, true);
}

Value array_rangef (ValueArray args, bool debug) {
    if (args.count < 1 || args.count > 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value start = GET_ARG(args, 0);
    int cast_error = castValue(&start, TYPE_DOUBLE);
    if (cast_error != E_NULL) {
        return BUILD_EXCEPTION(cast_error);
    }

    Value end;
    if (args.count > 1) {
        end = GET_ARG(args, 1);
        cast_error = castValue(&end, TYPE_DOUBLE);
        if (cast_error != E_NULL) {
            return BUILD_EXCEPTION(cast_error);
        }
    } else {
        end = (Value){ TYPE_DOUBLE, .as.doubleValue = start.as.doubleValue };
        start = (Value){ TYPE_DOUBLE, .as.doubleValue = 0 };
    }

    Value step;
    if (args.count > 2) {
        step = GET_ARG(args, 2);
        cast_error = castValue(&step, TYPE_DOUBLE);
        if (cast_error != E_NULL) {
            return BUILD_EXCEPTION(cast_error);
        }
    } else {
        step = (Value){ TYPE_DOUBLE, .as.doubleValue = 1 };
    }

    if (step.as.doubleValue == 0) {
        return BUILD_EXCEPTION(E_CANNOT_BE_ZERO);
    }

    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (double i = start.as.doubleValue; (step.as.doubleValue > 0 && i < end.as.doubleValue) || (step.as.doubleValue < 0 && i > end.as.doubleValue); i += step.as.doubleValue) {
        write_ValueArray(new_array, (Value){ TYPE_DOUBLE, .as.doubleValue = i });
    }

    return BUILD_ARRAY(new_array, true);
}

long long int counter = 0;

Value array_counter(ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    return (Value){ TYPE_LONG, .as.longValue = counter++ };
}

Value array_set_counter(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_error = castValue(&arg, TYPE_LONG);
    if (cast_error != E_NULL) {
        return BUILD_EXCEPTION(cast_error);
    }

    counter = arg.as.longValue;

    return (Value){ TYPE_LONG, .as.longValue = counter };
}