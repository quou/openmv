#include <math.h>
#include <stdio.h>

#include "consts.h"
#include "core.h"
#include "coresys.h"
#include "dialogue.h"
#include "dynlib.h"
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
#include "imui.h"

struct logic_store* logic_store;

EXPORT_SYM u64 C_DECL get_storage_size() {
	return sizeof(struct logic_store);
}

EXPORT_SYM void C_DECL on_reload(void* instance) {
	logic_store = instance;

	/* Sprites are also reloaded every time the code is.
	 * This is because the sprite textures defined in `sprites.c'
	 * are reset every reload, and so will become invalid.
	 *
	 * The sprite files are not reloaded, only looked up again
	 * in the resource manager cache, so this is fairly quick. */
	preload_sprites();
}

EXPORT_SYM struct window* C_DECL create_window() {
	return new_window(make_v2i(1366, 768), "OpenMV", false);
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

static void on_tutorial(struct menu* menu) {
	message_prompt(" - Move: Arrow keys\n - Jump: Z\n - Shoot: X\n - Interact: Down Arrow\n - Look Up: Up Arrow");
	logic_store->paused = false;
}

void init_debug_ui() {
	struct shader sprite_shader = load_shader("res/shaders/sprite.glsl");
	logic_store->ui = new_ui_context(sprite_shader, main_window, load_font("res/DejaVuSans.ttf", 14.0f));

	logic_store->debug_font = load_font("res/DejaVuSans.ttf", 12.0f);

	set_window_uptr(main_window, logic_store->ui);
	set_on_text_input(main_window, on_text_input);
}

void load_default_room() {
	if (logic_store->room) {
		free_room(logic_store->room);
	}

	logic_store->room = load_room(logic_store->world, "res/maps/a1/incinerator.dat");

	v2i spawn = get_spawn(logic_store->room);
	struct collider* pcol = get_component(logic_store->world, logic_store->player, struct collider);
	get_component(logic_store->world, logic_store->player, struct transform)->position =
		make_v2f(spawn.x - (pcol->rect.w / 2), spawn.y - pcol->rect.h);
}

static struct lsp_val command_window_size(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
	lsp_arg_assert(ctx, args[0], lsp_val_num, "Argument 0 to `window_size' must be a number.");
	lsp_arg_assert(ctx, args[1], lsp_val_num, "Argument 1 to `window_size' must be a number.");

	set_window_size(main_window, make_v2i((i32)args[0].as.num, (i32)args[1].as.num));
	return lsp_make_nil();
}

static struct lsp_val command_fullscreen(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
	lsp_arg_assert(ctx, args[0], lsp_val_bool, "Argument 0 to `fullscreen' must be a boolean.");

	set_window_fullscreen(main_window, lsp_as_bool(args[0]));

	return lsp_make_nil();
}

EXPORT_SYM void C_DECL on_init() {
	logic_store->lsp_out = fopen("command.log", "w");
	if (!logic_store->lsp_out) {
		fprintf(stderr, "Failed to open `command.log'\n");
	}

	logic_store->lsp = new_lsp_state(logic_store->lsp_out, logic_store->lsp_out);
	lsp_register_std(logic_store->lsp);
	lsp_register(logic_store->lsp, "window_size", 2, command_window_size);
	lsp_register(logic_store->lsp, "fullscreen", 1, command_fullscreen);

	FILE* autoexec_file = fopen("autoexec.lsp", "rb");
	if (autoexec_file) {
		fclose(autoexec_file);
		lsp_do_file(logic_store->lsp, "autoexec.lsp");
	}

	init_dialogue();

	keymap_init();
	default_keymap();
	load_keymap();

	savegame_init();

	struct shader sprite_shader = load_shader("res/shaders/sprite.glsl");
	logic_store->renderer = new_renderer(sprite_shader, make_v2i(1366, 768));
	logic_store->renderer->camera_enable = true;
	logic_store->hud_renderer = new_renderer(sprite_shader, make_v2i(1366, 768));
	logic_store->ui_renderer = new_renderer(sprite_shader, make_v2i(1366, 768));

	logic_store->explosion_sound = load_audio_clip("res/aud/explosion.wav");

	logic_store->paused = false;
	logic_store->frozen = false;
	logic_store->pause_menu = new_menu(load_font("res/CourierPrime.ttf", 35.0f));
	menu_add_label(logic_store->pause_menu, "= Paused =");
	menu_add_selectable(logic_store->pause_menu, "Resume", on_resume);
	menu_add_selectable(logic_store->pause_menu, "Load Save", on_load);
	menu_add_selectable(logic_store->pause_menu, "Tutorial", on_tutorial);
	menu_add_selectable(logic_store->pause_menu, "Quit", on_quit);

	prompts_init(load_font("res/CourierPrime.ttf", 25.0f));
	shops_init();

	struct world* world = new_world();
	logic_store->world = world;

	set_component_destroy_func(world, struct upgrade, on_upgrade_destroy);

	entity player = new_player_entity(world);

	logic_store->player = player;

	if (savegame_exists()) {
		loadgame();
	} else {
		load_default_room();
	}
}

EXPORT_SYM void C_DECL on_update(f64 ts) {
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

	if (logic_store->show_ui && key_just_pressed(main_window, KEY_F11)) {
		logic_store->show_components = !logic_store->show_components;
	}

	f64 time_scale;
	if (logic_store->frozen || logic_store->paused) {
		time_scale = 0.0;
	} else {
		time_scale = 1.0;
	}

	f64 timestep = ts * time_scale;

	i32 win_w, win_h;
	query_window(main_window, &win_w, &win_h);
	logic_store->ui_renderer->dimentions = make_v2i(win_w, win_h);
	logic_store->ui_renderer->camera = m4f_orth(0.0f, (f32)win_w, (f32)win_h, 0.0f, -1.0f, 1.0f);

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

	draw_room_forground(logic_store->room, renderer, logic_store->ui_renderer);

	hud_system(world, logic_store->hud_renderer);

	renderer_end_frame(renderer);

	if (!logic_store->show_ui) {
		renderer_flush(renderer);
	}

	renderer_end_frame(logic_store->hud_renderer);

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

			if (logic_store->show_components) {
				struct type_info types[32];

				u32 c = get_entity_component_types(world, view.e, types, 32);

				char info_text[32];
				sprintf(info_text, "id: %u version: %u", get_entity_id(view.e), get_entity_version(view.e));
				render_text(renderer, logic_store->debug_font, info_text, transform->position.x, transform->position.y - (14 * 3) - (c * 14),
					make_color(0xffffff, 255));

				sprintf(info_text, "x: %.2f y: %.2f z: %d", transform->position.x, transform->position.y, transform->z);
				render_text(renderer, logic_store->debug_font, info_text, transform->position.x, transform->position.y - (14 * 2) - (c * 14),
					make_color(0xffffff, 255));

				sprintf(info_text, "rotation: %.2f", transform->rotation);
				render_text(renderer, logic_store->debug_font, info_text, transform->position.x, transform->position.y - 14 - (c * 14),
				make_color(0xffffff, 255));

				for (u32 i = 0; i < c; i++) {
					render_text(renderer, logic_store->debug_font, types[i].name, transform->position.x, transform->position.y - 14 - (i * 14),
						make_color(0xffffff, 255));
				}
			} else {
				char info_text[32];
				sprintf(info_text, "id: %u version: %u", get_entity_id(view.e), get_entity_version(view.e));
				render_text(renderer, logic_store->debug_font, info_text, transform->position.x, transform->position.y - (14 * 3),
					make_color(0xffffff, 255));

				sprintf(info_text, "x: %.2f y: %.2f z: %d", transform->position.x, transform->position.y, transform->z);
				render_text(renderer, logic_store->debug_font, info_text, transform->position.x, transform->position.y - (14 * 2),
					make_color(0xffffff, 255));

				sprintf(info_text, "rotation: %.2f", transform->rotation);
				render_text(renderer, logic_store->debug_font, info_text, transform->position.x, transform->position.y - 14,
					make_color(0xffffff, 255));
			}

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
			sprintf(buf, "Memory Usage (KIB): %g", round(((f64)core_get_memory_usage() / 1024.0) * 100.0) / 100.0);
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

		if (ui_begin_window(ui, "Commands", make_v2i(0, 300))) {
			ui_text_input(ui, "Command", logic_store->lsp_buf, 256);
			
			if (ui_button(ui, "Submit")) {
				lsp_do_string(logic_store->lsp, "command", logic_store->lsp_buf);
				logic_store->lsp_buf[0] = 0;
			}

			ui_end_window(ui);
		}

		ui_end_frame(ui);
	}
}

EXPORT_SYM void C_DECL on_deinit() {
	deinit_dialogue();

	shops_deinit();
	prompts_deinit();
	free_menu(logic_store->pause_menu);

	free_room(logic_store->room);

	free_renderer(logic_store->renderer);
	free_renderer(logic_store->hud_renderer);
	free_renderer(logic_store->ui_renderer);

	if (logic_store->ui) {
		free_ui_context(logic_store->ui);
	}

	free_world(logic_store->world);

	savegame_deinit();

	keymap_deinit();

	free_lsp_state(logic_store->lsp);
	fclose(logic_store->lsp_out);
}
