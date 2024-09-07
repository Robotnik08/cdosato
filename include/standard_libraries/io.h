#ifndef STD_IO_H
#define STD_IO_H


#include "common_std.h"

Value io_say (ValueArray args, bool debug);
Value io_sayln (ValueArray args, bool debug);
Value io_listen (ValueArray args, bool debug);

#endif // STD_IO_H