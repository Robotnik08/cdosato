#include "../../include/standard_libraries/std_math.h"

Value math_abs(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (ISFLOATTYPE(arg.type)) {
        CAST_SAFE(arg, TYPE_DOUBLE);
        return BUILD_DOUBLE(fabs(AS_DOUBLE(arg)));
    } else {\
        CAST_SAFE(arg, TYPE_LONG);
        return BUILD_LONG(llabs(AS_LONG(arg)));
    }
}

Value math_round(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(round(AS_DOUBLE(arg)));
}

Value math_floor(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(floor(AS_DOUBLE(arg)));
}

Value math_ceil(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(ceil(AS_DOUBLE(arg)));
}

Value math_pow(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        CAST_SAFE(a, TYPE_DOUBLE);
        CAST_SAFE(b, TYPE_DOUBLE);
        double result = pow(AS_DOUBLE(a), AS_DOUBLE(b));
        if (isnan(result)) {
            return BUILD_EXCEPTION(E_MATH_DOMAIN_ERROR);
        }
        return BUILD_DOUBLE(result);
    } else {
        CAST_SAFE(a, TYPE_LONG);
        CAST_SAFE(b, TYPE_LONG);
        return BUILD_LONG(powl(AS_LONG(a), AS_LONG(b)));
    }
};

Value math_sqrt(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    if (AS_DOUBLE(arg) < 0) {
        return BUILD_EXCEPTION(E_MATH_DOMAIN_ERROR);
    }
    return BUILD_DOUBLE(sqrt(AS_DOUBLE(arg)));
}

Value math_min(ValueArray args, bool debug) {
    if (args.count == 2) {
        Value a = GET_ARG(args, 0);
        Value b = GET_ARG(args, 1);

        if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
            CAST_SAFE(a, TYPE_DOUBLE);
            CAST_SAFE(b, TYPE_DOUBLE);
            return BUILD_DOUBLE(fmin(AS_DOUBLE(a), AS_DOUBLE(b)));
        } else {
            CAST_SAFE(a, TYPE_LONG);
            CAST_SAFE(b, TYPE_LONG);
            return BUILD_LONG(AS_LONG(a) < AS_LONG(b) ? AS_LONG(a) : AS_LONG(b));
        }
    } else if (args.count == 1) {
        Value arg = GET_ARG(args, 0);
        if (arg.type != TYPE_ARRAY) {
            return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
        }

        ValueArray* arr = AS_ARRAY(arg);
        if (arr->count == 0) {
            return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
        }

        Value min = arr->values[0];
        for (size_t i = 1; i < arr->count; i++) {
            Value current = arr->values[i];
            if (ISFLOATTYPE(current.type) || ISFLOATTYPE(min.type)) {
                CAST_SAFE(current, TYPE_DOUBLE);
                CAST_SAFE(min, TYPE_DOUBLE);
                if (AS_DOUBLE(current) < AS_DOUBLE(min)) {
                    min = current;
                }
            } else {
                CAST_SAFE(current, TYPE_LONG);
                CAST_SAFE(min, TYPE_LONG);
                if (AS_LONG(current) < AS_LONG(min)) {
                    min = current;
                }
            }
        }

        return min;
    } else {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }
}

Value math_max(ValueArray args, bool debug) {
    if (args.count == 2) {
        Value a = GET_ARG(args, 0);
        Value b = GET_ARG(args, 1);

        if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
            CAST_SAFE(a, TYPE_DOUBLE);
            CAST_SAFE(b, TYPE_DOUBLE);
            return BUILD_DOUBLE(fmax(AS_DOUBLE(a), AS_DOUBLE(b)));
        } else {
            CAST_SAFE(a, TYPE_LONG);
            CAST_SAFE(b, TYPE_LONG);
            return BUILD_LONG(AS_LONG(a) > AS_LONG(b) ? AS_LONG(a) : AS_LONG(b));
        }
    } else if (args.count == 1) {
        Value arg = GET_ARG(args, 0);
        if (arg.type != TYPE_ARRAY) {
            return BUILD_EXCEPTION(E_NOT_AN_ARRAY);
        }

        ValueArray* arr = AS_ARRAY(arg);
        if (arr->count == 0) {
            return BUILD_EXCEPTION(E_INDEX_OUT_OF_BOUNDS);
        }

        Value max = arr->values[0];
        for (size_t i = 1; i < arr->count; i++) {
            Value current = arr->values[i];
            if (ISFLOATTYPE(current.type) || ISFLOATTYPE(max.type)) {
                CAST_SAFE(current, TYPE_DOUBLE);
                CAST_SAFE(max, TYPE_DOUBLE);
                if (AS_DOUBLE(current) > AS_DOUBLE(max)) {
                    max = current;
                }
            } else {
                CAST_SAFE(current, TYPE_LONG);
                CAST_SAFE(max, TYPE_LONG);
                if (AS_LONG(current) > AS_LONG(max)) {
                    max = current;
                }
            }
        }

        return max;
    } else {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
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
        CAST_SAFE(a, TYPE_DOUBLE);
        CAST_SAFE(b, TYPE_DOUBLE);
        CAST_SAFE(c, TYPE_DOUBLE);
        return BUILD_DOUBLE(fmax(AS_DOUBLE(b), fmin(AS_DOUBLE(a), AS_DOUBLE(c))));
    } else {
        CAST_SAFE(a, TYPE_LONG);
        CAST_SAFE(b, TYPE_LONG);
        CAST_SAFE(c, TYPE_LONG);
        
        return BUILD_LONG(__max(AS_LONG(b), __min(AS_LONG(a), AS_LONG(c))));
    }
}

