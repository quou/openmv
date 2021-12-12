#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "res.h"
#include "table.h"

static const char* package_path = "res.pck";

#if DEBUG
bool read_raw(const char* path, u8** buf, u64* size, bool term) {
	*buf = null;
	size ? *size = 0 : 0;

	FILE* file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "Failed to fopen file %s\n", path);
		return false;
	}

	fseek(file, 0, SEEK_END);
	const u64 file_size = ftell(file);
	rewind(file);

	*buf = core_alloc(file_size + (term ? 1 : 0));
	const u64 bytes_read = fread(*buf, sizeof(char), file_size, file);
	if (bytes_read < file_size) {
		printf("Failed to read file: %s\n", path);
	}

	if (term) {
		*((*buf) + file_size) = '\0';
	}

	if (size) {
		*size = file_size + (term ? 1 : 0);
	}

	fclose(file);

	return true;
}

struct file file_open(const char* path) {
	FILE* handle = fopen(path, "rb");
	if (!handle) {
		return (struct file) { 0 };
	}

	fseek(handle, 0, SEEK_END);
	u64 size = ftell(handle);
	fseek(handle, 0, SEEK_SET);

	return (struct file) { handle, 0, 0, size };
}

bool file_good(struct file* file) {
	return file->handle != null;
}

void file_close(struct file* file) {
	fclose(file->handle);
	file->handle = null;
}

u64 file_seek(struct file* file, u64 offset) {
	file->cursor = offset;
}

u64 file_read(void* buf, u64 size, u64 count, struct file* file) {
	fseek(file->handle, file->pk_offset + file->cursor, SEEK_SET);

	file->cursor += size * count;

	return fread(buf, size, count, file->handle);
}
#else
bool read_raw(const char* path, u8** buf, u64* size, bool term) {
	FILE* file = fopen(package_path, "rb");
	if (!file) {
		fprintf(stderr, "Failed to open `%s'\n", package_path);
		return false;
	}

	fseek(file, 0, SEEK_SET);

	u64 header_el_size = sizeof(u64) * 3;
	u64 header_size;
	fread(&header_size, sizeof(header_size), 1, file);
	u64 header_count = header_size / header_el_size;

	u64 name_hash = elf_hash(path, strlen(path));

	for (u64 i = 0; i < header_count; i++) {
		u64 hash, offset, f_size;

		fread(&hash, sizeof(hash), 1, file);
		fread(&offset, sizeof(offset), 1, file);
		fread(&f_size, sizeof(f_size), 1, file);

		if (hash == name_hash) {
			if (size) {
				*size = f_size;
			}
			*buf = core_alloc(f_size + (term ? 1 : 0));

			fseek(file, offset, SEEK_SET);
			fread(*buf, 1, f_size, file);

			if (term) {
					*((*buf) + f_size) = '\0';
			}

			fclose(file);
			return true;
		}
	}
	
	fprintf(stderr, "Failed to read file from package: %s\n", path);

	fclose(file);
	return false;
}

struct file file_open(const char* path) {
	FILE* handle = fopen(package_path, "rb");
	if (!handle) {
		return (struct file) { 0 };
	}

	u64 header_el_size = sizeof(u64) * 3;
	u64 header_size;
	fread(&header_size, sizeof(header_size), 1, handle);
	u64 header_count = header_size / header_el_size;

	u64 name_hash = elf_hash(path, strlen(path));

	for (u64 i = 0; i < header_count; i++) {
		u64 hash, offset, f_size;

		fread(&hash, sizeof(hash), 1, handle);
		fread(&offset, sizeof(offset), 1, handle);
		fread(&f_size, sizeof(f_size), 1, handle);

		if (hash == name_hash) {
			return (struct file) { handle, offset, 0, f_size };
		}
	}

	fclose(handle);

	return (struct file) { 0 };
}

bool file_good(struct file* file) {
	return file->handle != null;
}

void file_close(struct file* file) {
	fclose(file->handle);
	file->handle = null;
}

u64 file_seek(struct file* file, u64 offset) {
	file->cursor = offset;
}

u64 file_read(void* buf, u64 size, u64 count, struct file* file) {
	fseek(file->handle, file->pk_offset + file->cursor, SEEK_SET);

	file->cursor += size * count;

	return fread(buf, size, count, file->handle);
}
#endif

enum {
	res_shader,
	res_texture,
	res_font,
	res_audio_clip
};

struct res {
	char* path;
	u32 type;
	union {
		struct texture* texture;
		struct shader shader;
		struct font* font;
		struct audio_clip* audio_clip;
	} as;
};

struct table* res_table;

static struct res* res_load(const char* path, u32 type, void* udata) {
	char cache_name[256];
	strcpy(cache_name, path);

	if (type == res_font) {
		sprintf(cache_name, "%s%g", path, *(float*)udata);
	}

	struct res* got = table_get(res_table, cache_name);
	if (got) {
		return got;
	}

	u8* raw;
	u64 raw_size;
	read_raw(path, &raw, &raw_size, type == res_shader);

	struct res new_res = { 0 };

	new_res.type = type;
	
	switch (new_res.type) {
		case res_shader:
			init_shader(&new_res.as.shader, raw, path);
			core_free(raw);
			break;
		case res_texture:
			new_res.as.texture = core_calloc(1, sizeof(struct texture));
			init_texture(new_res.as.texture, raw, raw_size);
			core_free(raw);
			break;
		case res_font:
			new_res.as.font = load_font_from_memory(raw, raw_size, *(float*)udata);
			break;
		case res_audio_clip:
			new_res.as.audio_clip = new_audio_clip(raw, raw_size);
			break;
		default: break;
	}

	table_set(res_table, cache_name, &new_res);

	return table_get(res_table, cache_name);
}

static void res_free(struct res* res) {
	switch (res->type) {
		case res_shader:
			deinit_shader(&res->as.shader);
			break;
		case res_texture:
			deinit_texture(res->as.texture);
			core_free(res->as.texture);
			break;
		case res_font:
			free_font(res->as.font);
			break;
		case res_audio_clip:
			free_audio_clip(res->as.audio_clip);
			break;
		default: break;
	}
}

void res_init() {
	res_table = new_table(sizeof(struct res));
}

void res_deinit() {
	for (struct table_iter i = new_table_iter(res_table); table_iter_next(&i);) {
		res_free(i.value);
	}

	free_table(res_table);
}

void res_unload(const char* path) {
	struct res* res = table_get(res_table, path);
	if (!res) {
		return;
	}

	res_free(res);

	table_delete(res_table, path);
}

struct shader load_shader(const char* path) {
	return res_load(path, res_shader, null)->as.shader;
}

struct texture* load_texture(const char* path) {
	return res_load(path, res_texture, null)->as.texture;
}

struct font* load_font(const char* path, float size) {
	return res_load(path, res_font, &size)->as.font;
}

struct audio_clip* load_audio_clip(const char* path) {
	return res_load(path, res_audio_clip, null)->as.audio_clip;
}
