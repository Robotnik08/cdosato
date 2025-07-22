#ifndef STD_OBJECT_H
#define STD_OBJECT_H

#include "common_std.h"
#include "../hash.h"

Value object_keys(ValueArray args, bool debug);
Value object_values(ValueArray args, bool debug);
Value object_entries(ValueArray args, bool debug);

Value object_getHash(ValueArray args, bool debug);


#endif // STD_OBJECT_H