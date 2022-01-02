#pragma once

#include "entity.h"
#include "menu.h"
#include "room.h"
#include "ui.h"
#include "video.h"

struct logic_store {
	double fps_timer;
	char fps_buf[256];

	struct renderer* renderer;
	struct renderer* ui_renderer;
	struct ui_context* ui;

	struct world* world;
	struct room* room;

	struct menu* pause_menu;
	bool paused;
	bool frozen;

	void* keymap;
	void* prompt_ctx;

	v2f camera_position;

	entity player;

	struct audio_clip* explosion_sound;

	struct table* savegame_persist;
};

extern struct logic_store* logic_store;
