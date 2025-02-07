#include "../../include/standard_libraries/std_io.h"

#ifndef _WIN32
#include <errno.h>
#endif

Value io_say (ValueArray args, bool debug) {

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
    
    return BUILD_INT(char_count);
}

Value io_sayln (ValueArray args, bool debug) {
    Value say_result = io_say(args, debug);
    if (say_result.type == TYPE_EXCEPTION) {
        return say_result;
    }
    printf("%s", "\n"); // sayln adds a newline
    AS_LONG(say_result)++; // add one to the char count
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
        Value arg = GET_ARG(args, 1);
        CAST_SAFE(arg, TYPE_CHAR);
        delimiter = AS_CHAR(arg);
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

    return BUILD_STRING(input, true);
}


Value io_read_file (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_STRING);

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

    return BUILD_STRING(buffer, true);
}

Value io_write_file (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

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

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

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

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_STRING);

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

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_STRING);

    FILE* file = fopen(AS_STRING(arg), "r");
    if (file == NULL) {
        return BUILD_BOOL(false);
    }

    fclose(file);

    return BUILD_BOOL(true);
}

Value io_move_file (ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg1 = GET_ARG(args, 0);
    Value arg2 = GET_ARG(args, 1);

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

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

    CAST_SAFE(arg1, TYPE_STRING);
    CAST_SAFE(arg2, TYPE_STRING);

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

Value io_create_directory (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_STRING);

    #ifdef _WIN32
    int result = mkdir(AS_STRING(arg));
    #else
    int result = mkdir(AS_STRING(arg), 0777);
    #endif
    if (result != 0) {
        #ifdef _WIN32
        _fcloseall(); // close all files if any were opened
        #endif

        return BUILD_EXCEPTION((errno == EACCES ? E_FILE_PERMISSION_DENIED : E_FILE_ALREADY_EXISTS));
    }

    return UNDEFINED_VALUE;
}


int delete_directory_contents(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return -1;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                if (delete_directory_contents(full_path) != 0) {
                    closedir(dir);
                    return -1;
                }
                rmdir(full_path);
            } else {
                remove(full_path);
            }
        }
    }
    closedir(dir);
    return 0;
}

Value io_delete_directory (ValueArray args, bool debug) {
    if (args.count > 2 || args.count == 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    bool remove_contents = false;
    if (args.count == 2) {
        Value arg = GET_ARG(args, 1);
        CAST_SAFE(arg, TYPE_BOOL);
        remove_contents = AS_BOOL(arg);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_STRING);

    if (remove_contents) {
        // Function to recursively delete directory contents

        if (delete_directory_contents(AS_STRING(arg)) != 0) {
            return BUILD_EXCEPTION(E_FILE_PERMISSION_DENIED);
        }
    }

    int result = rmdir(AS_STRING(arg));
    if (result != 0) {
        #ifdef _WIN32
        _fcloseall(); // close all files if any were opened
        #endif

        return BUILD_EXCEPTION((errno == EACCES ? E_FILE_PERMISSION_DENIED : (errno == ENOTEMPTY ? E_DIRECTORY_NOT_EMPTY : E_FILE_NOT_FOUND)));
    }

    return UNDEFINED_VALUE;
}

Value io_directory_exists (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_STRING);

    int result = directory_exists(AS_STRING(arg));
    return BUILD_BOOL(result);
}

Value io_get_files_directory (ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_STRING);

    char** files;
    int count;
    get_files_in_directory(AS_STRING(arg), &files, &count);

    ValueArray* array = malloc(sizeof(ValueArray));
    init_ValueArray(array);
    for (int i = 0; i < count; i++) {
        write_ValueArray(array, BUILD_STRING(COPY_STRING(files[i]), false));
        free(files[i]);
    }
    
    free_files(files, count);

    return BUILD_ARRAY(array, true);
}