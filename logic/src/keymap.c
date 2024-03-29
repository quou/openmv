#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "keymap.h"
#include "platform.h"
#include "table.h"
#include "logic_store.h"

#define key_table_set(n_, k_) \
	do { \
		i32 k = k_; \
		table_set(keymap, (n_), &k); \
	} while (0)

extern struct logic_store* logic_store;

void keymap_init() {
	struct table** keymap = (struct table**)&logic_store->keymap;

	*keymap = new_table(sizeof(i32));
}

void keymap_deinit() {
	struct table* keymap = (struct table*)logic_store->keymap;

	free_table(keymap);
}

void default_keymap() {
	struct table* keymap = (struct table*)logic_store->keymap;

	key_table_set("jump",     key_Z);
	key_table_set("fire",     key_X);
	key_table_set("dash",     key_C);
	key_table_set("left",     key_left);
	key_table_set("right",    key_right);
	key_table_set("up",       key_up);
	key_table_set("down",     key_down);
	key_table_set("interact", key_down);
	key_table_set("submit",   key_return);
}

void save_keymap() {
	struct table* keymap = (struct table*)logic_store->keymap;

	FILE* file = fopen("keymap", "wb");
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
	struct table* keymap = (struct table*)logic_store->keymap;

	FILE* file = fopen("keymap", "rb");
	if (!file) {
		default_keymap();
		save_keymap();
		return;
	}

	while (!feof(file)) {
		u32 len;
		fread(&len, sizeof(len), 1, file);
		char* name = core_calloc(len + 1, 1);
		name[len] = '\0';
		fread(name, 1, len, file);
		i32 key;
		fread(&key, sizeof(key), 1, file);
		key_table_set(name, key);
		core_free(name);
	}

	fclose(file);
}

i32 mapped_key(const char* name) {
	struct table* keymap = (struct table*)logic_store->keymap;

	i32* k = table_get(keymap, name);

	return k ? *k : 0;
}
