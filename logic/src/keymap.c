#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "keymap.h"
#include "platform.h"
#include "table.h"

#define key_table_set(n_, k_) \
	do { \
		i32 k = k_; \
		table_set(keymap, (n_), &k); \
	} while (0) \

struct table* keymap;

void keymap_init() {
	keymap = new_table(sizeof(i32));
}

void keymap_deinit() {
	free_table(keymap);
}

void default_keymap() {
	key_table_set("jump",  KEY_Z);
	key_table_set("left",  KEY_LEFT);
	key_table_set("right", KEY_RIGHT);
}

void save_keymap() {
	FILE* file = fopen("keymap", "w");
	if (!file) {
		fprintf(stderr, "Failed to open `keymap' for writing.\n");
		return;
	}

	for (table_iter(keymap, i)) {
		u32 len = (u32)strlen(i.key);
		i32 key = *(i32*)i.value;
		fwrite(&len, sizeof(len), 1, file);
		fwrite(i.key, 1, len, file);
		fwrite(&key, sizeof(key), 1, file);
	}

	fclose(file);
}

void load_keymap() {
	FILE* file = fopen("keymap", "r");
	if (!file) {
		default_keymap();
		save_keymap();
		return;
	}

	while (!feof(file)) {
		u32 len;
		fread(&len, sizeof(len), 1, file);
		char* name = malloc(len + 1);
		name[len] = '\0';
		fread(name, 1, len, file);
		i32 key;
		fread(&key, sizeof(key), 1, file);
		key_table_set(name, key);
		free(name);
	}

	fclose(file);
}

i32 mapped_key(const char* name) {
	i32* k = table_get(keymap, name);

	return k ? *k : 0;
}
