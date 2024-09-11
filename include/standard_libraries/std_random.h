#ifndef STD_RANDOM_H
#define STD_RANDOM_H


#include "common_std.h"

Value io_seed_random (ValueArray args, bool debug);
Value io_random_double (ValueArray args, bool debug);
Value io_random_int (ValueArray args, bool debug);
Value io_random_range (ValueArray args, bool debug);
Value io_random_bool (ValueArray args, bool debug);

#endif // STD_SYSTEM_H