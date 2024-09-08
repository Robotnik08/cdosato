#ifndef STD_IO_H
#define STD_IO_H


#include "common_std.h"

Value io_say (ValueArray args, bool debug);
Value io_sayln (ValueArray args, bool debug);
Value io_listen (ValueArray args, bool debug);

// file io functions
Value io_read_file (ValueArray args, bool debug);
Value io_write_file (ValueArray args, bool debug);
Value io_append_file (ValueArray args, bool debug);
Value io_delete_file (ValueArray args, bool debug);
Value io_file_exists (ValueArray args, bool debug);
Value io_move_file (ValueArray args, bool debug);
Value io_copy_file (ValueArray args, bool debug);

#endif // STD_IO_H