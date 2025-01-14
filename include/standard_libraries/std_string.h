#ifndef STD_STRING_H
#define STD_STRING_H


#include "common_std.h"

Value string_split(ValueArray args, bool debug);
Value string_lower(ValueArray args, bool debug);
Value string_upper(ValueArray args, bool debug);
Value string_substr(ValueArray args, bool debug);
Value string_indexof(ValueArray args, bool debug);
Value string_lastindexof(ValueArray args, bool debug);
Value string_startswith(ValueArray args, bool debug);
Value string_endswith(ValueArray args, bool debug);
Value string_replace(ValueArray args, bool debug);
Value string_trim(ValueArray args, bool debug);
Value string_reverse(ValueArray args, bool debug);
Value string_contains(ValueArray args, bool debug);
Value string_remove(ValueArray args, bool debug);
Value string_insert(ValueArray args, bool debug);
Value string_atoi(ValueArray args, bool debug);
Value string_atod(ValueArray args, bool debug);
Value string_count(ValueArray args, bool debug);
Value string_join(ValueArray args, bool debug);
Value string_format(ValueArray args, bool debug);
Value string_to_string(ValueArray args, bool debug);
Value string_compare(ValueArray args, bool debug);


#endif // STD_STRING_H