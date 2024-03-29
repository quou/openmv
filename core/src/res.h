#pragma once

#include "audio.h"
#include "common.h"
#include "video.h"

/* Read the raw data from a file.
 *
 * If `term' is true, a null terminator is added to the end
 * of the file. */
API bool read_raw(const char* path, u8** buf, u64* size, bool term);
API bool read_raw_no_pck(const char* path, u8** buf, u64* size, bool term);

API void res_init();
API void res_deinit();

API void res_unload(const char* path);

API struct shader load_shader(const char* path);
API struct texture* load_texture(const char* path, u32 flags);
API struct font* load_font(const char* path, f32 size);
API struct audio_clip* load_audio_clip(const char* path);

API struct shader load_shader_no_pck(const char* path);
API struct texture* load_texture_no_pck(const char* path, u32 flags);
API struct font* load_font_no_pck(const char* path, f32 size);
API struct audio_clip* load_audio_clip_no_pck(const char* path);

/* File API, for reading only.
 *
 * In debug, it wraps the default C stdio.
 * In release, it contains extra functionality to read from the
 * packed resource file. */
struct file {
	void* handle;
	u64 pk_offset;
	u64 cursor;
	u64 size;
};

API struct file file_open(const char* path);
API bool file_good(struct file* file);
API void file_close(struct file* file);
API u64 file_seek(struct file* file, u64 offset);
API u64 file_read(void* buf, u64 size, u64 count, struct file* file);
