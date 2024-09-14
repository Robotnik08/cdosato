#include "../include/common.h"

#include "../include/memory.h"
#include "../include/value.h"

DOSATO_LIST_FUNC_GEN(ValueArray, Value, values)
DOSATO_LIST_FUNC_GEN(StackFrames, size_t, stack)
DOSATO_LIST_FUNC_GEN(ErrorJumps, ErrorJump, jumps)

void destroyValueArray(ValueArray* array) {
    for (size_t i = 0; i < array->count; i++) {
        destroyValue(&array->values[i]);
    }
    free_ValueArray(array);
}

void destroyValue(Value* value) {
    
    if (value->type == TYPE_ARRAY) {
        ValueArray* array = value->as.objectValue;
        destroyValueArray(array);
        free(value->as.objectValue);
        return;
    }

    if (value->type == TYPE_OBJECT) {
        ValueObject* object = value->as.objectValue;
        free_ValueObject(object);
        free(value->as.objectValue); // free the object
    } else if (value->type == TYPE_STRING) {
        free(value->as.stringValue);
    }
    value->defined = false;
    value->type = D_NULL;
}

void printValue(Value value, bool extensive) {
    char* str = valueToString (value, extensive);
    printf(str);
    free(str);
}

Value hardCopyValue(Value value) {
    value.defined = false;
    switch (value.type) {
        case TYPE_ARRAY: {
            ValueArray* array = value.as.objectValue;
            ValueArray* newArray = malloc(sizeof(ValueArray));
            init_ValueArray(newArray);
            for (size_t i = 0; i < array->count; i++) {
                write_ValueArray(newArray, hardCopyValue(array->values[i]));
            }
            value.as.objectValue = newArray;
            break;
        }
        case TYPE_OBJECT: {
            ValueObject* object = value.as.objectValue;
            ValueObject* newObject = malloc(sizeof(ValueObject));
            init_ValueObject(newObject);
            for (size_t i = 0; i < object->count; i++) {
                write_ValueObject(newObject, object->keys[i], hardCopyValue(object->values[i]));
            }
            value.as.objectValue = newObject;
            break;
        }
        case TYPE_STRING: {
            char* new_str = malloc(strlen(value.as.stringValue) + 1);
            strcpy(new_str, value.as.stringValue);
            value.as.stringValue = new_str;
            break;
        }
        default: {
            break;
        }
    }
    return value;
}

bool valueEquals (Value* a, Value* b) {
    if (a->type == TYPE_FUNCTION || b->type == TYPE_FUNCTION) {
        return false; // can't compare functions
    }
    
    if (ISFLOATTYPE(a->type) || ISFLOATTYPE(b->type)) {
        double aValue = 0;
        double bValue = 0;

        ErrorType aError = castValue(a, TYPE_DOUBLE);
        if (aError != 0) {
            return false;
        }
        aValue = a->as.doubleValue;
        ErrorType bError = castValue(b, TYPE_DOUBLE);
        if (bError != 0) {
            return false;
        }
        bValue = b->as.doubleValue;

        return aValue == bValue;

    } else if (a->type == TYPE_STRING || b->type == TYPE_STRING) {
        char* aStr = valueToString(*a, false);
        char* bStr = valueToString(*b, false);
        bool result = strcmp(aStr, bStr) == 0;
        free(aStr);
        free(bStr);
        return result;
    } else if ((ISINTTYPE(a->type) || a->type == TYPE_CHAR || a->type == TYPE_BOOL) || (ISINTTYPE(b->type) || b->type == TYPE_CHAR || b->type == TYPE_BOOL)) {
        long long int aValue = 0;
        long long int bValue = 0;

        ErrorType aError = castValue(a, TYPE_LONG);
        if (aError != 0) {
            return false;
        }
        aValue = a->as.longValue;
        ErrorType bError = castValue(b, TYPE_LONG);
        if (bError != 0) {
            return false;
        }
        bValue = b->as.longValue;

        return aValue == bValue;
    } else if (a->type == TYPE_OBJECT && b->type == TYPE_OBJECT) {
        ValueObject* aObject = a->as.objectValue;
        ValueObject* bObject = b->as.objectValue;

        if (aObject->count != bObject->count) {
            return false;
        }

        for (size_t i = 0; i < aObject->count; i++) {
            if (strcmp(aObject->keys[i], bObject->keys[i]) != 0) {
                return false;
            }
            if (!valueEquals(&aObject->values[i], &bObject->values[i])) {
                return false;
            }
        }

        return true;
    } else if (a->type == TYPE_ARRAY && b->type == TYPE_ARRAY) {
        ValueArray* aArray = a->as.objectValue;
        ValueArray* bArray = b->as.objectValue;

        if (aArray->count != bArray->count) {
            return false;
        }

        for (size_t i = 0; i < aArray->count; i++) {
            if (!valueEquals(&aArray->values[i], &bArray->values[i])) {
                return false;
            }
        }

        return true;
    }
    return false;
}

