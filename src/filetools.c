#include "../include/common.h"

#include "../include/filetools.h"

// Function to check if a directory exists
int directory_exists(const char *path) {
    struct stat statbuf;
    // Check if the path exists and is a directory
    if (stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
        return 1; // Directory exists
    }
    return 0; // Directory does not exist
}

void get_files_in_directory(const char *path, char ***files, int *count) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("opendir");
        *files = NULL;
        *count = 0;
        return;
    }

    // Temporary storage for file names
    char **file_list = NULL;
    int file_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Skip current directory "." and parent directory ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Allocate memory for a new file name
        file_list = realloc(file_list, sizeof(char *) * (file_count + 1));
        if (file_list == NULL) {
            perror("realloc");
            closedir(dir);
            *files = NULL;
            *count = 0;
            return;
        }

        file_list[file_count] = strdup(entry->d_name);
        if (file_list[file_count] == NULL) {
            perror("strdup");
            closedir(dir);
            // Free previously allocated strings
            for (int i = 0; i < file_count; i++) {
                free(file_list[i]);
            }
            free(file_list);
            *files = NULL;
            *count = 0;
            return;
        }

        file_count++;
    }

    closedir(dir);

    *files = file_list;
    *count = file_count;
}

void free_files(char **files, int count) {
    for (int i = 0; i < count; i++) {
        free(files[i]);
    }
    free(files);
}

long long int getFileSize(FILE *file) {
    long long size = 0;
    int ch;

    if (file == NULL) {
        return -1; // Error: file is NULL
    }

    // Set file position to the beginning
    if (fseeko(file, 0, SEEK_SET) != 0) {
        return -1; // Error
    }

    // Count characters, handling \r\n
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\r') {
            int next_ch = fgetc(file);
            if (next_ch != '\n') {
                // If the next character is not '\n', put it back
                if (next_ch != EOF) {
                    ungetc(next_ch, file);
                }
            }
            // Increment size for '\r'
            size++;
        }
        size++;
    }

    // Reset file pointer to the beginning
    if (fseeko(file, 0, SEEK_SET) != 0) {
        return -1; // Error
    }

    return size;
}

char* readStdin(long long int* length) {
    char* str = malloc(1);
    if (str == NULL) {
        return NULL;
    }
    str[0] = '\0';

    long long int size = 0;

    printf("%s", ">>> ");

    while (1) {
        char* new_str = realloc(str, size + 2);
        if (new_str == NULL) {
            free(str);
            return NULL;
        }
        str = new_str;

        char ch = getchar();
        if (ch == '\n' && str[size - 1] != '\\') {
            break;
        }

        if (ch == '\n') {
            size--;
            printf("%s", ">>> ");
        }
        
        str[size] = ch;
        size++;
    }

    str[size] = '\0';

    *length = size;

    return str;
}