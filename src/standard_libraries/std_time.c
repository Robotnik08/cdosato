#include "../../include/standard_libraries/std_time.h"

Value io_time (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    time_t t = time(NULL);
    return (Value){ TYPE_LONG, .as.longValue = t };
}

Value io_datetime (ValueArray args, bool debug) {
    if (args.count > 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    char* format = "%Y-%m-%d %H:%M:%S";
    if (args.count == 2) {
        Value arg = hardCopyValue(GET_ARG(args, 1));
        int cast_result = castValue(&arg, TYPE_STRING);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        format = arg.as.stringValue;
    }

    time_t t = time(NULL);
    if (args.count >= 1) {
        Value arg = GET_ARG(args, 0);
        int cast_result = castValue(&arg, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        t = arg.as.longValue;
    }

    struct tm* tm_info = localtime(&t);
    char* buffer = malloc(80);
    strftime(buffer, 80, format, tm_info);

    return (Value){ TYPE_STRING, .as.stringValue = buffer, .defined = false };
}

Value io_clock (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    return (Value){ TYPE_LONG, .as.longValue = clock() };
}

Value io_sleep (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    double seconds = arg.as.doubleValue;

    clock_t start = clock();
    while ((double)(clock() - start) / CLOCKS_PER_SEC < seconds);

    return UNDEFINED_VALUE;
}