#include "hash.h"
#include "value.h"
#include "common.h"

uint64_t hashValue(Value value) {
    uint64_t hash = ((uint64_t)-1) >> 1; // max value for uint64_t
    switch (value.type) {
        case D_NULL: {
            return 0;
        }
        case TYPE_BOOL: {
            return AS_BOOL(value) ? 2 : 1;
        }

        case TYPE_CHAR: {
            return AS_CHAR(value);
        }

        // number cases are raw index hash
        case TYPE_BYTE: {
            hash -= (uint64_t)AS_BYTE(value);
            break;
        }
        case TYPE_UBYTE: {
            hash -= (uint64_t)AS_UBYTE(value);
            break;
        }
        case TYPE_SHORT: {
            hash -= (uint64_t)AS_SHORT(value);
            break;
        }
        case TYPE_USHORT: {
            hash -= (uint64_t)AS_USHORT(value);
            break;
        }
        case TYPE_INT: {
            hash -= (uint64_t)AS_INT(value);
            break;
        }
        case TYPE_UINT: {
            hash -= (uint64_t)AS_UINT(value);
            break;
        }
        case TYPE_LONG: {
            hash -= (uint64_t)AS_LONG(value);
            break;
        }
        case TYPE_ULONG: {
            hash -= (uint64_t)AS_ULONG(value);
            break;
        }

        case TYPE_FLOAT: {
            // map 32 bit float to int
            union {
                float f;
                uint32_t i;
            } u;
            u.f = AS_FLOAT(value);
            hash -= u.i;
            break;
        }
        case TYPE_DOUBLE: {
            // map 64 bit double to int
            union {
                double d;
                uint64_t i;
            } u;
            u.d = AS_DOUBLE(value);
            hash -= u.i;
            break;
        }

        case TYPE_STRING: {
            return hashString(AS_STRING(value));
        }

        case TYPE_ARRAY: {
            return hashValueArray(AS_ARRAY(value));
        }

        case TYPE_OBJECT: {
            // hash object pointer
            return (uint64_t)value.as.objectValue;
        }
    }

    return hash;
}

uint64_t hashString(const char* str) {
    uint64_t hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + *str; // hash * 33 + c
        str++;
    }
    return hash;
}

uint64_t hashValueArray(ValueArray* array) {
    uint64_t hash = 5381;
    for (size_t i = 0; i < array->count; i++) {
        hash = ((hash << 5) + hash) + hashValue(array->values[i]);
    }
    return hash;
}

void init_ValueObjectHashTable(ValueObjectHashTable* table) {
    table->entries = malloc(INITIAL_HASH_TABLE_SIZE * sizeof(ValueObjectHashEntry));
    for (size_t i = 0; i < INITIAL_HASH_TABLE_SIZE; i++) {
        table->entries[i].is_used = false;
    }
    table->count = 0;
    table->size = INITIAL_HASH_TABLE_SIZE;
}
void freeNextEntry(ValueObjectHashEntry* entry);
void freeNextEntry(ValueObjectHashEntry* entry) {
    if (entry->next != NULL) {
        freeNextEntry(entry->next);
        free(entry->next);
        entry->next = NULL;
    }
}

void free_ValueObjectHashTable(ValueObjectHashTable* table) {
    for (size_t i = 0; i < table->size; i++) {
        if (table->entries[i].is_used) {
            freeNextEntry(&table->entries[i]);
        }
    }
    free(table->entries);
}

void write_ValueObjectHashTable(ValueObjectHashTable* table, Value key, Value value) {
    uint64_t hash = hashValue(key);
    size_t index = hash % table->size;

    // Check if the key already exists
    ValueObjectHashEntry* entry = &table->entries[index];
    while (entry->is_used) {
        if (entry->hash == hash) {
            // Key already exists, update the value
            entry->value = value;
            return;
        }

        if (entry->next == NULL) {
            break; // No more entries in this chain
        }
        entry = (ValueObjectHashEntry*)entry->next;
    }

    // If we reach here, we need to add a new entry
    if (table->count >= LOAD_FACTOR(table->size)) {
        // Resize the table if necessary
        growValueObjectHashTable(table);
        index = hash % table->size; // Recalculate index after resizing
    }

    // add the new entry
    entry = &table->entries[index];
    while (entry->is_used) {
        if (entry->next == NULL) {
            entry->next = malloc(sizeof(ValueObjectHashEntry));
            entry = (ValueObjectHashEntry*)entry->next;
            break; // No more entries in this chain
        }
        entry = (ValueObjectHashEntry*)entry->next;
    }

    entry->is_used = true;
    entry->hash = hash;
    entry->keyValue = key; // Store the key value
    entry->value = value; // Store the value
    entry->next = NULL; // Initialize next pointer to NULL
    table->count++;
}

