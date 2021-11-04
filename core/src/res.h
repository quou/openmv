#pragma once

#include "common.h"
#include "video.h"

/* Read the raw data from a file.
 *
 * If `term' is true, a null terminator is added to the end
 * of the file. */
API bool read_raw(const char* path, u8** buf, u32* size, bool term);

API void res_init();
API void res_deinit();

API void res_unload(const char* path);

API struct shader* load_shader(const char* path);
API struct texture* load_texture(const char* path);
API struct font* load_font(const char* path, float size);
