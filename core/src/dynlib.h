#pragma once

#include "common.h"

void* open_dynlib(const char* path);
void close_dynlib(void* handle);
void* dynlib_get_sym(void* handle, const char* name);
const char* dynlib_get_error();
