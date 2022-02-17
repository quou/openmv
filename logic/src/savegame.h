#pragma once

#include "logic_common.h"

void savegame_init();
void savegame_deinit();

void savegame();
void loadgame();

bool savegame_exists();

void ask_savegame();

enum {
	persist_i32 = 0,
	persist_u32,
	persist_bool,
	persist_f32,
	persist_str,
};

struct persistent {
	u32 type;

	union {
		i32 i;
		i32 u;
		bool b;
		f64 f;
		char* str;
	} as;
};

LOGIC_API void set_persistent(const char* name, u32 type, const void* val);
LOGIC_API struct persistent* get_persistent(const char* name);
