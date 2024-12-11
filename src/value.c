#include "../include/common.h"

#include "../include/memory.h"
#include "../include/value.h"
#include "../include/virtual-machine.h"

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
        ValueArray* array = AS_ARRAY(*value);
        destroyValueArray(array);
        return;
    }

    if (value->type == TYPE_OBJECT) {
        ValueObject* object = AS_OBJECT(*value);
        free_ValueObject(object);
    } else if (value->type == TYPE_STRING) {
        free(AS_STRING(*value));
    }
    value->defined = false;
    value->type = D_NULL;
}

void printValue(Value value, bool extensive) {
    char* str = valueToString (value, extensive);
    printf("%s", str);
    free(str);
}

Value hardCopyValue(Value value) {
    DosatoObject** pointers = malloc(sizeof(DosatoObject**));
    Value result = hardCopyValueSafe(value, &pointers, 0);
    free(pointers);
    return result;
}

Value hardCopyValueSafe (Value value, DosatoObject*** pointers, int count) {
    value.defined = false;
    switch (value.type) {
        case TYPE_ARRAY: {
            for (int i = 0; i < count; i++) {
                if ((*pointers)[i] == value.as.objectValue) {
                    return BUILD_NULL();
                }
            }
            (*pointers) = realloc(*pointers, sizeof(DosatoObject**) * (count + 1));
            (*pointers)[count++] = value.as.objectValue;

            ValueArray* array = AS_ARRAY(value);
            ValueArray* newArray = malloc(sizeof(ValueArray));
            init_ValueArray(newArray);
            for (size_t i = 0; i < array->count; i++) {
                write_ValueArray(newArray, hardCopyValueSafe(array->values[i], pointers, count));
            }
            value = BUILD_ARRAY(newArray, false);
            break;
        }
        case TYPE_OBJECT: {
            for (int i = 0; i < count; i++) {
                if ((*pointers)[i] == value.as.objectValue) {
                    return BUILD_NULL();
                }
            }
            (*pointers) = realloc(*pointers, sizeof(DosatoObject**) * (count + 1));
            (*pointers)[count++] = value.as.objectValue;

            ValueObject* object = AS_OBJECT(value);
            ValueObject* newObject = malloc(sizeof(ValueObject));
            init_ValueObject(newObject);
            for (size_t i = 0; i < object->count; i++) {
                write_ValueObject(newObject, object->keys[i], hardCopyValueSafe(object->values[i], pointers, count));
            }
            value = BUILD_OBJECT(newObject, false);
            break;
        }
        case TYPE_STRING: {
            value = BUILD_STRING(COPY_STRING(AS_STRING(value)), false);
            break;
        }
        default: {
            return value;
        }
    }
    return value;
}

bool valueEqualsStrict (Value* a, Value* b) {
    if (a->type != b->type) {
        return false;
    }

    return valueEquals(a, b);
}

