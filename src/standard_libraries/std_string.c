#include "../../include/standard_libraries/std_string.h"

Value string_split(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

    char* str = AS_STRING(arg1);
    char* delim = AS_STRING(arg2);

    if (strlen(delim) == 0) {
        // build an array of characters
        ValueArray* result = malloc(sizeof(ValueArray));
        init_ValueArray(result);

        for (size_t i = 0; i < strlen(str); i++) {
            char* new_token = malloc(2);
            new_token[0] = str[i];
            new_token[1] = '\0';
            Value value = BUILD_STRING(new_token, false);
            write_ValueArray(result, value);
        }

        return BUILD_ARRAY(result, true);
    }

    if (strlen(delim) == 1) {
        char* str = COPY_STRING(AS_STRING(arg1));
        char* token = strtok(str, delim);
        ValueArray* result = malloc(sizeof(ValueArray));
        init_ValueArray(result);
        while (token != NULL) {
            Value value = BUILD_STRING(COPY_STRING(token), false);
            write_ValueArray(result, value);
            token = strtok(NULL, delim);
        }
        free(str);
        
        return BUILD_ARRAY(result, true);
    }

    // build an array of strings
    ValueArray* result = malloc(sizeof(ValueArray));
    init_ValueArray(result);

    int last_index = 0;
    for (size_t i = 0; i < strlen(str) - strlen(delim); i++) {
        bool match = true;
        for (size_t j = 0; j < strlen(delim); j++) {
            if (str[i + j] != delim[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            if (i - last_index == 0) {
                last_index = i + strlen(delim);
                i += strlen(delim) - 1;
                continue;
            }
            char* new_token = malloc(i - last_index + 1);
            strncpy(new_token, str + last_index, i - last_index);
            new_token[i - last_index] = '\0';
            Value value = BUILD_STRING(new_token, false);
            write_ValueArray(result, value);
            last_index = i + strlen(delim);
            i += strlen(delim) - 1;
        }
    }

    if (last_index < strlen(str)) {
        char* new_token = malloc(strlen(str) - last_index + 1);
        strncpy(new_token, str + last_index, strlen(str) - last_index);
        new_token[strlen(str) - last_index] = '\0';
        Value value = BUILD_STRING(new_token, false);
        write_ValueArray(result, value);
    }

    return BUILD_ARRAY(result, true);
}

Value string_lower(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    CAST_SAFE(arg, TYPE_STRING);

    char* str = COPY_STRING(AS_STRING(arg));

    toLower(str);

    return BUILD_STRING(str, true);
}

Value string_upper(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    CAST_SAFE(arg, TYPE_STRING);

    char* str = COPY_STRING(AS_STRING(arg));
    
    toUpper(str);

    return BUILD_STRING(str, true);
}

Value string_substr(ValueArray args, bool debug) {
    if (args.count > 3 || args.count < 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_LONG);

    

    char* str = AS_STRING(arg1);
    long long int start = AS_LONG(arg2);
    long long int end = strlen(str) - 1;
    if (args.count == 3) {
        Value arg3 = GET_ARG(args, 2);
        CAST_SAFE(arg3, TYPE_LONG);
        end = AS_LONG(arg3);
    }

    if (end < 0) {
        end = strlen(str) + end;
    }

    if (start < 0) {
        start = end + start;
    }

    if (start > end) {
        return BUILD_STRING(COPY_STRING(""), true);
    }

    char* result = malloc(end - start + 2);
    strncpy(result, str + start, end - start + 1);
    result[end - start + 1] = '\0';


    return BUILD_STRING(result, true);
}

Value string_indexof(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    char* result = strstr(str, substr);


    if (result == NULL) {
        return BUILD_LONG(-1);
    }

    long long int res = result - str;


    return BUILD_LONG(res);
}

Value string_lastindexof(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    long long int res = -1;
    for (size_t i = 0; i < strlen(str); i++) {
        if (strncmp(str + i, substr, strlen(substr)) == 0) {
            res = i;
        }
    }


    return BUILD_LONG(res);
}

Value string_startswith(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    bool result = strncmp(str, substr, strlen(substr)) == 0;


    return BUILD_BOOL(result);
}

Value string_endswith(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    bool result = strcmp(str + strlen(str) - strlen(substr), substr) == 0;


    return BUILD_BOOL(result);
}

Value string_replace(ValueArray args, bool debug) {
    if (args.count != 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);
    Value arg3 = GET_ARG(args, 2);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);
    CAST_SAFE(arg3, TYPE_STRING);

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);
    char* replacement = AS_STRING(arg3);

    int replacement_len = strlen(replacement);
    int substr_len = strlen(substr);
    int str_len = strlen(str);

    char* result = malloc(1);
    result[0] = '\0';
    for (size_t i = 0; i < str_len; i++) {
        bool match = true;
        for (size_t j = 0; j < substr_len; j++) {
            if (i + substr_len > str_len) {
                match = false;
                break;
            }
            if (str[i + j] != substr[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            int len = strlen(result);
            result = realloc(result, len + replacement_len + 1);
            strcat(result, replacement);
            i += substr_len - 1;
        } else {
            result = realloc(result, strlen(result) + 2);
            size_t len = strlen(result);
            result[len] = str[i];
            result[len + 1] = '\0';
        }
    }

    return BUILD_STRING(result, true);
}

Value string_replace_first(ValueArray args, bool debug) {
    if (args.count != 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);
    Value arg3 = GET_ARG(args, 2);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);
    CAST_SAFE(arg3, TYPE_STRING);

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);
    char* replacement = AS_STRING(arg3);

    int replacement_len = strlen(replacement);
    int substr_len = strlen(substr);

    char* result = malloc(1);
    result[0] = '\0';
    bool replaced = false;
    for (size_t i = 0; i < strlen(str); i++) {
        bool match = true;
        for (size_t j = 0; j < strlen(substr); j++) {
            if (str[i + j] != substr[j]) {
                match = false;
                break;
            }
        }
        if (match && !replaced) {
            int len = strlen(result);
            result = realloc(result, strlen(result) + replacement_len + 1);
            strcat(result, replacement);
            i += substr_len - 1;
            result[len + replacement_len + 1] = '\0';
            replaced = true;
        } else {
            result = realloc(result, strlen(result) + 2);
            size_t len = strlen(result);
            result[len] = str[i];
            result[len + 1] = '\0';
        }
    }

    return BUILD_STRING(result, true);
}

