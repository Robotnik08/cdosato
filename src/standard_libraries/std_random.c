#include "../../include/standard_libraries/std_random.h"

Value io_seed_random (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_UINT);

    srand(AS_UINT(arg));
    rand(); // Discard the first random number

    return UNDEFINED_VALUE;
}

Value io_random_double (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    double result = (double)rand() / (double)RAND_MAX;
    return BUILD_DOUBLE(result);
}

Value io_random_int (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    long long int result = rand();
    return BUILD_LONG(result);
}

Value io_random_range (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_LONG);
    CAST_SAFE(arg2, TYPE_LONG);

    long long int result = rand() % (AS_LONG(arg2) - AS_LONG(arg1) + 1) + AS_LONG(arg1);

    return BUILD_LONG(result);
}

Value io_random_bool (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    bool result = rand() % 2;
    return BUILD_BOOL(result);
}