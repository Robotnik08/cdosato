#ifndef STD_MATH_H
#define STD_MATH_H


#include "common_std.h"

Value math_abs(ValueArray args, bool debug);
Value math_round(ValueArray args, bool debug);
Value math_floor(ValueArray args, bool debug);
Value math_ceil(ValueArray args, bool debug);
Value math_pow(ValueArray args, bool debug);
Value math_sqrt(ValueArray args, bool debug);
Value math_min(ValueArray args, bool debug);
Value math_max(ValueArray args, bool debug);
Value math_clamp(ValueArray args, bool debug);
Value math_log(ValueArray args, bool debug);
Value math_log10(ValueArray args, bool debug);
Value math_log2(ValueArray args, bool debug);
Value math_sin(ValueArray args, bool debug);
Value math_cos(ValueArray args, bool debug);
Value math_tan(ValueArray args, bool debug);
Value math_asin(ValueArray args, bool debug);
Value math_acos(ValueArray args, bool debug);
Value math_atan(ValueArray args, bool debug);
Value math_atan2(ValueArray args, bool debug);
Value math_exp(ValueArray args, bool debug);
Value math_digits(ValueArray args, bool debug);
Value math_gdc(ValueArray args, bool debug);
Value math_lcm(ValueArray args, bool debug);

#endif // STD_MATH_H    