#ifndef STD_ARRAY_H
#define STD_ARRAY_H

#include "common_std.h"

Value dosato_quick_sort (ValueArray* array, int left, int right, size_t function);
Value dosato_partition (ValueArray* array, int left, int right, size_t function);

Value array_sort(ValueArray args, bool debug);

#endif // STD_ARRAY_H