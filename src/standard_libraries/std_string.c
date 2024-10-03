#include "../../include/standard_libraries/std_string.h"

Value string_split(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    char* delim = AS_STRING(arg2);

    char* token = strtok(str, delim);
    ValueArray* result = malloc(sizeof(ValueArray));
    init_ValueArray(result);
    while (token != NULL) {
        char* new_token = malloc(strlen(token) + 1);
        strcpy(new_token, token);
        Value value = BUILD_STRING(new_token);
        write_ValueArray(result, value);
        token = strtok(NULL, delim);
    }

    return BUILD_ARRAY(result);
}

Value string_lower(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    int cast_result = castValue(&arg, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    toLower(AS_STRING(arg));

    return arg;
}

Value string_upper(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    int cast_result = castValue(&arg, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    toUpper(AS_STRING(arg));

    return arg;
}

Value string_length(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    int cast_result = castValue(&arg, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    size_t len = strlen(AS_STRING(arg));

    return (Value){ TYPE_LONG, .as.longValue = len };
}

Value string_substr(ValueArray args, bool debug) {
    if (args.count != 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);
    Value arg3 = GET_ARG(args, 2);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_LONG);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg3, TYPE_LONG);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    long long int start = arg2.as.longValue;
    long long int end = arg3.as.longValue;

    if (start < 0 || start >= strlen(str) || end < 0 || end >= strlen(str) || start > end) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    char* result = malloc(end - start + 2);
    strncpy(result, str + start, end - start + 1);
    result[end - start + 1] = '\0';


    return BUILD_STRING(result);
}

Value string_indexof(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    char* result = strstr(str, substr);


    if (result == NULL) {
        return (Value){ TYPE_LONG, .as.longValue = -1 };
    }

    long long int res = result - str;


    return (Value){ TYPE_LONG, .as.longValue = res };   
}

Value string_lastindexof(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    long long int res = -1;
    for (size_t i = 0; i < strlen(str); i++) {
        if (strncmp(str + i, substr, strlen(substr)) == 0) {
            res = i;
        }
    }


    return (Value){ TYPE_LONG, .as.longValue = res };
}

Value string_startswith(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    bool result = strncmp(str, substr, strlen(substr)) == 0;


    return (Value){ TYPE_BOOL, .as.boolValue = result };
}

Value string_endswith(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    bool result = strcmp(str + strlen(str) - strlen(substr), substr) == 0;


    return (Value){ TYPE_BOOL, .as.boolValue = result };
}

Value string_replace(ValueArray args, bool debug) {
    if (args.count != 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);
    Value arg3 = GET_ARG(args, 2);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg3, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);
    char* replacement = AS_STRING(arg3);


    char* result = malloc(1);
    result[0] = '\0';
    for (size_t i = 0; i < __max(strlen(str), strlen(result)); i++) {
        bool match = true;
        for (size_t j = 0; j < strlen(substr); j++) {
            if (str[i + j] != substr[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            result = realloc(result, strlen(result) + strlen(replacement) + 1);
            strcat(result, replacement);
            result[strlen(result)] = '\0';
            i += strlen(substr) - 1;
        } else {
            result = realloc(result, strlen(result) + 2);
            size_t len = strlen(result);
            result[len] = str[i];
            result[len + 1] = '\0';
        }
    }
    result[strlen(result)] = '\0';

    return BUILD_STRING(result);
}

Value string_trim(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    int cast_result = castValue(&arg, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg);

    size_t start = 0;
    size_t end = strlen(str) - 1;
    while (IS_SPACE(str[start])) {
        start++;
    }
    while (IS_SPACE(str[end])) {
        end--;
    }
    char* result = malloc(end - start + 2);
    strncpy(result, str + start, end - start + 1);
    result[strlen(result)] = '\0';


    return BUILD_STRING(result);
}

Value string_reverse(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    int cast_result = castValue(&arg, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg);
    size_t len = strlen(str);

    char* result = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        result[i] = str[len - i - 1];
    }
    result[len] = '\0';

    return BUILD_STRING(result);
}

Value string_contains(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    bool result = strstr(str, substr) != NULL;

    return (Value){ TYPE_BOOL, .as.boolValue = result };
}

Value string_remove(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    char* result = malloc(1);
    result[0] = '\0';
    for (size_t i = 0; i < __max(strlen(str), strlen(result)); i++) {
        bool match = true;
        for (size_t j = 0; j < strlen(substr); j++) {
            if (str[i + j] != substr[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            i += strlen(substr) - 1;
        } else {
            result = realloc(result, strlen(result) + 2);
            size_t len = strlen(result);
            result[len] = str[i];
            result[len + 1] = '\0';
        }
    }
    result[strlen(result)] = '\0';

    return BUILD_STRING(result);
}

Value string_insert(ValueArray args, bool debug) {
    if (args.count != 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);
    Value arg3 = GET_ARG(args, 2);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_LONG);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg3, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    long long int index = arg2.as.longValue;
    char* substr = AS_STRING(arg3);

    if (index < 0 || index >= strlen(str)) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    char* result = malloc(strlen(str) + strlen(substr) + 1);
    for (size_t i = 0; i < index; i++) {
        result[i] = str[i];
    }
    for (size_t i = 0; i < strlen(substr); i++) {
        result[index + i] = substr[i];
    }
    for (size_t i = index; i < strlen(str); i++) {
        result[strlen(substr) + i] = str[i];
    }
    result[strlen(str) + strlen(substr)] = '\0';

    return BUILD_STRING(result);
}

Value string_atoi(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    int cast_result = castValue(&arg, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg);
    long long int result = atoll(str);

    return (Value){ TYPE_LONG, .as.longValue = result };
}

Value string_atod(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    int cast_result = castValue(&arg, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg);
    double result = atof(str);

    return (Value){ TYPE_DOUBLE, .as.doubleValue = result };
}

Value string_count(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);


    size_t count = 0;
    char* result = str;
    while ((result = strstr(result, substr)) != NULL) {
        count++;
        result++;
    }

    return (Value){ TYPE_LONG, .as.longValue = count };
}

Value string_join(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    if (arg1.type != TYPE_ARRAY) {
        return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
    }

    int cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    ValueArray* arr = AS_ARRAY(arg1);
    char* delim = AS_STRING(arg2);

    char* result = malloc(1);
    result[0] = '\0';
    for (int i = 0; i < arr->count; i++) {
        char* str = valueToString(arr->values[i], false);
        result = realloc(result, strlen(result) + strlen(str) + strlen(delim) + 1);
        strcat(result, str);
        if (i != arr->count - 1) {
            strcat(result, delim);
        }
        free(str);
    }


    return BUILD_STRING(result);
}