#include "../../include/standard_libraries/std_math.h"

Value math_abs(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    if (ISFLOATTYPE(arg.type)) {
        int cast_result = castValue(&arg, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_DOUBLE, .as.doubleValue = fabs(arg.as.doubleValue) };
    } else {\
        int cast_result = castValue(&arg, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_LONG, .as.longValue = llabs(arg.as.longValue) };
    }
}

Value math_round(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = round(arg.as.doubleValue) };
}

Value math_floor(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = floor(arg.as.doubleValue) };
}

Value math_ceil(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = ceil(arg.as.doubleValue) };
}

Value math_pow(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG_COPY(args, 0);
    Value b = GET_ARG_COPY(args, 1);

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
        return (Value){ TYPE_DOUBLE, .as.doubleValue = result };
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_LONG, .as.longValue = powl(a.as.longValue, b.as.longValue) };
    }
};

Value math_sqrt(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    if (arg.as.doubleValue < 0) {
        return BUILD_EXCEPTION(E_MATH_DOMAIN_ERROR);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = sqrt(arg.as.doubleValue) };
}

Value math_min(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG_COPY(args, 0);
    Value b = GET_ARG_COPY(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        int cast_result = castValue(&a, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_DOUBLE, .as.doubleValue = fmin(a.as.doubleValue, b.as.doubleValue) };
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_LONG, .as.longValue = a.as.longValue < b.as.longValue ? a.as.longValue : b.as.longValue };
    }
}

Value math_max(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG_COPY(args, 0);
    Value b = GET_ARG_COPY(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        int cast_result = castValue(&a, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_DOUBLE, .as.doubleValue = fmax(a.as.doubleValue, b.as.doubleValue) };
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_LONG, .as.longValue = a.as.longValue > b.as.longValue ? a.as.longValue : b.as.longValue };
    }
}

Value math_clamp(ValueArray args, bool debug) {
    if (args.count != 3) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG_COPY(args, 0);
    Value b = GET_ARG_COPY(args, 1);
    Value c = GET_ARG_COPY(args, 2);

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
        return (Value){ TYPE_DOUBLE, .as.doubleValue = fmax(b.as.doubleValue, fmin(a.as.doubleValue, c.as.doubleValue)) };
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
        
        return (Value){ TYPE_LONG, .as.longValue = __max(b.as.longValue, __min(a.as.longValue, c.as.longValue)) };
    }
}

Value math_log(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG_COPY(args, 0);
    Value b = GET_ARG_COPY(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        int cast_result = castValue(&a, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_DOUBLE, .as.doubleValue = log(b.as.doubleValue) / log(a.as.doubleValue) };
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_LONG, .as.longValue = logl(b.as.longValue) / logl(a.as.longValue) };
    }
}

Value math_log10(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = log10(arg.as.doubleValue) };
}

Value math_log2(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = log2(arg.as.doubleValue) };
}

Value math_sin(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = sin(arg.as.doubleValue) };
}

Value math_cos(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = cos(arg.as.doubleValue) };
}

Value math_tan(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = tan(arg.as.doubleValue) };
}

Value math_asin(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = asin(arg.as.doubleValue) };
}

Value math_acos(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = acos(arg.as.doubleValue) };
}

Value math_atan(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = atan(arg.as.doubleValue) };
}

Value math_atan2(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG_COPY(args, 0);
    Value b = GET_ARG_COPY(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        int cast_result = castValue(&a, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_DOUBLE);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_DOUBLE, .as.doubleValue = atan2(a.as.doubleValue, b.as.doubleValue) };
    } else {
        int cast_result = castValue(&a, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        cast_result = castValue(&b, TYPE_LONG);
        if (cast_result != 0) {
            return BUILD_EXCEPTION(cast_result);
        }
        return (Value){ TYPE_DOUBLE, .as.doubleValue = atan2(a.as.longValue, b.as.longValue) };
    }
}

Value math_exp(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG_COPY(args, 0);
    int cast_result = castValue(&arg, TYPE_DOUBLE);
    if (cast_result != 0) {
        return BUILD_EXCEPTION(cast_result);
    }
    return (Value){ TYPE_DOUBLE, .as.doubleValue = exp(arg.as.doubleValue) };
}