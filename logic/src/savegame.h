#pragma once

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
	persist_float,
	persist_str,
};

struct persistent {
	u32 type;

	union {
		i32 i;
		i32 u;
		bool b;
		double f;
		char* str;
	} as;
};

API void set_persistent(const char* name, u32 type, const void* val);
API struct persistent* get_persistent(const char* name);
