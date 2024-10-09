#include "../../include/standard_libraries/std_type.h"

Value type_typeof(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    DataType type = GET_ARG(args, 0).type;
    char* typeString = dataTypeToString(type);
    char* result = malloc(strlen(typeString) + 1);
    strcpy(result, typeString);
    return BUILD_STRING(result, true);
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