ErrorType castValue(Value* value, DataType type) {
    if (value->type == type) {
        return 0;
    }

    if (type == TYPE_VAR) {
        value->is_variable_type = true;
        return 0; // no need to cast to var
    }

    if (type == TYPE_OBJECT || type == TYPE_FUNCTION) {
        return E_CANT_CONVERT_TO_OBJECT;
    }

    if (type == TYPE_ARRAY) {
        return E_CANT_CONVERT_TO_ARRAY;
    }

    Value newValue = UNDEFINED_VALUE;

    if (ISINTTYPE(type) || type == TYPE_CHAR) {
        long long int numberValue = 0;
        switch (value->type) {
            case TYPE_UBYTE: {
                numberValue = value->as.ubyteValue;
                break;
            }
            case TYPE_BYTE: {
                numberValue = value->as.byteValue;
                break;
            }
            case TYPE_USHORT: {
                numberValue = value->as.ushortValue;
                break;
            }
            case TYPE_SHORT: {
                numberValue = value->as.shortValue;
                break;
            }
            case TYPE_UINT: {
                numberValue = value->as.uintValue;
                break;
            }
            case TYPE_INT: {
                numberValue = value->as.intValue;
                break;
            }
            case TYPE_ULONG: {
                numberValue = value->as.ulongValue;
                break;
            }
            case TYPE_LONG: {
                numberValue = value->as.longValue;
                break;
            }
            case TYPE_FLOAT: {
                numberValue = (long long int)value->as.floatValue;
                break;
            }
            case TYPE_DOUBLE: {
                numberValue = (long long int)value->as.doubleValue;
                break;
            }
            case TYPE_BOOL: {
                numberValue = value->as.boolValue ? 1 : 0;
                break;
            }
            case TYPE_CHAR: {
                numberValue = value->as.charValue;
                break;
            }
            case TYPE_STRING: {
                char* str = value->as.stringValue;
                numberValue = strlen(str);
                break;
            }
            case TYPE_ARRAY: {
                numberValue = ((ValueArray*)value->as.objectValue)->count;
                break;
            }

            case TYPE_OBJECT: {
                return E_CANT_CONVERT_TO_INT;
            }
            default: {
                return E_CANT_CONVERT_TO_INT;
            }
        }


        newValue.type = type;
        
        switch (type) {
            case TYPE_UBYTE: {
                newValue.as.ubyteValue = (uint8_t)numberValue;
                break;
            }
            case TYPE_BYTE: {
                newValue.as.byteValue = (int8_t)numberValue;
                break;
            }
            case TYPE_USHORT: {
                newValue.as.ushortValue = (uint16_t)numberValue;
                break;
            }
            case TYPE_SHORT: {
                newValue.as.shortValue = (int16_t)numberValue;
                break;
            }
            case TYPE_UINT: {
                newValue.as.uintValue = (uint32_t)numberValue;
                break;
            }
            case TYPE_INT: {
                newValue.as.intValue = (int32_t)numberValue;
                break;
            }
            case TYPE_ULONG: {
                newValue.as.ulongValue = (uint64_t)numberValue;
                break;
            }
            case TYPE_LONG: {
                newValue.as.longValue = (int64_t)numberValue;
                break;
            }
            case TYPE_CHAR: {
                newValue.as.charValue = (char)numberValue;
                break;
            }
            default: {
                return E_CANT_CONVERT_TO_INT;
            }
        }

    } else if (ISFLOATTYPE(type) || type == TYPE_BOOL) {
        double numberValue = 0;
        switch (value->type) {
            case TYPE_UBYTE: {
                numberValue = value->as.ubyteValue;
                break;
            }
            case TYPE_BYTE: {
                numberValue = value->as.byteValue;
                break;
            }
            case TYPE_USHORT: {
                numberValue = value->as.ushortValue;
                break;
            }
            case TYPE_SHORT: {
                numberValue = value->as.shortValue;
                break;
            }
            case TYPE_UINT: {
                numberValue = value->as.uintValue;
                break;
            }
            case TYPE_INT: {
                numberValue = value->as.intValue;
                break;
            }
            case TYPE_ULONG: {
                numberValue = value->as.ulongValue;
                break;
            }
            case TYPE_LONG: {
                numberValue = value->as.longValue;
                break;
            }
            case TYPE_FLOAT: {
                numberValue = value->as.floatValue;
                break;
            }
            case TYPE_DOUBLE: {
                numberValue = value->as.doubleValue;
                break;
            }
            case TYPE_BOOL: {
                numberValue = value->as.boolValue ? 1 : 0;
                break;
            }
            case TYPE_CHAR: {
                numberValue = value->as.charValue;
                break;
            }
            case TYPE_STRING: {
                char* str = value->as.stringValue;
                numberValue = strlen(str);
                break;
            }
            case TYPE_ARRAY: {
                numberValue = ((ValueArray*)value->as.objectValue)->count;
                break;
            }
            case TYPE_OBJECT: {
                return E_CANT_CONVERT_TO_FLOAT;
            }
            default: {
                return E_CANT_CONVERT_TO_FLOAT;
            }
        }

        newValue.type = type;

        switch (type) {
            case TYPE_FLOAT: {
                newValue.as.floatValue = (float)numberValue;
                break;
            }
            case TYPE_DOUBLE: {
                newValue.as.doubleValue = (double)numberValue;
                break;
            }
            case TYPE_BOOL: {
                newValue.as.boolValue = numberValue != 0;
                break;
            }
            default: {
                return E_CANT_CONVERT_TO_FLOAT;
            }
        }
    } else if (type == TYPE_STRING) {
        char* str = valueToString(*value, false);
        newValue.type = type;
        newValue.as.stringValue = str;
    }

    destroyValue(value);
    *value = newValue;

    return 0;
}

