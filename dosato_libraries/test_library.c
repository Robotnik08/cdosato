#include "test_library.h" // test_library.h



DosatoFunctionMapList functions;
void* main_vm;

void init(void* vm) {
    main_vm = vm;

    DosatoFunctionMap map;
    map.name = "sayHello";
    map.function = sayHello;
    write_DosatoFunctionMapList(&functions, map);
    DosatoFunctionMap map2;
    map2.name = "multiply";
    map2.function = multiply;
    write_DosatoFunctionMapList(&functions, map2);
    DosatoFunctionMap map3;
    map3.name = "count";
    map3.function = count;
    write_DosatoFunctionMapList(&functions, map3);
    DosatoFunctionMap map4;
    map4.name = "donotcall";
    map4.function = custom_error;
    write_DosatoFunctionMapList(&functions, map4);
    DosatoFunctionMap map5;
    map5.name = "return_string";
    map5.function = return_string;
    write_DosatoFunctionMapList(&functions, map5);
    DosatoFunctionMap map6;
    map6.name = "return_array";
    map6.function = return_array;
    write_DosatoFunctionMapList(&functions, map6);
    DosatoFunctionMap map7;
    map7.name = "return_object";
    map7.function = return_object;
    write_DosatoFunctionMapList(&functions, map7);
}


Value sayHello(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }
    int cast_result = castValue(&args.values[0], TYPE_STRING);
    if (cast_result != E_NULL) {
        return BUILD_EXCEPTION(cast_result);
    }

    printf("Hello, %s!\n", AS_STRING(args.values[0]));
    
    return UNDEFINED_VALUE;
}

Value multiply(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }
    int cast_result = castValue(&args.values[0], TYPE_DOUBLE);
    if (cast_result != E_NULL) {
        return BUILD_EXCEPTION(cast_result);
    }
    cast_result = castValue(&args.values[1], TYPE_DOUBLE);
    if (cast_result != E_NULL) {
        return BUILD_EXCEPTION(cast_result);
    }

    double result = args.values[0].as.doubleValue * args.values[1].as.doubleValue;
    return BUILD_DOUBLE(result);
}

int global = 0;

Value count (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }
    return BUILD_LONG(global++);
}

Value custom_error(ValueArray args, bool debug) {
    PRINT_ERROR("You are not allowed to call this function silly!\n");
    return BUILD_EXCEPTION(E_EMPTY_MESSAGE);
}

Value return_string(ValueArray args, bool debug) {
    return RETURN_STRING(COPY_STRING("Hello from test library!"));
}

Value return_array(ValueArray args, bool debug) {
    return RETURN_ARRAY(buildArray(10, 
        BUILD_LONG(1), 
        BUILD_LONG(2), 
        BUILD_LONG(3), 
        BUILD_LONG(4), 
        BUILD_LONG(5), 
        BUILD_LONG(6), 
        BUILD_LONG(7), 
        BUILD_LONG(8), 
        BUILD_LONG(9), 
        BUILD_LONG(10)
    ));
}

Value return_object(ValueArray args, bool debug) {
    return RETURN_OBJECT(buildObject(3, 
        "name", BUILD_STRING(COPY_STRING("John")), 
        "age", BUILD_LONG(30), 
        "isStudent", BUILD_BOOL(false)
    ));
}