#ifndef test_library_h
#define test_library_h

#include "dosato.h"


Value sayHello(ValueArray args, bool debug);
Value multiply(ValueArray args, bool debug);
Value count (ValueArray args, bool debug);
Value custom_error(ValueArray args, bool debug);
Value return_string(ValueArray args, bool debug);
Value return_array(ValueArray args, bool debug);
Value return_object(ValueArray args, bool debug);

DosatoFunctionMapList functions;
void init(void* vm);

#endif // test_library_h