bool valueEquals (Value* aPtr, Value* bPtr) {
    if (aPtr->type == D_NULL && bPtr->type == D_NULL) {
        return true;
    }

    if (aPtr->type == TYPE_FUNCTION || bPtr->type == TYPE_FUNCTION) {
        return false; // can't compare functions
    }

    Value a = *aPtr;
    Value b = *bPtr;
    
    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        double aValue = 0;
        double bValue = 0;

        ErrorType aError = castValue(&a, TYPE_DOUBLE);
        if (aError != 0) {
            return false;
        }
        aValue = a.as.doubleValue;
        ErrorType bError = castValue(&b, TYPE_DOUBLE);
        if (bError != 0) {
            return false;
        }
        bValue = b.as.doubleValue;

        return aValue == bValue;

    } else if (a.type == TYPE_STRING || b.type == TYPE_STRING) {
        char* aStr = valueToString(a, false);
        char* bStr = valueToString(b, false);
        bool result = strcmp(aStr, bStr) == 0;
        free(aStr);
        free(bStr);
        return result;
    } else if ((ISINTTYPE(a.type) || a.type == TYPE_CHAR || a.type == TYPE_BOOL) || (ISINTTYPE(b.type) || b.type == TYPE_CHAR || b.type == TYPE_BOOL)) {
        long long int aValue = 0;
        long long int bValue = 0;

        ErrorType aError = castValue(&a, TYPE_LONG);
        if (aError != 0) {
            return false;
        }
        aValue = a.as.longValue;
        ErrorType bError = castValue(&b, TYPE_LONG);
        if (bError != 0) {
            return false;
        }
        bValue = b.as.longValue;

        return aValue == bValue;
    } else if (a.type == TYPE_OBJECT && b.type == TYPE_OBJECT) {
        ValueObject* aObject = AS_OBJECT(a);
        ValueObject* bObject = AS_OBJECT(b);

        if (aObject->count != bObject->count) {
            return false;
        }

        for (size_t i = 0; i < aObject->count; i++) {
            Value key = aObject->keys[i];
            if (!hasKey(bObject, key)) {
                return false;
            }
            Value* val = getValueAtKey(bObject, key);

            Value a = aObject->values[i];
            Value b = *val;
            if (!valueEquals(&a, &b)) {
                return false;
            }
        }

        return true;
    } else if (a.type == TYPE_ARRAY && b.type == TYPE_ARRAY) {
        ValueArray* aArray = AS_ARRAY(a);
        ValueArray* bArray = AS_ARRAY(b);

        if (aArray->count != bArray->count) {
            return false;
        }

        for (size_t i = 0; i < aArray->count; i++) {
            Value a = aArray->values[i];
            Value b = bArray->values[i];
            if (!valueEquals(&a, &b)) {
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
            case D_NULL: {
                numberValue = 0;
                break;
            }

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
                char* str = AS_STRING(*value);
                numberValue = strlen(str);
                break;
            }
            case TYPE_ARRAY: {
                numberValue = AS_ARRAY(*value)->count;
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
            case D_NULL: {
                numberValue = 0.0;
                break;
            }

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
                char* str = AS_STRING(*value);
                numberValue = strlen(str);
                break;
            }
            case TYPE_ARRAY: {
                numberValue = AS_ARRAY(*value)->count;
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
        newValue = BUILD_STRING(str, false);
    }


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
            default: {
                return E_CANT_PERFORM_OPERATION;
            }
        }
    } else {
        return E_CANT_PERFORM_OPERATION;
    }
    return 0;
}

ValueArray* buildArray(size_t count, ...) {
    ValueArray* array = malloc(sizeof(ValueArray));
    init_ValueArray(array);

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i++) {
        Value value = va_arg(args, Value);
        write_ValueArray(array, value);
    }

    va_end(args);

    return array;
}

ValueObject* buildObject(size_t count, ...) {
    ValueObject* object = malloc(sizeof(ValueObject));
    init_ValueObject(object);

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i++) {
        Value key = va_arg(args, Value);
        Value value = va_arg(args, Value);
        write_ValueObject(object, key, value);
    }

    va_end(args);

    return object;
}

char* valueToString (Value value, bool extensive) {
    DosatoObject** pointers = malloc(sizeof(DosatoObject**));
    char* str = valueToStringSafe(value, extensive, &pointers, 0);
    free(pointers);
    return str;
}

