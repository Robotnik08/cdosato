#ifndef STD_SYSTEM_H
#define STD_SYSTEM_H


#include "common_std.h"

Value io_clear (ValueArray args, bool debug);
Value io_end (ValueArray args, bool debug);
Value io_pause (ValueArray args, bool debug);
Value io_system (ValueArray args, bool debug);

#endif // STD_SYSTEM_H