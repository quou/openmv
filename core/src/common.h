#pragma once

#include <stdbool.h>
#include <stdint.h>

#define null (void*)0x0

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* Platform stuff */
#if defined(_WIN32) || defined(__CYGWIN__)
	#define PLATFORM_WINDOWS
#elif defined(__linux__)
	#define PLATFORM_LINUX
#else
	#error platform not supported.
#endif

#if defined(EXPORT_SYMBOLS)
	#if defined(PLATFORM_WINDOWS)
		#define API __declspec(dllexport)
	#else
		#define API
	#endif
#elif defined(IMPORT_SYMBOLS)
	#if defined(PLATFORM_WINDOWS)
		#define API __declspec(dllimport)
	#else
		#define API
	#endif
#endif

#if defined(_MSC_VER)
	#define CDECL __cdecl
#else
	#define CDECL
#endif
