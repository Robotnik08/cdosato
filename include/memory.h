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


// macro to generate list functions
#define DOSATO_LIST_FUNC_GEN(list_type, item_type, item_name) \
    void init_##list_type (list_type* list) { \
        list->item_name = NULL; \
        list->count = 0; \
        list->capacity = 0; \
    } \
    void write_##list_type (list_type* list, item_type item) { \
        if (list->capacity < list->count + 1) { \
            size_t oldCapacity = list->capacity; \
            list->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity); \
            list->item_name = DOSATO_RESIZE_LIST(item_type, list->item_name, oldCapacity, list->capacity); \
        } \
        list->item_name[list->count] = item; \
        list->count++; \
    } \
    void free_##list_type (list_type* list) { \
        DOSATO_FREE_LIST(item_type, list->item_name, list->capacity); \
        init_##list_type(list); \
    }


#endif // dosato_memory_h