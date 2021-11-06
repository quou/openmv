#pragma once

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
};

#define type_info(t_) ((struct type_info) { str_id(#t_), sizeof(t_) })
