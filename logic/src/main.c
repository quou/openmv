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
#include "shop.h"
#include "sprites.h"
#include "ui.h"
#include "dynlib.h"

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

	if (logic_store->show_ui) {
		ui_text_input_event(ui, text);
	}
}

static void on_load_ask(bool yes, void* udata) {
	if (yes) {
		loadgame();
	}
}

static void on_resume(struct menu* menu) {
	logic_store->paused = false;
}

static void on_load(struct menu* menu) {
	prompt_ask("Do you want to load?", on_load_ask, null);

	logic_store->paused = false;
}

static void on_quit_ask(bool yes, void* udata) {
	if (yes) {
		set_window_should_close(main_window, true);
	}
}

static void on_quit(struct menu* menu) {
	prompt_ask("Do you want to stop playing? Unsaved progress will be lost!", on_quit_ask, null);
	logic_store->paused = false;
}

void init_debug_ui() {
	struct shader sprite_shader = load_shader("res/shaders/sprite.glsl");
	logic_store->ui = new_ui_context(sprite_shader, main_window, load_font("res/DejaVuSans.ttf", 14.0f));

	logic_store->debug_font = load_font("res/DejaVuSans.ttf", 12.0f);

	set_window_uptr(main_window, logic_store->ui);
	set_on_text_input(main_window, on_text_input);
}

API void CALL on_init() {
#ifndef PLATFORM_WINDOWS
	logic_store->dialogue_lib = open_dynlib("./libdialogue.so");

	if (!logic_store->dialogue_lib) {
		fprintf(stderr, "Failed to load libdialogue.so: %s\n", dynlib_get_error());
	}
#else
	logic_store->dialogue_lib = open_dynlib("dialogue.dll");
	
	if (!logic_store->dialogue_lib) {
		fprintf(stderr, "Failed to load dialogue.dll.\n");
	}
#endif	

	keymap_init();
	default_keymap();
	load_keymap();

	savegame_init();

	struct shader sprite_shader = load_shader("res/shaders/sprite.glsl");
	logic_store->renderer = new_renderer(sprite_shader, make_v2i(1366, 768));
	logic_store->renderer->camera_enable = true;
	logic_store->ui_renderer = new_renderer(sprite_shader, make_v2i(1366, 768));

	logic_store->explosion_sound = load_audio_clip("res/aud/explosion.wav");

	logic_store->paused = false;
	logic_store->frozen = false;
	logic_store->pause_menu = new_menu(load_font("res/CourierPrime.ttf", 35.0f));
	menu_add_label(logic_store->pause_menu, "= Paused =");
	menu_add_selectable(logic_store->pause_menu, "Resume", on_resume);
	menu_add_selectable(logic_store->pause_menu, "Load Save", on_load);
	menu_add_selectable(logic_store->pause_menu, "Quit", on_quit);

	prompts_init(load_font("res/CourierPrime.ttf", 25.0f));
	shops_init();

	struct world* world = new_world();
	logic_store->world = world;

	set_component_destroy_func(world, struct upgrade, on_upgrade_destroy);

	entity player = new_player_entity(world);
	struct player* pc = get_component(world, player, struct player);
	struct collider* pcol = get_component(world, player, struct collider);

	logic_store->player = player;

	if (savegame_exists()) {
		loadgame();
	} else {
		logic_store->room = load_room(world, "res/maps/a1/incinerator.dat");

		v2i spawn = get_spawn(logic_store->room);
		get_component(world, player, struct transform)->position = make_v2f(spawn.x - (pcol->rect.w / 2), spawn.y - pcol->rect.h);
	}
}

