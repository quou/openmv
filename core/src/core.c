#include <stdlib.h>
#include <string.h>

#include "core.h"

u64 elf_hash(const u8* data, u32 size) {
	u64 hash = 0, x = 0;

	for (u32 i = 0; i < size; i++) {
		hash = (hash << 4) + data[i];
		if ((x = hash & 0xF000000000LL) != 0) {
			hash ^= (x >> 24);
			hash &= ~x;
		}
	}

	return (hash & 0x7FFFFFFFFF);
}

u32 str_id(const char* str) {
	u32 r = 0;
	
	u32 len = (u32)strlen(str);

	for (u32 i = 0; i < len; i++) {
		r += (u32)str[i];
	}

	return r;
}

char* copy_string(const char* src) {
	const u32 len = (u32)strlen(src);

	char* s = core_alloc(len + 1);
	memcpy(s, src, len);
	s[len] = 0;

	return s;
}

#ifdef DEBUG
u64 memory_usage = 0;

void* core_alloc(u64 size) {
	u8* ptr = malloc(sizeof(u64) + size);

	memcpy(ptr, &size, sizeof(u64));

	memory_usage += size;

	return ptr + sizeof(u64);
}

void* core_calloc(u64 count, u64 size) {
	u64 alloc_size = count * size;

	u8* ptr = malloc(sizeof(u64) + alloc_size);

	memset(ptr + sizeof(u64), 0, alloc_size);

	memcpy(ptr, &alloc_size, sizeof(u64));

	memory_usage += size;

	return ptr + sizeof(u64);
}

void* core_realloc(void* p, u64 size) {
	u8* ptr = p;

	if (ptr) {
		u64* old_size = (u64*)(ptr - sizeof(u64));
		memory_usage -= *old_size;
	}

	void* new_ptr = realloc(ptr ? ptr - sizeof(u64) : null, sizeof(u64) + size);

	memcpy(new_ptr, &size, sizeof(u64));

	memory_usage += size;

	return new_ptr + sizeof(u64);
}

void core_free(void* p) {
	u8* ptr = p;

	u64* old_size = (u64*)(ptr - sizeof(u64));
	memory_usage -= *old_size;

	free(old_size);
}

u64 core_get_memory_usage() {
	return memory_usage;
}
#else
u64 core_get_memory_usage() {
	return 0;
}
#endif
