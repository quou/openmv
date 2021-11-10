#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "core.h"

#define load_factor 0.75

struct table_el {
	char* key;
	i32 val_idx;
};

struct table {
	u32 element_size;

	void* data;
	u32 data_count;
	u32 data_capacity;

	struct table_el* els;
	u32 count;
	u32 capacity;
};

static struct table_el* find_el(struct table_el* els, u32 capacity, const char* key) {
	u32 idx = elf_hash((u8*)key, (u32)strlen(key)) % capacity;

	struct table_el* tombstone = null;

	for (;;) {
		struct table_el* el = &els[idx];
		if (el->key == null) {
			if (el->val_idx != -2) {
				return tombstone != null ? tombstone : el;
			} else if (tombstone == null) {
				tombstone = el;
			}
		} else if (strcmp(el->key, key) == 0) {
			return el;
		}

		idx = (idx + 1) % capacity;
	}
}

static void table_resize(struct table* table, u32 capacity) {
	struct table_el* els = malloc(capacity * sizeof(struct table_el));
	for (u32 i = 0; i < capacity; i++) {
		els[i].key = null;
		els[i].val_idx = -1;
	}

	for (u32 i = 0; i < table->capacity; i++) {
		struct table_el* el = &table->els[i];
		if (!el->key) { continue; }

		struct table_el* dst = find_el(els, capacity, el->key);
		dst->key = el->key;
		dst->val_idx = el->val_idx;
	}

	if (table->els) { free(table->els); }

	table->els = els;
	table->capacity = capacity;
}

static i32 table_data_add(struct table* table) {
	if (table->data_count >= table->data_capacity) {
		table->data_capacity = table->data_capacity < 8 ? 8 : table->data_capacity * 2;
		table->data = realloc(table->data, table->data_capacity * table->element_size);
	}

	return table->data_count++;
}

static void table_data_set(struct table* table, i32 idx, const void* ptr) {
	memcpy(&((char*)table->data)[idx * table->element_size], ptr, table->element_size);
}

static void* table_data_get(struct table* table, i32 idx) {
	return &((char*)table->data)[idx * table->element_size]; 
}

struct table* new_table(u32 element_size) {
	struct table* table = malloc(sizeof(struct table));

	*table = (struct table) {
		.element_size = element_size,

		.data = null,
		.data_count = 0,
		.data_capacity = 0,

		.els = null,
		.count = 0,
		.capacity = 0
	};

	return table;
}

void free_table(struct table* table) {
	for (u32 i = 0; i < table->capacity; i++) {
		if (table->els[i].key) {
			free(table->els[i].key);
		}
	}

	if (table->data) { free(table->data); }
	if (table->els)  { free(table->els); }

	free(table);
}

void* table_get(struct table* table, const char* key) {
	if (table->count == 0) { return null; }

	struct table_el* el = find_el(table->els, table->capacity, key);
	if (!el->key || el->val_idx < 0) { return null; }

	return table_data_get(table, el->val_idx);
}

const char* table_get_key(struct table* table, const char* key) {
	if (table->count == 0) { return null; }

	struct table_el* el = find_el(table->els, table->capacity, key);
	if (!el->key || el->val_idx < 0) { return null; }

	return el->key;
}

void* table_set(struct table* table, const char* key, const void* val) {
	if (table->count >= table->capacity * load_factor) {
		u32 capacity = table->capacity < 8 ? 8 : table->capacity * 2;
		table_resize(table, capacity);
	}

	struct table_el* el = find_el(table->els, table->capacity, key);
	if (!el->key) { /* New key. */
		table->count++;
		el->val_idx = table_data_add(table);
	} else {
		free(el->key);
	}

	const u32 key_len = (u32)strlen(key);

	el->key = malloc(key_len + 1);
	strcpy(el->key, key);
	el->key[key_len] = '\0';

	table_data_set(table, el->val_idx, val);

	return table_data_get(table, el->val_idx);
}

void table_delete(struct table* table, const char* key) {
	if (table->count == 0) { return; }

	struct table_el* el = find_el(table->els, table->capacity, key);
	if (!el->key) { return; }

	free(el->key);

	/* -2 marks a key/value pair as a tombstone;
	 * Gone but not deleted. */
	el->key = null;
	el->val_idx = -2;
}

/* Iterator. */
struct table_iter new_table_iter(struct table* table) {
	if (table->count == 0) {
		return (struct table_iter) {
			.table = table,
			.i = 0,
			.key = null,
			.value = null
		};
	}

	u32 start;
	for (start = 0; start < table->capacity; start++) {
		if (table->els[start].key) { break; }
	}

	return (struct table_iter) {
		.table = table,
		.i = start,
		.key = table->els[start].key,
		.value = table_data_get(table, table->els[start].val_idx)
	};
}

bool table_iter_next(struct table_iter* iter) {
	struct table* table = iter->table;

	if (iter->i >= table->capacity) { return false; }

	for (u32 i = iter->i; i < table->capacity; i++) {
		struct table_el* el = &table->els[i];
		if (el->key) {
			iter->key = el->key;
			iter->value = table_data_get(table, el->val_idx);
			iter->i++;
			return true;
		}
		iter->i++;
	}

	return false;
}
