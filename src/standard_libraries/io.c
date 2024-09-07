#include "../../include/standard_libraries/io.h"

Value io_say (ValueArray args, bool debug) {
    if (args.count == 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    int char_count = 0;
    for (int i = 0; i < args.count; i++) {
        Value arg = GET_ARG(args, i);
        if (arg.type == TYPE_STRING) {
            char_count += strlen(arg.as.stringValue);
            printf("%s", arg.as.stringValue);
        } else {
            char* str = valueToString(arg, false);
            char_count += strlen(str);
            printf("%s", str);
            free(str);
        }
    }
    
    return (Value){ TYPE_LONG, .as.longValue = char_count };
}

Value io_sayln (ValueArray args, bool debug) {
    Value say_result = io_say(args, debug);
    if (say_result.type == TYPE_EXPECTION) {
        return say_result;
    }
    printf("\n"); // sayln adds a newline
    say_result.as.longValue++; // add one to the char count
    return say_result;
}

Value io_listen (ValueArray args, bool debug) {
    if (args.count > 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    if (args.count >= 1) {
        // print the prompt
        Value arg = GET_ARG(args, 0);
        if (arg.type == TYPE_STRING) {
            printf("%s", arg.as.stringValue);
        } else {
            char* str = valueToString(arg, false);
            printf("%s", str);
            free(str);
        }
    }

    char delimiter = '\n';
    if (args.count == 2) {
        Value arg = GET_ARG(args, 1);
        int cast_result = castValue(&arg, TYPE_CHAR);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        delimiter = arg.as.charValue;
    }

    char* input = malloc(1);
    input[0] = '\0';
    size_t size = 0;
    size_t capacity = 1;
    char c;
    while ((c = getchar()) != delimiter) {
        if (size + 1 >= capacity) {
            capacity *= 2;
            input = realloc(input, capacity);
        }
        input[size++] = c;
        input[size] = '\0';
    }

    return (Value){ TYPE_STRING, .as.stringValue = input };
}
