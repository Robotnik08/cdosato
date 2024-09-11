#include "test_library.h" // test_library.h

DosatoFunctionMapList functions;

void init() {
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
}


Value sayHello(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }
    int cast_result = castValue(&args.values[0], TYPE_STRING);
    if (cast_result != E_NULL) {
        return BUILD_EXCEPTION(cast_result);
    }

    printf("Hello, %s!\n", args.values[0].as.stringValue);
    
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
    return (Value){ TYPE_DOUBLE, .as.doubleValue = result};
}

int global = 0;

Value count (ValueArray args, bool debug) {
    if (args.count != 0) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }
    return (Value){ TYPE_LONG, .as.longValue = global++};
}