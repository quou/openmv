#include "core.h"

#define MA_MALLOC core_alloc
#define MA_REALLOC core_realloc
#define MA_FREE core_free

#define STBTT_malloc(x,u)  ((void)(u),core_alloc(x))
#define STBTT_free(x,u)    ((void)(u),core_free(x))

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define SML_IMPL
#include "maths.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"