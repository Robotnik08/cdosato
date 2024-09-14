#ifndef test_library_h
#define test_library_h

#include "dosato.h"


Value sayHello(ValueArray args, bool debug);
Value multiply(ValueArray args, bool debug);
Value count (ValueArray args, bool debug);
Value custom_error(ValueArray args, bool debug);

DosatoFunctionMapList functions;
void init();

#endif // test_library_h