void growValueObjectHashTable(ValueObjectHashTable* table) {
    size_t newSize = table->size * 2;
    ValueObjectHashEntry* newEntries = malloc(newSize * sizeof(ValueObjectHashEntry));
    for (size_t i = 0; i < newSize; i++) {
        newEntries[i].is_used = false;
    }
    for (size_t i = 0; i < table->size; i++) {
        if (!table->entries[i].is_used) continue; // Skip unused entries
        ValueObjectHashEntry oldEntry = table->entries[i];
        while (true) {
            size_t newIndex = oldEntry.hash % newSize;
            ValueObjectHashEntry* newEntry = &newEntries[newIndex];
            while (newEntry->is_used) {
                if (newEntry->next == NULL) {
                    newEntry->next = malloc(sizeof(ValueObjectHashEntry));
                    newEntry = (ValueObjectHashEntry*)newEntry->next;
                    newEntry->is_used = false; // Initialize new entry
                    break; // No more entries in this chain
                }
                newEntry = (ValueObjectHashEntry*)newEntry->next;
            }

            if (!newEntry->is_used) {
                newEntry->is_used = true;
                newEntry->hash = oldEntry.hash;
                newEntry->value = oldEntry.value;
                newEntry->keyValue = oldEntry.keyValue; // Copy key value
                newEntry->next = NULL; // Initialize next pointer
            }

            NEXT_ENTRY(oldEntry);
        }
    }

    free_ValueObjectHashTable(table);
    table->entries = newEntries;
    table->size = newSize;
}

Value* getValueAtKey(ValueObjectHashTable* table, Value key) {
    return getValueAtKeyHash(table, hashValue(key));
}

Value* getValueAtKeyHash(ValueObjectHashTable* table, uint64_t key) {
    size_t index = key % table->size;
    ValueObjectHashEntry* entry = &table->entries[index];

    while (entry->is_used) {
        if (entry->hash == key) {
            return &entry->value; // Return the value associated with the key
        }
        if (entry->next == NULL) {
            break; // No more entries in this chain
        }
        entry = (ValueObjectHashEntry*)entry->next;
    }

    return NULL; // Key not found
}

bool hasKey(ValueObjectHashTable* table, Value key) {
    return hasKeyHash(table, hashValue(key));
}

bool hasKeyHash(ValueObjectHashTable* table, uint64_t key) {
    size_t index = key % table->size;
    ValueObjectHashEntry* entry = &table->entries[index];

    while (entry->is_used) {
        if (entry->hash == key) {
            return true; // Key found
        }
        if (entry->next == NULL) {
            break; // No more entries in this chain
        }
        entry = (ValueObjectHashEntry*)entry->next;
    }

    return false; // Key not found
}

void removeFromKey(ValueObjectHashTable* table, Value key) {
    removeFromKeyHash(table, hashValue(key));
}

void removeFromKeyHash(ValueObjectHashTable* table, uint64_t key) {
    size_t index = key % table->size;
    ValueObjectHashEntry* entry = &table->entries[index];

    while (entry->is_used) {
        if (entry->hash == key) {
            entry->is_used = false; // Mark the entry as unused
            table->count--;
            if (entry->next != NULL) {
                // If there is a next entry, we need to handle the chain
                ValueObjectHashEntry* nextEntry = (ValueObjectHashEntry*)entry->next;
                *entry = *nextEntry; // Copy the next entry to the current one
                free(nextEntry); // Free the old next entry
            }
            return; // Key found and removed
        }
        if (entry->next == NULL) {
            break; // No more entries in this chain
        }
        entry = (ValueObjectHashEntry*)entry->next;
    }

    // Key not found, do nothing
}