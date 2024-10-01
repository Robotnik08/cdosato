#include "../../include/standard_libraries/std_io.h"

#ifndef _WIN32
#include <errno.h>
#endif

Value io_say (ValueArray args, bool debug) {
    if (args.count == 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    int char_count = 0;
    for (int i = 0; i < args.count; i++) {
        Value arg = GET_ARG(args, i);
        if (arg.type == TYPE_STRING) {
            char_count += strlen(AS_STRING(arg));
            printf("%s", AS_STRING(arg));
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
    if (say_result.type == TYPE_EXCEPTION) {
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
            printf("%s", AS_STRING(arg));
        } else {
            char* str = valueToString(arg, false);
            printf("%s", str);
            free(str);
        }
    }

    char delimiter = '\n';
    if (args.count == 2) {
        Value arg = GET_ARG_COPY(args, 1);
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

    return BUILD_STRING(input);
}


Value io_read_file (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    FILE* file = fopen(AS_STRING(arg), "r");
    if (file == NULL) {
        #ifdef _WIN32
        _fcloseall(); // close all files if any were opened
        #endif

        return BUILD_EXCEPTION((errno == EACCES ? E_FILE_PERMISSION_DENIED : E_FILE_NOT_FOUND));
    }

    long long int file_size = getFileSize(file);

    char* buffer = malloc(file_size + 1);
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';

    fclose(file);

    return BUILD_STRING(buffer);
}

Value io_write_file (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG_COPY(args, 0);
    Value arg2 = GET_ARG_COPY(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    FILE* file = fopen(AS_STRING(arg1), "w");
    if (file == NULL) {
        #ifdef _WIN32
        _fcloseall(); // close all files if any were opened
        #endif

        return BUILD_EXCEPTION((errno == EACCES ? E_FILE_PERMISSION_DENIED : E_FILE_NOT_FOUND));
    }

    fwrite(AS_STRING(arg2), sizeof(char), strlen(AS_STRING(arg2)), file);
    fclose(file);

    return UNDEFINED_VALUE;
}

Value io_append_file (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG_COPY(args, 0);
    Value arg2 = GET_ARG_COPY(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    FILE* file = fopen(AS_STRING(arg1), "a");
    if (file == NULL) {
        #ifdef _WIN32
        _fcloseall(); // close all files if any were opened
        #endif

        return BUILD_EXCEPTION((errno == EACCES ? E_FILE_PERMISSION_DENIED : E_FILE_NOT_FOUND));
    }

    fwrite(AS_STRING(arg2), sizeof(char), strlen(AS_STRING(arg2)), file);
    fclose(file);

    return UNDEFINED_VALUE;
}

Value io_delete_file (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    int result = remove(AS_STRING(arg));
    if (result != 0) {
        #ifdef _WIN32
        _fcloseall(); // close all files if any were opened
        #endif

        return BUILD_EXCEPTION((errno == EACCES ? E_FILE_PERMISSION_DENIED : E_FILE_NOT_FOUND));
    }

    return UNDEFINED_VALUE;
}

Value io_file_exists (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    FILE* file = fopen(AS_STRING(arg), "r");
    if (file == NULL) {
        return (Value){ TYPE_BOOL, .as.boolValue = false };
    }

    fclose(file);

    return (Value){ TYPE_BOOL, .as.boolValue = true };
}

Value io_move_file (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG_COPY(args, 0);
    Value arg2 = GET_ARG_COPY(args, 1);

    int cast_result = castValue(&arg1, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    cast_result = castValue(&arg2, TYPE_STRING);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }

    int result = rename(AS_STRING(arg1), AS_STRING(arg2));
    if (result != 0) {
        #ifdef _WIN32
        _fcloseall(); // close all files if any were opened
        #endif

        return BUILD_EXCEPTION((errno == EACCES ? E_FILE_PERMISSION_DENIED : E_FILE_ALREADY_EXISTS));
    }

    return UNDEFINED_VALUE;
}

Value io_copy_file (ValueArray args, bool debug) {
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

    FILE* file1 = fopen(AS_STRING(arg1), "r");
    if (file1 == NULL) {
        #ifdef _WIN32
        _fcloseall(); // close all files if any were opened
        #endif

        return BUILD_EXCEPTION((errno == EACCES ? E_FILE_PERMISSION_DENIED : E_FILE_NOT_FOUND));
    }

    FILE* file2 = fopen(AS_STRING(arg2), "w");
    if (file2 == NULL) {
        #ifdef _WIN32
        _fcloseall(); // close all files if any were opened
        #endif

        fclose(file1);
        return BUILD_EXCEPTION((errno == EACCES ? E_FILE_PERMISSION_DENIED : E_FILE_NOT_FOUND));
    }

    long long int file_size = getFileSize(file1);
    char* buffer = malloc(file_size);
    fread(buffer, 1, file_size, file1);
    fwrite(buffer, 1, file_size, file2);

    free(buffer);

    fclose(file1);
    fclose(file2);

    return UNDEFINED_VALUE;
}