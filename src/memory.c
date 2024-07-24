#include "../include/common.h"

#include "../include/memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        if (oldSize != 0) {
            free(pointer);
        }
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) {
        printf("Memory reallocation failed: %zu bytes\n", newSize);
        exit(1);
    }
    return result;
}