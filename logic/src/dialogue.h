#pragma once

#include "common.h"
#include "table.h"
#include "video.h"

typedef void (*on_dialogue_next_func)();
typedef void (*on_dialogue_play_func)(void* ctx);

struct dialogue {
	struct rect rect;
	on_dialogue_next_func on_next;
	on_dialogue_play_func on_play;
	bool want_next;
};

void init_dialogue();
void deinit_dialogue();

void* get_dialogue_fun(const char* name);
