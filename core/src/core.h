#pragma once

#include <stdlib.h>

#include "common.h"

struct window;
API extern struct window* main_window;

#define prop_offset(t_, e_) ((u64)(&(((t_*)(0))->e_)))

#define _get_nth_arg(_1, _2, _3, _4, N, ...) N
#define count_va_args(...) _get_nth_arg(__VA_ARGS__, 4, 3, 2, 1)

/* Return the hash of a string using the ELF hash algorithm*/
API u64 elf_hash(const u8* data, u32 size);

/* Sum up all the characters in a string, creating an ID for
 * that string. Not to be used in place of a hash function. */
API u32 str_id(const char* str);

struct type_info {
	u32 id;
	u32 size;
	const char* name;
};

#define type_info(t_) ((struct type_info) { str_id(#t_), sizeof(t_), (#t_) })

API char* copy_string(const char* src);

API void* core_alloc(u64 size);
API void* core_calloc(u64 count, u64 size);
API void* core_realloc(void* ptr, u64 size);
API void core_free(void* ptr);

API u64 core_get_memory_usage();

API i32 random_int(i32 min, i32 max);
API bool random_chance(double chance);
