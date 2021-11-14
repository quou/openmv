#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "res.h"
#include "table.h"

bool read_raw(const char* path, u8** buf, u32* size, bool term) {
	*buf = null;
	size ? *size = 0 : 0;

	FILE* file = fopen(path, "rb");
	if (!file) {
		printf("Failed to fopen file %s\n", path);
		return false;
	}

	fseek(file, 0, SEEK_END);
	const u32 file_size = ftell(file);
	rewind(file);

	*buf = core_alloc(file_size + (term ? 1 : 0));
	const u32 bytes_read = (u32)fread(*buf, sizeof(char), file_size, file);
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

enum {
	res_shader,
	res_texture,
	res_font
};

struct res {
	char* path;
	u32 type;
	union {
		struct texture texture;
		struct shader shader;
		struct font* font;
	} as;
};

struct table* res_table;

static struct res* res_load(const char* path, u32 type, void* udata) {
	void* got = table_get(res_table, path);
	if (got) {
		return got;
	}

	u8* raw;
	u32 raw_size;
	read_raw(path, &raw, &raw_size, type == res_shader);

	struct res new_res = { 0 };

	new_res.type = type;
	
	switch (new_res.type) {
		case res_shader:
			init_shader(&new_res.as.shader, raw, path);
			core_free(raw);
			break;
		case res_texture:
			init_texture(&new_res.as.texture, raw, raw_size);
			core_free(raw);
			break;
		case res_font:
			new_res.as.font = load_font_from_memory(raw, raw_size, *(float*)udata);
			break;
	}

	table_set(res_table, path, &new_res);

	return table_get(res_table, path);
}

static void res_core_free(struct res* res) {
	switch (res->type) {
		case res_shader:
			deinit_shader(&res->as.shader);
			break;
		case res_texture:
			deinit_texture(&res->as.texture);
			break;
		case res_font:
			free_font(res->as.font);
			break;
	}
}

void res_init() {
	res_table = new_table(sizeof(struct res));
}

void res_deinit() {
	for (struct table_iter i = new_table_iter(res_table); table_iter_next(&i);) {
		res_core_free(i.value);
	}

	free_table(res_table);
}

void res_unload(const char* path) {
	struct res* res = table_get(res_table, path);
	if (!res) {
		return;
	}

	res_core_free(res);

	table_delete(res_table, path);
}

struct shader* load_shader(const char* path) {
	return &res_load(path, res_shader, null)->as.shader;
}

struct texture* load_texture(const char* path) {
	return &res_load(path, res_texture, null)->as.texture;
}

struct font* load_font(const char* path, float size) {
	return res_load(path, res_font, &size)->as.font;
}
