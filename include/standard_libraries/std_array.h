#ifndef STD_ARRAY_H
#define STD_ARRAY_H

#include "common_std.h"

Value dosato_quick_sort (ValueArray* array, int left, int right, size_t function);
Value dosato_partition (ValueArray* array, int left, int right, size_t function);

Value array_sort(ValueArray args, bool debug);
Value array_push(ValueArray args, bool debug);
Value array_pop(ValueArray args, bool debug);
Value array_shift(ValueArray args, bool debug);
Value array_unshift(ValueArray args, bool debug);
Value array_slice(ValueArray args, bool debug);
Value array_splice(ValueArray args, bool debug);
Value array_index_of(ValueArray args, bool debug);

Value array_range(ValueArray args, bool debug);
Value array_rangef(ValueArray args, bool debug);

#endif // STD_ARRAY_H