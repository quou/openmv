#include "core.h"

#define MA_MALLOC core_alloc
#define MA_REALLOC core_realloc
#define MA_FREE core_free

#define MINIAUDIO_IMPLEMENTATION
#define SML_IMPL
#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "maths.h"
#include "miniaudio.h"
#include "stb_rect_pack.h"
#include "stb_truetype.h"
