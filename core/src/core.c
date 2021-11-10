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

	char* s = malloc(len + 1);
	memcpy(s, src, len);
	s[len] = 0;

	return s;
}
