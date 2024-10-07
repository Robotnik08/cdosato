#ifndef dosato_common_h
#define dosato_common_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#include "strtools.h"
#include "error.h"
#include "filetools.h"

// unistd.h is not available on windows
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#define DOSATO_VERSION "0.4"
#define DOSATO_DATE "07/10/2024"

#endif // dosato_common_h