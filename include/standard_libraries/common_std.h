#ifndef DOSATO_STD_LIBRARY_COMMON_STD_H
#define DOSATO_STD_LIBRARY_COMMON_STD_H

#include "../common.h"
#include "../value.h"
#include "../memory.h"
#include "../error.h"
#include "../filetools.h"
#include "../strtools.h"
#include "../virtual-machine.h"

#define GET_ARG(args, index) (args.values[args.count - index - 1])

#define CAST_SAFE(value, type) \
    do { \
        ErrorType cast_result_##value = castValue(&value, type); \
        if (cast_result_##value != 0) { \
            return BUILD_EXCEPTION(cast_result_##value); \
        } \
    } while (0)

#endif // DOSATO_STD_LIBRARY_COMMON_STD_H