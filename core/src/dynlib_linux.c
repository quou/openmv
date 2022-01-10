#include <dlfcn.h>

#include "dynlib.h"

void* open_dynlib(const char* path) {
	return dlopen(path, RTLD_NOW);
}

void close_dynlib(void* handle) {
	dlclose(handle);
}

void* dynlib_get_sym(void* handle, const char* name) {
	return dlsym(handle, name);
}

const char* dynlib_get_error() {
	return dlerror();
}
