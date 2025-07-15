#ifndef dosato_hash_h
#define dosato_hash_h

#include "common.h"
#include "value.h"

#include <stdint.h>

#define INITIAL_HASH_TABLE_SIZE 32 // Initial size of the hash table
#define LOAD_FACTOR(size) (size / 4 * 3) // Load factor for resizing the hash table (3/4 or 0.75)

#define NEXT_ENTRY(variable_name) if (variable_name.next == NULL) { \
    break; \
} \
variable_name = *(ValueObjectHashEntry*)variable_name.next;

typedef struct {
    bool is_used;           // Flag to indicate if the entry is used
    uint64_t hash;          // Hash value of the key
    Value keyValue;         // Key value
    Value value;            // Value associated with the key
    void* next;             // Pointer to the next entry in case of a collision
} ValueObjectHashEntry;

typedef struct {
    ValueObjectHashEntry* entries; // Array of pointers to HashEntry
    size_t count;                   // Number of entries in the hash table
    size_t size;                    // Size of the hash table
} ValueObjectHashTable;

typedef ValueObjectHashTable ValueObject;

void init_ValueObjectHashTable(ValueObjectHashTable* table);
void free_ValueObjectHashTable(ValueObjectHashTable* table);
void write_ValueObjectHashTable(ValueObjectHashTable* table, Value key, Value value);
Value* getValueAtKey(ValueObjectHashTable* table, Value key);
Value* getValueAtKeyHash(ValueObjectHashTable* table, uint64_t key);
bool hasKey(ValueObjectHashTable* table, Value key);
bool hasKeyHash(ValueObjectHashTable* table, uint64_t key);
void removeFromKey(ValueObjectHashTable* table, Value key);
void removeFromKeyHash(ValueObjectHashTable* table, uint64_t key);

void growValueObjectHashTable(ValueObjectHashTable* table);

uint64_t hashValue(Value value);

uint64_t hashString(const char* str);
uint64_t hashValueArray(ValueArray* array);

#endif // dosato_hash_h