ErrorType incValue (Value* value, int amount) {
    DataType type = value->type;

    if (ISINTTYPE(type) || ISFLOATTYPE(type) || type == TYPE_CHAR) {
        switch (type) {
            case TYPE_UBYTE: {
                value->as.ubyteValue += amount;
                break;
            }
            case TYPE_BYTE: {
                value->as.byteValue += amount;
                break;
            }
            case TYPE_USHORT: {
                value->as.ushortValue += amount;
                break;
            }
            case TYPE_SHORT: {
                value->as.shortValue += amount;
                break;
            }
            case TYPE_UINT: {
                value->as.uintValue += amount;
                break;
            }
            case TYPE_INT: {
                value->as.intValue += amount;
                break;
            }
            case TYPE_ULONG: {
                value->as.ulongValue += amount;
                break;
            }
            case TYPE_LONG: {
                value->as.longValue += amount;
                break;
            }
            case TYPE_FLOAT: {
                value->as.floatValue += amount;
                break;
            }
            case TYPE_DOUBLE: {
                value->as.doubleValue += amount;
                break;
            }
            case TYPE_CHAR: {
                value->as.charValue += amount;
                break;
            }
        }
    } else {
        return E_CANT_PERFORM_OPERATION;
    }
    return 0;
}

Value buildArray(size_t count, ...) {
    ValueArray* array = malloc(sizeof(ValueArray));
    init_ValueArray(array);

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i++) {
        Value value = va_arg(args, Value);
        write_ValueArray(array, value);
    }

    va_end(args);

    return (Value) { TYPE_ARRAY, .as.objectValue = array, .defined = false };
}

Value buildObject(size_t count, ...) {
    ValueObject* object = malloc(sizeof(ValueObject));
    init_ValueObject(object);

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i++) {
        char* key = va_arg(args, char*);
        Value value = va_arg(args, Value);
        write_ValueObject(object, key, value);
    }

    va_end(args);

    return (Value) { TYPE_OBJECT, .as.objectValue = object, .defined = false };
}



