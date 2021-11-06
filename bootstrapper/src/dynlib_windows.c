#include <windows.h>

#include "dynlib.h"

void* open_dynlib(const char* path) {
	return LoadLibraryA(path);
}

void close_dynlib(void* handle) {
	FreeLibrary(handle);
}

void* dynlib_get_sym(void* handle, const char* name) {
	return GetProcAddress(handle, name);
}

const char* dynlib_get_error() {
	return "";
}
