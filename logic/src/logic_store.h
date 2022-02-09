#pragma once

#include <stdio.h>

#include "entity.h"
#include "lsp.h"
#include "menu.h"
#include "room.h"
#include "ui.h"
#include "video.h"

struct logic_store {
	double fps_timer;
	char fps_buf[256];

	struct renderer* renderer;
	struct renderer* hud_renderer;
	struct renderer* ui_renderer;
	struct ui_context* ui;
	bool show_ui;
	bool show_components;

	struct font* debug_font;

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

	void* dialogue_lib;

	struct lsp_state* lsp;
	char lsp_buf[256];

	FILE* lsp_out;
};

extern struct logic_store* logic_store;

void load_default_room();