API void CALL on_update(double ts) {
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

	if (key_just_pressed(main_window, KEY_F12)) {
		if (logic_store->ui) {
			logic_store->show_ui = !logic_store->show_ui;
		} else {
			init_debug_ui();
			logic_store->show_ui = true;
		}
	}

	double time_scale;
	if (logic_store->frozen || logic_store->paused) {
		time_scale = 0.0;
	} else {
		time_scale = 1.0;
	}

	double timestep = ts * time_scale;

	i32 win_w, win_h;
	query_window(main_window, &win_w, &win_h);
	logic_store->ui_renderer->camera = m4f_orth(0.0f, (float)win_w, (float)win_h, 0.0f, -1.0f, 1.0f);

	if (!logic_store->frozen && !logic_store->paused) {
		player_system(world, renderer, &logic_store->room, timestep);
	}

	apply_lights(world, renderer);
	update_room_light(logic_store->room, renderer);
	update_player_light(world, renderer, logic_store->player);

	camera_system(world, renderer, logic_store->room, ts);
	enemy_system(world, logic_store->room, timestep);
	projectile_system(world, logic_store->room, timestep);
	fx_system(world, timestep);
	anim_fx_system(world, timestep);
	update_room(logic_store->room, timestep, ts);
	draw_room(logic_store->room, renderer, timestep);
	damage_fx_system(world, renderer, timestep);

	render_system(world, renderer, timestep);

	hud_system(world, logic_store->ui_renderer);

	draw_room_forground(logic_store->room, renderer, logic_store->ui_renderer);

	renderer_end_frame(renderer);

	if (!logic_store->show_ui) {
		renderer_flush(renderer);
	}

	if (logic_store->paused) {
		menu_update(logic_store->pause_menu);
	} else {
		prompts_update(ts);
		shops_update(ts);
	}

	renderer_flush(logic_store->ui_renderer);
	renderer_end_frame(logic_store->ui_renderer);

	if (logic_store->ui && logic_store->show_ui) {
		struct ui_context* ui = logic_store->ui;

		ui_begin_frame(ui);

		for (view(world, view, type_info(struct transform))) {
			struct transform* transform = view_get(&view, struct transform);

			char info_text[32];
			sprintf(info_text, "x: %.2f y: %.2f z: %d", transform->position.x, transform->position.y, transform->z);
			render_text(renderer, logic_store->debug_font, info_text, transform->position.x, transform->position.y - 14 * 2,
				make_color(0xffffff, 255));

			sprintf(info_text, "rotation: %.2f", transform->rotation);
			render_text(renderer, logic_store->debug_font, info_text, transform->position.x, transform->position.y - 14,
				make_color(0xffffff, 255));

			struct textured_quad quad = {
				.position = { transform->position.x, transform->position.y },
				.dimentions = { 50, 1 },
				.color = make_color(0xff0000, 255),
				.rotation = transform->rotation,
				.unlit = true
			};
			renderer_push(renderer, &quad);

			struct textured_quad quad2 = {
				.position = { transform->position.x, transform->position.y },
				.dimentions = { 1, 50 },
				.color = make_color(0x0000ff, 255),
				.rotation = transform->rotation,
				.unlit = true
			};
			renderer_push(renderer, &quad2);
		}

		for (view(world, view, type_info(struct transform), type_info(struct collider))) {
			struct transform* transform = view_get(&view, struct transform);
			struct collider* collider = view_get(&view, struct collider);

			struct textured_quad quad = {
				.position = { transform->position.x + collider->rect.x, transform->position.y + collider->rect.y },
				.dimentions = { collider->rect.w, collider->rect.h },
				.color = make_color(0x00ff00, 125),
				.rotation = transform->rotation,
				.unlit = true
			};
			renderer_push(renderer, &quad);
		}

		renderer_flush(renderer);
		renderer_end_frame(renderer);
		
		if (ui_begin_window(ui, "Debug", make_v2i(0, 0))) {
			ui_text(ui, logic_store->fps_buf);

			char buf[256];
			sprintf(buf, "Memory Usage (KIB): %g", round(((double)core_get_memory_usage() / 1024.0) * 100.0) / 100.0);
			ui_text(ui, buf);

			sprintf(buf, "Entities: %u", get_alive_entity_count(world));
			ui_text(ui, buf);

			sprintf(buf, "Pools: %u", get_component_pool_count(world));
			ui_text(ui, buf);

			if (ui_button(ui, "Give Coin")) {
				struct player* player = get_component(world, logic_store->player, struct player);

				player->money += 1;
			}

			if (ui_button(ui, "Reload Room")) {
				char* path = copy_string(get_room_path(logic_store->room));
				free_room(logic_store->room);
				logic_store->room = load_room(world, path);
				core_free(path);
			}

			if (ui_button(ui, "Save Game")) {
				savegame();
			}

			if (ui_button(ui, "Load Game")) {
				loadgame();
			}

			ui_end_window(ui);
		}

		ui_end_frame(ui);
	}
}

API void CALL on_deinit() {
	close_dynlib(logic_store->dialogue_lib);

	shops_deinit();
	prompts_deinit();
	free_menu(logic_store->pause_menu);

	free_room(logic_store->room);

	free_renderer(logic_store->renderer);
	free_renderer(logic_store->ui_renderer);

	if (logic_store->ui) {
		free_ui_context(logic_store->ui);
	}

	free_world(logic_store->world);

	savegame_deinit();

	keymap_deinit();
}
