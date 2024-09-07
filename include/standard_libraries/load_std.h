#ifndef DOSATO_STD_LOADER_H
#define DOSATO_STD_LOADER_H

#include "common.h"
#include "value.h"
#include "virtual-machine.h"
#include "common_std.h"
#include "dynamic_library_loader.h"


#include "io.h"

int loadStandardLibrary(VirtualMachine* vm);

#endif // DOSATO_STD_LOADER_H	