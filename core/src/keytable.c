#include "keytable.h"

void init_key_table(struct key_table* table) {
	i32 i;

	for (i = 0; i < KEY_COUNT; i++) {
		table->elements[i] = (struct key_table_item) { -1, 0 };
	}
}

i32 search_key_table(struct key_table* table, i32 key) {
	i32 hash_idx, i;

	hash_idx = key % KEY_COUNT;

	i = 0;

	while (table->elements[hash_idx].key != key && i < KEY_COUNT) {
		hash_idx++;
		hash_idx %= KEY_COUNT;

		i++;
	}

	if (i >= KEY_COUNT) { return -1; }

	return table->elements[hash_idx].value;
}

void key_table_insert(struct key_table* table, i32 key, i32 value) {
	i32 hash_idx;
	struct key_table_item item = { key, value };

	hash_idx = key % KEY_COUNT;

	while (table->elements[hash_idx].key != -1) {
		hash_idx++;
		hash_idx %= KEY_COUNT;
	}

	table->elements[hash_idx] = item;
}
