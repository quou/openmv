#pragma once

#include "common.h"

/* This is a basic hash table implementation that uses
 * a generic value and a string key.
 *
 * It uses open addressing with linear probing, which means
 * although it's not the fastest hash table in the world,
 * it is certainly enough for anywhere in the game where
 * a string hash table is needed. */

struct table;
struct table_iter;

API struct table* new_table(u32 element_size);
API void free_table(struct table* table);

API void* table_get(struct table* table, const char* key);
API void* table_set(struct table* table, const char* key, const void* val);
API void table_delete(struct table* table, const char* key);

API const char* table_get_key(struct table* table, const char* key);

struct table_iter {
	struct table* table;
	u32 i;
	const char* key;
	void* value;
};

API struct table_iter new_table_iter(struct table* table);
API bool table_iter_next(struct table_iter* iter);

#define table_iter(t_, n_) \
	struct table_iter n_ = new_table_iter((t_));\
	table_iter_next(&(n_));