char* valueToString (Value value, bool extensive) {
    char* string = malloc(1);
    string[0] = '\0';

    switch (value.type) {
        case TYPE_OBJECT: {
            string = realloc(string, strlen(string) + 2);
            strcat(string, "{");
            ValueObject* object = value.as.objectValue;
            for (size_t i = 0; i < object->count; i++) {
                string = realloc(string, strlen(string) + strlen(object->keys[i]) + 5);
                strcat(string, "\"");
                strcat(string, object->keys[i]);
                strcat(string, "\": ");
                char* valueString = valueToString(object->values[i], true);
                string = realloc(string, strlen(string) + strlen(valueString) + 3);
                strcat(string, valueString);
                if (i < object->count - 1) {
                    strcat(string, ", ");
                }
                free(valueString);
            }
            string = realloc(string, strlen(string) + 2);
            strcat(string, "}");
            break;
        }

        case TYPE_ARRAY: {
            string = realloc(string, strlen(string) + 2);
            strcat(string, "[");
            ValueArray* array = value.as.objectValue;
            for (size_t i = 0; i < array->count; i++) {
                char* valueString = valueToString(array->values[i], true);
                string = realloc(string, strlen(string) + strlen(valueString) + 3);
                strcat(string, valueString);
                if (i < array->count - 1) {
                    strcat(string, ", ");
                }
                free(valueString);
            }
            string = realloc(string, strlen(string) + 2);
            strcat(string, "]");
            break;
        }

        case TYPE_FUNCTION: {
            string = realloc(string, strlen(string) + 8);
            strcat(string, "<function>");
            break;
        }

        case TYPE_UBYTE: {
            string = realloc(string, 8);
            sprintf(string, "%u", value.as.ubyteValue);
            break;
        }
        case TYPE_BYTE: {
            string = realloc(string, 8);
            sprintf(string, "%d", value.as.byteValue);
            break;
        }
        case TYPE_USHORT: {
            string = realloc(string, 8);
            sprintf(string, "%u", value.as.ushortValue);
            break;
        }
        case TYPE_SHORT: {
            string = realloc(string, 8);
            sprintf(string, "%d", value.as.shortValue);
            break;
        }
        case TYPE_UINT: {
            string = realloc(string, 16);
            sprintf(string, "%u", value.as.uintValue);
            break;
        }
        case TYPE_INT: {
            string = realloc(string, 16);
            sprintf(string, "%d", value.as.intValue);
            break;
        }
        case TYPE_ULONG: {
            string = realloc(string, 24);
            sprintf(string, "%llu", value.as.ulongValue);
            break;
        }
        case TYPE_LONG: {
            string = realloc(string, 24);
            sprintf(string, "%lld", value.as.longValue);
            break;
        }
        case TYPE_STRING: {
            if (extensive) {
                string = realloc(string, strlen(string) + strlen(value.as.stringValue) + 3);
                strcat(string, "\"");
                strcat(string, value.as.stringValue);
                strcat(string, "\"");
            } else {
                string = realloc(string, strlen(string) + strlen(value.as.stringValue) + 1);
                strcat(string, value.as.stringValue);
            }
            break;
        }
        case TYPE_CHAR: {
            if (extensive) {
                string = realloc(string, 5);
                sprintf(string, "'%c'", value.as.charValue);
            } else {
                string = realloc(string, 2);
                sprintf(string, "%c", value.as.charValue);
            }
            break;
        }
        case TYPE_FLOAT: {
            string = realloc(string, 32);
            sprintf(string, "%f", value.as.floatValue);
            break;
        }
        case TYPE_DOUBLE: {
            string = realloc(string, 32);
            sprintf(string, "%f", value.as.doubleValue);
            break;
        }
        case TYPE_BOOL: {
            string = realloc(string, 6);
            sprintf(string, "%s", value.as.boolValue ? "TRUE" : "FALSE");
            break;
        }

        case D_NULL: {
            string = realloc(string, 6);
            sprintf(string, "NULL");
            break;
        }

        default: {
            string = realloc(string, 16);
            sprintf(string, "<unknown type>");
            break;
        }
    }

    return string;
}

