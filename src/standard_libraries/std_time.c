#include "../../include/standard_libraries/std_time.h"

Value io_time (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    time_t t = time(NULL);
    return BUILD_LONG(t);
}

Value io_datetime (ValueArray args, bool debug) {
    if (args.count > 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    char* format = "%Y-%m-%d %H:%M:%S";
    bool allocated = false;
    if (args.count == 2) {
        Value arg = GET_ARG(args, 1);
        CAST_SAFE(arg, TYPE_STRING);
        format = malloc(strlen(AS_STRING(arg)) + 1);
        strcpy(format, AS_STRING(arg));
        allocated = true;
    }

    time_t t = time(NULL);
    if (args.count >= 1) {
        Value arg = GET_ARG(args, 0);
        CAST_SAFE(arg, TYPE_LONG);
        t = AS_LONG(arg);
    }

    struct tm* tm_info = localtime(&t);
    char* buffer = malloc(80);
    strftime(buffer, 80, format, tm_info);

    if (allocated) free(format);

    return BUILD_STRING(buffer, true);
}

Value io_clock (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    return BUILD_LONG(clock());
}

Value io_sleep (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);

    double seconds = AS_DOUBLE(arg);

    clock_t start = clock();
    while ((double)(clock() - start) / CLOCKS_PER_SEC < seconds);

    return UNDEFINED_VALUE;
}