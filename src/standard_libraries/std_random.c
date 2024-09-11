#include "../../include/standard_libraries/std_random.h"

Value io_seed_random (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_UINT);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    srand(arg.as.uintValue);

    return UNDEFINED_VALUE;
}

Value io_random_double (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    double result = (double)rand() / RAND_MAX;
    return (Value){ TYPE_DOUBLE, .as.doubleValue = result };
}

Value io_random_int (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    long long int result = rand();
    return (Value){ TYPE_LONG, .as.longValue = result };
}

Value io_random_range (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    int cast_result = castValue(&arg1, TYPE_LONG);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_LONG);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    long long int result = rand() % (arg2.as.longValue - arg1.as.longValue + 1) + arg1.as.longValue;

    return (Value){ TYPE_LONG, .as.longValue = result };
}

Value io_random_bool (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    bool result = rand() % 2;
    return (Value){ TYPE_BOOL, .as.boolValue = result };
}