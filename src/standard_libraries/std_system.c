#include "../../include/standard_libraries/std_system.h"

Value io_clear (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif

    return UNDEFINED_VALUE;
}

Value io_end (ValueArray args, bool debug) {
    if (args.count > 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    if (args.count == 1) {
        Value arg = GET_ARG(args, 0);
        CAST_SAFE(arg, TYPE_LONG);

        return BUILD_HLT(AS_LONG(arg));
    }

    return BUILD_HLT(0);
}

Value io_pause (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    printf("Press enter to continue...");
    getchar();

    return UNDEFINED_VALUE;
}

Value io_system (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_STRING);

    int result = system(AS_STRING(arg));

    return BUILD_LONG(result);
}