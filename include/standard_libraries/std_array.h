#ifndef STD_ARRAY_H
#define STD_ARRAY_H

#include "common_std.h"

Value dosato_quick_sort (ValueArray* array, int left, int right, Value function);
Value dosato_partition (ValueArray* array, int left, int right, Value function);

Value array_sort(ValueArray args, bool debug);
Value array_push(ValueArray args, bool debug);
Value array_pop(ValueArray args, bool debug);
Value array_shift(ValueArray args, bool debug);
Value array_unshift(ValueArray args, bool debug);
Value array_slice(ValueArray args, bool debug);
Value array_splice(ValueArray args, bool debug);
Value array_index_of(ValueArray args, bool debug);
Value array_last_index_of(ValueArray args, bool debug);
Value array_reverse(ValueArray args, bool debug);

Value array_fill(ValueArray args, bool debug);
Value array_range(ValueArray args, bool debug);
Value array_rangef(ValueArray args, bool debug);

Value array_counter(ValueArray args, bool debug);
Value array_set_counter(ValueArray args, bool debug);

Value array_map(ValueArray args, bool debug);
Value array_reduce(ValueArray args, bool debug);
Value array_some(ValueArray args, bool debug);
Value array_filter(ValueArray args, bool debug);
Value array_every(ValueArray args, bool debug);
Value array_count(ValueArray args, bool debug);
Value array_sum(ValueArray args, bool debug);
Value array_find(ValueArray args, bool debug);
Value array_combinations(ValueArray args, bool debug);
void combinationUtil(Value* arr, int n, int r, int index, int* data, int i, ValueArray* new_array);
Value array_permutations(ValueArray args, bool debug);
void permutationUtil(Value* arr, int n, int r, int index, int* data, bool* used, ValueArray* new_array);
Value array_remove_duplicates(ValueArray args, bool debug);
Value array_length(ValueArray args, bool debug);
Value array_difference (ValueArray args, bool debug);
Value array_intersection (ValueArray args, bool debug);
Value array_contains (ValueArray args, bool debug);

#endif // STD_ARRAY_H