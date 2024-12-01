#include "../../include/standard_libraries/std_type.h"

Value type_typeof(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    DataType type = GET_ARG(args, 0).type;
    return BUILD_STRING(COPY_STRING(dataTypeToString(type)), true);
}

Value type_isnull(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value value = GET_ARG(args, 0);
    return BUILD_BOOL(value.type == D_NULL);
}

Value type_typeof_int(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    DataType type = GET_ARG(args, 0).type;
    return BUILD_LONG(type);
}

Value type_isnan(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value value = GET_ARG(args, 0);
    if (ISFLOATTYPE(value.type)) {
        CAST_SAFE(value, TYPE_DOUBLE);
        return BUILD_BOOL(isnan(AS_DOUBLE(value)));
    } else if (ISINTTYPE(value.type) || value.type == TYPE_CHAR || value.type == TYPE_BOOL) {
        return BUILD_BOOL(false);
    } else {
        return BUILD_BOOL(true); // not a number
    }
}

Value type_throw(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value exception = GET_ARG(args, 0);
    if (exception.type == TYPE_STRING) {
        printf("ERROR: \n%s\n", AS_STRING(exception));
        return BUILD_EXCEPTION(E_EMPTY_MESSAGE);
    }

    CAST_SAFE(exception, TYPE_LONG);
    return BUILD_EXCEPTION(AS_LONG(exception));
}