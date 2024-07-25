#include "../include/common.h"

#include "../include/filetools.h"

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