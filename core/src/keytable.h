#pragma once

#include "common.h"
#include "platform.h"

/* The key table is a very basic integer hash table
 * for mapping system virtual keycodes to standardized ones. */

struct key_table_item {
	i32 key, value;
};

struct key_table {
	struct key_table_item elements[KEY_COUNT];
};

API void init_key_table(struct key_table* table);
API i32 search_key_table(struct key_table* table, i32 key);
API void key_table_insert(struct key_table* table, i32 key, i32 value);
