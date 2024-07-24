#ifndef dosato_memory_h
#define dosato_memory_h

#include "common.h"

#define DOSATO_UPDATE_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

#define DOSATO_RESIZE_LIST(type, pointer, oldCount, newCount) \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))

#define DOSATO_REALLOC_LIST_SIZE(type, list, size) (type *)realloc(list, sizeof(type) * size)

#define DOSATO_FREE_LIST(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)


void *reallocate(void *pointer, size_t oldSize, size_t newSize);

#endif // dosato_memory_h