#ifndef STD_TIME_H
#define STD_TIME_H

#include "common_std.h"

Value io_time (ValueArray args, bool debug);
Value io_datetime (ValueArray args, bool debug);
Value io_clock (ValueArray args, bool debug);
Value io_sleep (ValueArray args, bool debug);

#endif // STD_TIME_H