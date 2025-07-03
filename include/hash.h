#ifndef dosato_hash_h
#define dosato_hash_h

#include "common.h"
#include "value.h"

#include <stdint.h>

#define INITIAL_HASH_TABLE_SIZE 32 // Initial size of the hash table
#define LOAD_FACTOR (INITIAL_HASH_TABLE_SIZE / 4 * 3) // Load factor for resizing the hash table (3/4 or 0.75)

typedef struct {
    uint64_t hash;       // Hash value of the key
    Value value;
    struct HashEntry* next; // Pointer to the next entry in case of a collision
} HashEntry;

typedef struct {
    HashEntry** entries; // Array of pointers to HashEntry
    size_t count;        // Number of entries in the hash table
    size_t size;         // Size of the hash table
} HashTable;

uint64_t hashValue(Value value);

#endif // dosato_hash_h