#pragma once

#include "common.h"

/* Wraps libdl on Linux, and some Win32 API stuff on Windows for
 * loading dynamic libraries.
 *
 * TODO: Figure out a way to pack dynamic libraries into res.pck?
 * Not sure if that's possible without dumping them to external files
 * before loading. */

API void* open_dynlib(const char* path);
API void close_dynlib(void* handle);
API void* dynlib_get_sym(void* handle, const char* name);
API const char* dynlib_get_error();
