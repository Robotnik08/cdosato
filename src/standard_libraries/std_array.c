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
        write_ValueArray(&args, hardCopyValue(array->values[j]));
        write_ValueArray(&args, hardCopyValue(pivot));
        Value result = callExternalFunction(function, args, false);
        free_ValueArray(&args);
        if (result.type == TYPE_EXCEPTION || result.type == TYPE_HLT) {
            return result;
        }

        if (result.type != TYPE_LONG) {
            destroyValue(&result);
            return BUILD_EXCEPTION(E_EXPECTED_LONG);
        }
        
        if (result.as.longValue < 0) {
            i++;
            Value temp = array->values[i];
            array->values[i] = array->values[j];
            array->values[j] = temp;
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

    Value arg = GET_ARG_COPY(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    if (args.count == 2) {
        Value arg2 = GET_ARG(args, 1);
        if (arg2.type != TYPE_FUNCTION) {
            return BUILD_EXCEPTION(E_NOT_A_FUNCTION);
        }
        Value res = dosato_quick_sort(((ValueArray*)arg.as.objectValue), 0, ((ValueArray*)arg.as.objectValue)->count - 1, arg2.as.longValue);

        if (res.type == TYPE_EXCEPTION || res.type == TYPE_HLT) {
            return res;
        }
        return arg;
    }

    qsort(((ValueArray*)arg.as.objectValue)->values, ((ValueArray*)arg.as.objectValue)->count, sizeof(Value), compareValues);

    return arg;
}

Value array_push (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    Value value = GET_ARG(args, 1);
    write_ValueArray((ValueArray*)arg.as.objectValue, hardCopyValue(value));

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

    if (((ValueArray*)arg.as.objectValue)->count == 0) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    Value arr = hardCopyValue(arg);

    destroyValue(&((ValueArray*)arr.as.objectValue)->values[((ValueArray*)arr.as.objectValue)->count - 1]);

    ((ValueArray*)arr.as.objectValue)->count--;

    return arr;
}

Value array_shift (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    if (((ValueArray*)arg.as.objectValue)->count == 0) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    Value arr = hardCopyValue(arg);
    ValueArray* obj = (ValueArray*)arr.as.objectValue;

    destroyValue(&obj->values[0]);
    obj->count--;
    for (int i = 0; i < obj->count; i++) {
        obj->values[i] = obj->values[i + 1];
    }

    return arr;
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
    ValueArray* obj = (ValueArray*)arg.as.objectValue;

    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    write_ValueArray(new_array, hardCopyValue(value));
    for (int i = 0; i < obj->count; i++) {
        write_ValueArray(new_array, hardCopyValue(obj->values[i]));
    }

    return (Value){ TYPE_ARRAY, .as.objectValue = new_array, .defined = false };
}

Value array_slice (ValueArray args, bool debug) {
    if (args.count < 2 || args.count > 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    Value start = GET_ARG_COPY(args, 1);
    int cast_error = castValue(&start, TYPE_LONG);
    if (cast_error != E_NULL) {
        return BUILD_EXCEPTION(cast_error);
    }

    Value end;
    if (args.count == 3) {
        end = GET_ARG_COPY(args, 2);
        cast_error = castValue(&end, TYPE_LONG);
        if (cast_error != E_NULL) {
            return BUILD_EXCEPTION(cast_error);
        }
    } else {
        end = (Value){ TYPE_LONG, .as.longValue = ((ValueArray*)arg.as.objectValue)->count };
    }

    if (start.as.longValue < 0 || start.as.longValue >= ((ValueArray*)arg.as.objectValue)->count || end.as.longValue < 0 || end.as.longValue > ((ValueArray*)arg.as.objectValue)->count) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    ValueArray* obj = (ValueArray*)arg.as.objectValue;
    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = start.as.longValue; i < end.as.longValue; i++) {
        write_ValueArray(new_array, hardCopyValue(obj->values[i]));
    }

    return (Value){ TYPE_ARRAY, .as.objectValue = new_array, .defined = false };
}

Value array_splice (ValueArray args, bool debug) {
    if (args.count < 2 || args.count > 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (arg.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    Value start = GET_ARG_COPY(args, 1);
    int cast_error = castValue(&start, TYPE_LONG);
    if (cast_error != E_NULL) {
        return BUILD_EXCEPTION(cast_error);
    }

    Value delete_count;
    if (args.count > 2) {
        delete_count = GET_ARG_COPY(args, 2);
        cast_error = castValue(&delete_count, TYPE_LONG);
        if (cast_error != E_NULL) {
            return BUILD_EXCEPTION(cast_error);
        }
    } else {
        delete_count = (Value){ TYPE_LONG, .as.longValue = ((ValueArray*)arg.as.objectValue)->count - start.as.longValue };
    }

    ValueArray* obj = (ValueArray*)arg.as.objectValue;
    if (start.as.longValue < 0 || start.as.longValue >= obj->count || delete_count.as.longValue < 0 || start.as.longValue + delete_count.as.longValue > obj->count) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    ValueArray* new_array = malloc(sizeof(ValueArray));
    init_ValueArray(new_array);
    for (int i = 0; i < start.as.longValue; i++) {
        write_ValueArray(new_array, hardCopyValue(obj->values[i]));
    }
    for (int i = start.as.longValue + delete_count.as.longValue; i < obj->count; i++) {
        write_ValueArray(new_array, hardCopyValue(obj->values[i]));
    }

    return (Value){ TYPE_ARRAY, .as.objectValue = new_array, .defined = false };
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

    ValueArray* obj = (ValueArray*)arg.as.objectValue;
    for (int i = 0; i < obj->count; i++) {
        if (valueEquals(&obj->values[i], &value)) {
            return (Value){ TYPE_LONG, .as.longValue = i };
        }
    }

    return (Value){ TYPE_LONG, .as.longValue = -1 };
}

Value array_range (ValueArray args, bool debug) {
    if (args.count < 1 || args.count > 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value start = GET_ARG_COPY(args, 0);
    int cast_error = castValue(&start, TYPE_LONG);
    if (cast_error != E_NULL) {
        return BUILD_EXCEPTION(cast_error);
    }

    Value end;
    if (args.count > 1) {
        end = GET_ARG_COPY(args, 1);
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
        step = GET_ARG_COPY(args, 2);
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

    return (Value){ TYPE_ARRAY, .as.objectValue = new_array, .defined = false };
}

Value array_rangef (ValueArray args, bool debug) {
    if (args.count < 1 || args.count > 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value start = GET_ARG_COPY(args, 0);
    int cast_error = castValue(&start, TYPE_DOUBLE);
    if (cast_error != E_NULL) {
        return BUILD_EXCEPTION(cast_error);
    }

    Value end;
    if (args.count > 1) {
        end = GET_ARG_COPY(args, 1);
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
        step = GET_ARG_COPY(args, 2);
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

    return (Value){ TYPE_ARRAY, .as.objectValue = new_array, .defined = false };
}