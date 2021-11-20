#pragma once

#include "entity.h"
#include "room.h"
#include "ui.h"
#include "video.h"
#include "menu.h"

struct logic_store {
	double fps_timer;
	char fps_buf[256];

	struct renderer* renderer;
	struct ui_context* ui;

	struct world* world;
	struct room* room;

	struct menu* pause_menu;
	bool paused;

	void* keymap;

	v2f camera_position;

	entity player;
};

extern struct logic_store* logic_store;