Value string_trim(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    CAST_SAFE(arg, TYPE_STRING);

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
    result[end - start + 1] = '\0';


    return BUILD_STRING(result, true);
}

Value string_reverse(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    CAST_SAFE(arg, TYPE_STRING);

    char* str = AS_STRING(arg);
    size_t len = strlen(str);

    char* result = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        result[i] = str[len - i - 1];
    }
    result[len] = '\0';

    return BUILD_STRING(result, true);
}

Value string_contains(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);

    bool result = strstr(str, substr) != NULL;

    return BUILD_BOOL(result);
}

Value string_remove(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

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

    return BUILD_STRING(result, true);
}

Value string_insert(ValueArray args, bool debug) {
    if (args.count != 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);
    Value arg3 = GET_ARG(args, 2);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_LONG);
    CAST_SAFE(arg3, TYPE_STRING);


    char* str = AS_STRING(arg1);
    long long int index = AS_LONG(arg2);
    char* substr = AS_STRING(arg3);

    if (index < 0 || index >= strlen(str) + 1) {
        return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
    }

    char* result = malloc(strlen(str) + strlen(substr) + 10);
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

    return BUILD_STRING(result, true);
}

Value string_atoi(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    CAST_SAFE(arg, TYPE_STRING);

    char* str = AS_STRING(arg);
    long long int result = atoll(str);

    return BUILD_LONG(result);
}

Value string_atod(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);

    CAST_SAFE(arg, TYPE_STRING);

    char* str = AS_STRING(arg);
    double result = atof(str);

    return BUILD_DOUBLE(result);
}

Value string_count(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

    char* str = AS_STRING(arg1);
    char* substr = AS_STRING(arg2);


    size_t count = 0;
    char* result = str;
    while ((result = strstr(result, substr)) != NULL) {
        count++;
        result++;
    }

    return BUILD_ULONG(count);
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

    CAST_SAFE(arg2, TYPE_STRING);

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


    return BUILD_STRING(result, true);
}

Value string_format(ValueArray args, bool debug) {
    if (args.count < 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);

    CAST_SAFE(arg1, TYPE_STRING);

    char* format = AS_STRING(arg1);
    char* result = malloc(1);
    result[0] = '\0';
    int arg_index = 1;
    for (size_t i = 0; i < strlen(format); i++) {
        if (format[i] == '{' && format[i + 1] == '}') {
            if (arg_index >= args.count) {
                return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
            }
            Value arg = GET_ARG(args, arg_index);
            arg_index++;
            char* str = valueToString(arg, false);
            result = realloc(result, strlen(result) + strlen(str) + 1);
            strcat(result, str);
            i++; // skip the closing brace
            free(str);
        } else {
            result = realloc(result, strlen(result) + 2);
            size_t len = strlen(result);
            result[len] = format[i];
            result[len + 1] = '\0';
        }
    }

    return BUILD_STRING(result, true);
}

Value string_to_string(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_COPY_TO_STRING(arg);

    return arg;
}

Value string_compare (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    CAST_SAFE(a, TYPE_STRING);
    CAST_SAFE(b, TYPE_STRING);

    return BUILD_LONG(strcmp(AS_STRING(a), AS_STRING(b)));
}