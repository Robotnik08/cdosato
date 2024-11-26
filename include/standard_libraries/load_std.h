#ifndef DOSATO_STD_LOADER_H
#define DOSATO_STD_LOADER_H

#include "common.h"
#include "value.h"
#include "virtual-machine.h"
#include "common_std.h"
#include "dynamic_library_loader.h"

// temporaryly disabled 
#include "std_io.h"
#include "std_system.h"
#include "std_random.h"
#include "std_time.h"
#include "std_math.h"
#include "std_string.h"
#include "std_array.h"
#include "std_type.h"
#include "std_object.h"

int loadStandardLibrary(VirtualMachine* vm);
int loadConstants (VirtualMachine* vm, char** argv, int argc);

#endif // DOSATO_STD_LOADER_H	