char* dataTypeToString (DataType type) {
    switch (type) {
        case TYPE_INT: {
            return "int";
        }
        case TYPE_BOOL: {
            return "bool";
        }
        case TYPE_STRING: {
            return "string";
        }
        case TYPE_FLOAT: {
            return "float";
        }
        case TYPE_DOUBLE: {
            return "double";
        }
        case TYPE_CHAR: {
            return "char";
        }
        case TYPE_SHORT: {
            return "short";
        }
        case TYPE_LONG: {
            return "long";
        }
        case TYPE_BYTE: {
            return "byte";
        }
        case TYPE_VOID: {
            return "void";
        }
        case TYPE_ARRAY: {
            return "array";
        }
        case TYPE_UINT: {
            return "uint";
        }
        case TYPE_USHORT: {
            return "ushort";
        }
        case TYPE_ULONG: {
            return "ulong";
        }
        case TYPE_UBYTE: {
            return "ubyte";
        }
        case TYPE_OBJECT: {
            return "object";
        }
        case TYPE_VAR: {
            return "var";
        }
        case TYPE_FUNCTION: {
            return "function";
        }
        case TYPE_EXCEPTION: {
            return "exception";
        }
        case TYPE_HLT: {
            return "hlt";
        }
        default: {
            return "<unknown type>";
        }
    }
}

void markDefined(Value* value) {
    value->defined = true;
    if (value->type == TYPE_ARRAY) {
        ValueArray* array = value->as.objectValue;
        for (size_t i = 0; i < array->count; i++) {
            markDefined(&array->values[i]);
        }
    }

    if (value->type == TYPE_OBJECT) {
        ValueObject* object = value->as.objectValue;
        for (size_t i = 0; i < object->count; i++) {
            markDefined(&object->values[i]);
        }
    }
}


void init_NameMap(NameMap* map) {
    map->names = NULL;
    map->count = 0;
    map->capacity = 0;
}

void write_NameMap(NameMap* map, char* name) {
    if (map->capacity < map->count + 1) {
        size_t oldCapacity = map->capacity;
        map->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        map->names = DOSATO_RESIZE_LIST(char*, map->names, oldCapacity, map->capacity);
    }
    map->names[map->count] = name;
    map->count++;
}

void free_NameMap(NameMap* map) {
    for (size_t i = 0; i < map->count; i++) {
        free(map->names[i]);
    }
    free(map->names);
    init_NameMap(map);
}

bool hasName(NameMap* map, char* name) {
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->names[i], name) == 0) {
            return true;
        }
    }
    return false;
}

size_t getName(NameMap* map, char* name) {
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->names[i], name) == 0) {
            return i;
        }
    }
    return 0;
}

size_t addName(NameMap* map, char* name) {
    if (hasName(map, name)) {
        return 0;
    }
    char* newName = malloc(strlen(name) + 1);
    strcpy(newName, name);
    write_NameMap(map, newName);
    return map->count - 1;
}

void init_ValueObject(ValueObject* object) {
    object->values = NULL;
    object->keys = NULL;
    object->count = 0;
    object->capacity = 0;
}

void write_ValueObject(ValueObject* object, char* key, Value value) {
    if (object->capacity < object->count + 1) {
        size_t oldCapacity = object->capacity;
        object->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        object->values = DOSATO_RESIZE_LIST(Value, object->values, oldCapacity, object->capacity);
        object->keys = DOSATO_RESIZE_LIST(char*, object->keys, oldCapacity, object->capacity);
    }
    object->values[object->count] = value;
    // Copy the key
    char* newKey = malloc(strlen(key) + 1);
    strcpy(newKey, key);
    object->keys[object->count] = newKey;
    object->count++;
}

void free_ValueObject(ValueObject* object) {
    for (size_t i = 0; i < object->count; i++) {
        free(object->keys[i]);
        destroyValue(&object->values[i]);
    }
    free(object->values);
    free(object->keys);
    init_ValueObject(object);
}

bool hasKey(ValueObject* object, char* key) {
    for (size_t i = 0; i < object->count; i++) {
        if (strcmp(object->keys[i], key) == 0) {
            return true;
        }
    }
    return false;
}

Value* getValueAtKey(ValueObject* object, char* key) {
    for (size_t i = 0; i < object->count; i++) {
        if (strcmp(object->keys[i], key) == 0) {
            return &object->values[i];
        }
    }
    return NULL;
}

void removeFromKey (ValueObject* object, char* key) {
    for (size_t i = 0; i < object->count; i++) {
        if (strcmp(object->keys[i], key) == 0) {
            free(object->keys[i]);
            destroyValue(&object->values[i]);
            for (size_t j = i; j < object->count - 1; j++) {
                object->keys[j] = object->keys[j + 1];
                object->values[j] = object->values[j + 1];
            }
            object->count--;
            return;
        }
    }
}