char* valueToStringSafe (Value value, bool extensive, DosatoObject*** pointers, int count) {
    char* string = malloc(1);
    string[0] = '\0';

    switch (value.type) {
        case TYPE_OBJECT: {
            for (size_t i = 0; i < count; i++) {
                if ((*pointers)[i] == value.as.objectValue) {
                    string = realloc(string, strlen(string) + 10);
                    strcat(string, "{...}");
                    return string;
                }
            }
            (*pointers) = realloc(*pointers, (count + 1) * sizeof(DosatoObject**));
            (*pointers)[count++] = value.as.objectValue;

            string = realloc(string, strlen(string) + 2);
            strcat(string, "{");
            ValueObject* object = AS_OBJECT(value);
            for (size_t i = 0; i < object->count; i++) {
                char* keyString = valueToStringSafe(object->keys[i], true, pointers, count);
                string = realloc(string, strlen(string) + strlen(keyString) + 10);
                strcat(string, keyString);
                strcat(string, ": ");
                char* valueString = valueToStringSafe(object->values[i], true, pointers, count);
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
            for (size_t i = 0; i < count; i++) {
                if ((*pointers)[i] == value.as.objectValue) {
                    string = realloc(string, strlen(string) + 10);
                    strcat(string, "[...]");
                    return string;
                }
            }
            (*pointers) = realloc(*pointers, (count + 1) * sizeof(DosatoObject**));
            (*pointers)[count++] = value.as.objectValue;

            string = realloc(string, strlen(string) + 2);
            strcat(string, "[");
            ValueArray* array = AS_ARRAY(value);
            for (size_t i = 0; i < array->count; i++) {
                char* valueString = valueToStringSafe(array->values[i], true, pointers, count);
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
            string = realloc(string, strlen(string) + 16);
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
                char* str = AS_STRING(value);
                string = realloc(string, strlen(string) + strlen(str) + 3);
                strcat(string, "\"");
                strcat(string, AS_STRING(value));
                strcat(string, "\"");
            } else {
                char* str = AS_STRING(value);
                string = realloc(string, strlen(string) + strlen(str) + 1);
                strcat(string, str);
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
            if (isnan(value.as.floatValue)) {
                string = realloc(string, 4);
                sprintf(string, "NaN");
                break;
            } else if (isinf(value.as.floatValue)) {
                if (value.as.floatValue < 0) {
                    string = realloc(string, 10);
                    sprintf(string, "-infinity");
                } else {
                    string = realloc(string, 9);
                    sprintf(string, "infinity");
                }
                break;
            }

            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.15f", value.as.floatValue);
            
            char *dot = strchr(buffer, '.');
            if (dot != NULL) {
                char *end = dot + strlen(dot) - 1;
                while (end > dot && *end == '0') {
                    *end-- = '\0';
                }
                if (*end == '.') {
                    *end = '\0';
                }
            }

            string = realloc(string, strlen(buffer) + 1);
            strcpy(string, buffer);
            break;
        }
        case TYPE_DOUBLE: {
            if (isnan(value.as.doubleValue)) {
                string = realloc(string, 4);
                sprintf(string, "NaN");
                break;
            } else if (isinf(value.as.doubleValue)) {
                if (value.as.doubleValue < 0) {
                    string = realloc(string, 10);
                    sprintf(string, "-infinity");
                } else {
                    string = realloc(string, 9); 
                    sprintf(string, "infinity");
                }
                break;
            }

            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.15f", value.as.doubleValue);
            
            char *dot = strchr(buffer, '.');
            if (dot != NULL) {
                char *end = dot + strlen(dot) - 1;
                while (end > dot && *end == '0') {
                    *end-- = '\0';
                }
                if (*end == '.') {
                    *end = '\0';
                }
            }

            string = realloc(string, strlen(buffer) + 1);
            strcpy(string, buffer);
            
            break;
        }
        case TYPE_BOOL: {
            string = realloc(string, 6);
            sprintf(string, "%s", value.as.boolValue ? "true" : "false");
            break;
        }

        case D_NULL: {
            string = realloc(string, 6);
            sprintf(string, "null");
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
        case D_NULL: {
            return "null";
        }
        default: {
            return "<unknown type>";
        }
    }
}

void markDefined(Value* value) {
    value->defined = true;
    if (value->type == TYPE_ARRAY) {
        ValueArray* array = AS_ARRAY(*value);
        for (size_t i = 0; i < array->count; i++) {
            if (array->values[i].defined) continue;
            markDefined(&array->values[i]);
        }
    }

    if (value->type == TYPE_OBJECT) {
        ValueObject* object = AS_OBJECT(*value);
        for (size_t i = 0; i < object->count; i++) {
            if (object->values[i].defined) continue;
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

void write_ValueObject(ValueObject* object, Value key, Value value) {
    if (object->capacity < object->count + 1) {
        size_t oldCapacity = object->capacity;
        object->capacity = DOSATO_UPDATE_CAPACITY(oldCapacity);
        object->values = DOSATO_RESIZE_LIST(Value, object->values, oldCapacity, object->capacity);
        object->keys = DOSATO_RESIZE_LIST(Value, object->keys, oldCapacity, object->capacity);
    }
    object->values[object->count] = value;
    object->keys[object->count] = key;
    object->count++;
}

void free_ValueObject(ValueObject* object) {
    free(object->values);
    free(object->keys);
    init_ValueObject(object);
}

bool hasKey(ValueObject* object, Value key) {
    for (size_t i = 0; i < object->count; i++) {
        if (valueEqualsStrict(&object->keys[i], &key)) {
            return true;
        }
    }
    return false;
}

Value* getValueAtKey(ValueObject* object, Value key) {
    for (size_t i = 0; i < object->count; i++) {
        if (valueEqualsStrict(&object->keys[i], &key)) {
            return &object->values[i];
        }
    }
    return NULL;
}

void removeFromKey (ValueObject* object, Value key) {
    for (size_t i = 0; i < object->count; i++) {
        if (valueEqualsStrict(&object->keys[i], &key)) {
            for (size_t j = i; j < object->count - 1; j++) {
                object->keys[j] = object->keys[j + 1];
                object->values[j] = object->values[j + 1];
            }
            object->count--;
            return;
        }
    }
}