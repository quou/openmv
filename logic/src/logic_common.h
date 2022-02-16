#pragma once

#include "common.h"

#ifdef PLATFORM_WINDOWS
#ifdef LOGIC_EXPORT_SYMBOLS
	#define LOGIC_API EXPORT_SYM
#else
	#define LOGIC_API IMPORT_SYM
#endif
#else
	#define LOGIC_API
#endif