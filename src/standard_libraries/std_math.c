#include "../../include/standard_libraries/std_math.h"

Value math_abs(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (ISFLOATTYPE(arg.type)) {
        int cast_result = castValue(&arg, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_DOUBLE(fabs(arg.as.doubleValue));
    } else {\
        int cast_result = castValue(&arg, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_LONG(llabs(arg.as.longValue));
    }
}

Value math_round(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(round(arg.as.doubleValue));
}

Value math_floor(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(floor(arg.as.doubleValue));
}

Value math_ceil(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(ceil(arg.as.doubleValue));
}

Value math_pow(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        int cast_result = castValue(&a, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        double result = pow(a.as.doubleValue, b.as.doubleValue);
        if (isnan(result)) {
            return BUILD_EXCEPTION(E_MATH_DOMAIN_ERROR);
        }
        return BUILD_DOUBLE(result);
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_LONG(powl(a.as.longValue, b.as.longValue));
    }
};

Value math_sqrt(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    if (arg.as.doubleValue < 0) {
        return BUILD_EXCEPTION(E_MATH_DOMAIN_ERROR);
    }
    return BUILD_DOUBLE(sqrt(arg.as.doubleValue));
}

Value math_min(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        int cast_result = castValue(&a, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_DOUBLE(fmin(a.as.doubleValue, b.as.doubleValue));
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_LONG(a.as.longValue < b.as.longValue ? a.as.longValue : b.as.longValue);
    }
}

Value math_max(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        int cast_result = castValue(&a, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_DOUBLE(a.as.doubleValue > b.as.doubleValue ? a.as.doubleValue : b.as.doubleValue);
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_LONG(a.as.longValue > b.as.longValue ? a.as.longValue : b.as.longValue);
    }
}

Value math_clamp(ValueArray args, bool debug) {
    if (args.count != 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);
    Value c = GET_ARG(args, 2);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type) || ISFLOATTYPE(c.type)) {
        int cast_result = castValue(&a, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&c, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_DOUBLE(fmax(b.as.doubleValue, fmin(a.as.doubleValue, c.as.doubleValue)));
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&c, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        
        return BUILD_LONG(__max(b.as.longValue, __min(a.as.longValue, c.as.longValue)));
    }
}

Value math_log(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        int cast_result = castValue(&a, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_DOUBLE(log(b.as.doubleValue) / log(a.as.doubleValue));
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_LONG(log(b.as.longValue) / log(a.as.longValue));
    }
}

Value math_log10(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(log10(arg.as.doubleValue));
}

Value math_log2(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(log2(arg.as.doubleValue));
}

Value math_sin(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(sin(arg.as.doubleValue));
}

Value math_cos(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(cos(arg.as.doubleValue));
}

Value math_tan(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(tan(arg.as.doubleValue));
}

Value math_asin(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(asin(arg.as.doubleValue));
}

Value math_acos(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(acos(arg.as.doubleValue));
}

Value math_atan(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(atan(arg.as.doubleValue));
}

Value math_atan2(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        int cast_result = castValue(&a, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_DOUBLE(atan2(a.as.doubleValue, b.as.doubleValue));
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return BUILD_DOUBLE(atan2(a.as.longValue, b.as.longValue));
    }
}

Value math_exp(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return BUILD_DOUBLE(exp(arg.as.doubleValue));
}