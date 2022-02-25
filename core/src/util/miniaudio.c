#include "core.h"

#define MA_MALLOC core_alloc
#define MA_REALLOC core_realloc
#define MA_FREE core_free

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
