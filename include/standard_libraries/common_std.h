#ifndef DOSATO_STD_LIBRARY_COMMON_STD_H
#define DOSATO_STD_LIBRARY_COMMON_STD_H

#include "../common.h"
#include "../value.h"
#include "../memory.h"
#include "../error.h"
#include "../filetools.h"
#include "../strtools.h"

#define GET_ARG(args, index) (args.values[args.count - index - 1])

#endif // DOSATO_STD_LIBRARY_COMMON_STD_H