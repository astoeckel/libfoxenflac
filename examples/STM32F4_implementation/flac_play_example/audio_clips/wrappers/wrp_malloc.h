#ifndef __WRP_MALLOC__
#define __WRP_MALLOC__

#include "rtos.h"

#define malloc(size) pvPortMalloc(size)
#define free(ptr) vPortFree(ptr)

#endif // __WRP_MALLOC__
