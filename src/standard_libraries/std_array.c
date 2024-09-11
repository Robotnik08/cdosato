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
            return (Value){ TYPE_EXCEPTION, .as.longValue = E_EXPECTED_LONG };
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

    Value arg = GET_ARG(args, 0);
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
        return hardCopyValue(arg);
    }

    qsort(((ValueArray*)arg.as.objectValue)->values, ((ValueArray*)arg.as.objectValue)->count, sizeof(Value), compareValues);

    return hardCopyValue(arg);
}