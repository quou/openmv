#include <string.h>

#include "platform.h"

API struct window* main_window;

const char* get_file_name(const char* path) {
	const char* c;
	u32 len = (u32)strlen(path);

	for (c = path + len; c != path; c--) {
		if (*c == '/') {
			return c + 1;
		}
	}

	return c;
}

const char* get_file_extension(const char* name) {
	const char* c;

	for (c = name; *c; c++) {
		if (*c == '.') {
			return c + 1;
		}
	}

	return c;
}
