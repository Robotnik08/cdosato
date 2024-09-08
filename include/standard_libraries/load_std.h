#ifndef DOSATO_STD_LOADER_H
#define DOSATO_STD_LOADER_H

#include "common.h"
#include "value.h"
#include "virtual-machine.h"
#include "common_std.h"
#include "dynamic_library_loader.h"


#include "std_io.h"
#include "std_system.h"
#include "std_random.h"
#include "std_time.h"
#include "std_math.h"
#include "std_string.h"

int loadStandardLibrary(VirtualMachine* vm);

#endif // DOSATO_STD_LOADER_H	