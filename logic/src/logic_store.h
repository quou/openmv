#pragma once

#include "entity.h"
#include "room.h"
#include "ui.h"
#include "video.h"

struct logic_store {
	double fps_timer;
	char fps_buf[256];

	struct renderer* renderer;
	struct ui_context* ui;

	struct world* world;
	struct room* room;

	void* keymap;
	void* selected_layer;
	u8 selected_tile;
};
