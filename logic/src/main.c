#include <math.h>
#include <stdio.h>

#include "consts.h"
#include "core.h"
#include "coresys.h"
#include "enemy.h"
#include "entity.h"
#include "fx.h"
#include "keymap.h"
#include "logic_store.h"
#include "menu.h"
#include "player.h"
#include "res.h"
#include "savegame.h"
#include "sprites.h"
#include "ui.h"

struct logic_store* logic_store;

API u64 CALL get_storage_size() {
	return sizeof(struct logic_store);
}

API void CALL on_reload(void* instance) {
	logic_store = instance;

	/* Sprites are also reloaded every time the code is.
	 * This is because the sprite textures defined in `sprites.c'
	 * are reset every reload, and so will become invalid.
	 *
	 * The sprite files are not reloaded, only looked up again
	 * in the resource manager cache, so this is fairly quick. */
	preload_sprites();
}

static void on_text_input(struct window* window, const char* text, void* udata) {
	struct ui_context* ui = udata;

	ui_text_input_event(ui, text);
}

static void on_save_ask(bool yes) {
	if (yes) {
		savegame();
	}
}

static void on_load_ask(bool yes) {
	if (yes) {
		loadgame();
	}
}

static void on_resume(struct menu* menu) {
	logic_store->paused = false;
}

static void on_save(struct menu* menu) {
	prompt_ask("Do you want to save?", on_save_ask);

	logic_store->paused = false;
}

static void on_load(struct menu* menu) {
	prompt_ask("Do you want to load?", on_load_ask);

	logic_store->paused = false;
}

static void on_quit_ask(bool yes) {
	if (yes) {
		set_window_should_close(main_window, true);
	}
}

static void on_quit(struct menu* menu) {
	prompt_ask("Do you want to stop playing? Unsaved progress will be lost!", on_quit_ask);
	logic_store->paused = false;
}

API void CALL on_init() {
	keymap_init();
	default_keymap();
	load_keymap();

	struct shader sprite_shader = load_shader("res/shaders/sprite.glsl");
	logic_store->renderer = new_renderer(sprite_shader, make_v2i(1366, 768));
	logic_store->renderer->camera_enable = true;

	logic_store->ui = new_ui_context(sprite_shader, main_window, load_font("res/DejaVuSans.ttf", 14.0f));

	logic_store->paused = false;
	logic_store->frozen = false;
	logic_store->pause_menu = new_menu(sprite_shader, load_font("res/DejaVuSansMono.ttf", 35.0f));
	menu_add_label(logic_store->pause_menu, "= Paused =");
	menu_add_selectable(logic_store->pause_menu, "Resume", on_resume);
	menu_add_selectable(logic_store->pause_menu, "Save Game", on_save);
	menu_add_selectable(logic_store->pause_menu, "Load Save", on_load);
	menu_add_selectable(logic_store->pause_menu, "Quit", on_quit);

	prompts_init(sprite_shader, load_font("res/DejaVuSansMono.ttf", 25.0f));

	set_window_uptr(main_window, logic_store->ui);
	set_on_text_input(main_window, on_text_input);

	struct world* world = new_world();
	logic_store->world = world;

	logic_store->room = load_room(world, "res/maps/test_room.dat");

	entity player = new_player_entity(world);
	struct player* pc = get_component(world, player, struct player);

	v2i spawn = get_spawn(logic_store->room);
	pc->position = make_v2f(spawn.x - (pc->collider.w / 2), spawn.y - pc->collider.h);

	logic_store->player = player;
}

API void CALL on_update(double ts) {
	struct ui_context* ui = logic_store->ui;
	struct renderer* renderer = logic_store->renderer;
	struct world* world = logic_store->world;

	logic_store->fps_timer += ts;
	if (logic_store->fps_timer > 1.0) {
		sprintf(logic_store->fps_buf, "FPS: %g    Timestep: %g", 1.0 / ts, ts);
		logic_store->fps_timer = 0.0;
	}

	if (key_just_pressed(main_window, KEY_ESCAPE)) {
		menu_reset_selection(logic_store->pause_menu);
		logic_store->paused = !logic_store->paused;
	}
	
	double time_scale;
	if (logic_store->frozen || logic_store->paused) {
		time_scale = 0.0;
	} else {
		time_scale = 1.0;
	}

	double timestep = ts * time_scale;

	if (!logic_store->frozen && !logic_store->paused) { player_system(world, renderer, &logic_store->room, timestep); }
	enemy_system(world, timestep);
	projectile_system(world, logic_store->room, timestep);
	fx_system(world, timestep);
	anim_fx_system(world, timestep);
	draw_room(logic_store->room, renderer);	
	damage_fx_system(world, renderer, timestep);

	render_system(world, renderer, timestep);

	renderer_flush(renderer);

	if (logic_store->paused) {
		menu_update(logic_store->pause_menu);
	} else {
		prompts_update(ts);
	}

	ui_begin_frame(ui);
	
	if (ui_begin_window(ui, "Debug", make_v2i(0, 0))) {
		ui_text(ui, logic_store->fps_buf);

		char mem_buf[256];
		sprintf(mem_buf, "Memory Usage (KIB): %g", round(((double)core_get_memory_usage() / 1024.0) * 100.0) / 100.0);
		ui_text(ui, mem_buf);

		ui_end_window(ui);
	}

	ui_end_frame(ui);
}

API void CALL on_deinit() {
	prompts_deinit();
	free_menu(logic_store->pause_menu);

	free_room(logic_store->room);

	free_renderer(logic_store->renderer);
	free_ui_context(logic_store->ui);
	free_world(logic_store->world);

	keymap_deinit();
}