Value math_log(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        CAST_SAFE(a, TYPE_DOUBLE);
        CAST_SAFE(b, TYPE_DOUBLE);
        return BUILD_DOUBLE(log(AS_DOUBLE(b)) / log(AS_DOUBLE(a)));
    } else {
        CAST_SAFE(a, TYPE_LONG);
        CAST_SAFE(b, TYPE_LONG);
        return BUILD_LONG(log(AS_LONG(b)) / log(AS_LONG(a)));
    }
}

Value math_log10(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(log10(AS_DOUBLE(arg)));
}

Value math_log2(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(log2(AS_DOUBLE(arg)));
}

Value math_sin(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(sin(AS_DOUBLE(arg)));
}

Value math_cos(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(cos(AS_DOUBLE(arg)));
}

Value math_tan(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(tan(AS_DOUBLE(arg)));
}

Value math_asin(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(asin(AS_DOUBLE(arg)));
}

Value math_acos(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(acos(AS_DOUBLE(arg)));
}

Value math_atan(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(atan(AS_DOUBLE(arg)));
}

Value math_atan2(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    if (ISFLOATTYPE(a.type) || ISFLOATTYPE(b.type)) {
        CAST_SAFE(a, TYPE_DOUBLE);
        CAST_SAFE(b, TYPE_DOUBLE);
        return BUILD_DOUBLE(atan2(AS_DOUBLE(a), AS_DOUBLE(b)));
    } else {
        CAST_SAFE(a, TYPE_LONG);
        CAST_SAFE(b, TYPE_LONG);
        return BUILD_DOUBLE(atan2(AS_LONG(a), AS_LONG(b)));
    }
}

Value math_exp(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    CAST_SAFE(arg, TYPE_DOUBLE);
    return BUILD_DOUBLE(exp(AS_DOUBLE(arg)));
}

Value math_digits(ValueArray args, bool debug) {
    if (args.count != 1) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value arg = GET_ARG(args, 0);
    if (ISFLOATTYPE(arg.type)) {
        CAST_SAFE(arg, TYPE_DOUBLE);
        if (AS_DOUBLE(arg) == 0) {
            return BUILD_LONG(1);
        }
        if (AS_DOUBLE(arg) < 0) {
            return BUILD_LONG((long long int)log10(-AS_DOUBLE(arg)) + 2);
        }
        return BUILD_LONG((long long int)log10(AS_DOUBLE(arg)) + 1);
    } else {
        CAST_SAFE(arg, TYPE_LONG);
        if (AS_LONG(arg) == 0) {
            return BUILD_LONG(1);
        }
        // Accurate for positive numbers (even really large ones)
        if (AS_LONG(arg) < 0) {
            return BUILD_LONG((long long int)log10(-AS_LONG(arg)) + 2);
        }
        return BUILD_LONG((long long int)log10(AS_LONG(arg)) + 1);
    }
}

long long int gcd(long long int a, long long int b) {
    if (a && b) for(;(a %= b) && (b %= a););
    return a | b;
}

long long int lcm(long long int a, long long int b) {
    return a / gcd(a, b) * b;
}

Value math_gdc(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    CAST_SAFE(a, TYPE_LONG);
    CAST_SAFE(b, TYPE_LONG);
    return BUILD_LONG(gcd(AS_LONG(a), AS_LONG(b)));
}

Value math_lcm(ValueArray args, bool debug) {
    if (args.count != 2) {
        return BUILD_EXCEPTION(E_WRONG_NUMBER_OF_ARGUMENTS);
    }

    Value a = GET_ARG(args, 0);
    Value b = GET_ARG(args, 1);

    CAST_SAFE(a, TYPE_LONG);
    CAST_SAFE(b, TYPE_LONG);
    return BUILD_LONG(lcm(AS_LONG(a), AS_LONG(b)));
}