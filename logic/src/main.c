#include <stdio.h>

#include "consts.h"
#include "core.h"
#include "coresys.h"
#include "entity.h"
#include "keymap.h"
#include "logic_store.h"
#include "player.h"
#include "res.h"
#include "sprites.h"
#include "ui.h"

struct logic_store* logic_store;

API u64 CALL get_storage_size() {
	return sizeof(struct logic_store);
}

API void CALL on_reload(void* instance) {
	logic_store = instance;
}

static void on_text_input(struct window* window, const char* text, void* udata) {
	struct ui_context* ui = udata;

	ui_text_input_event(ui, text);
}

API void CALL on_init() {
	keymap_init();
	default_keymap();
	load_keymap();

	struct shader* sprite_shader = load_shader("res/shaders/sprite.glsl");
	logic_store->renderer = new_renderer(sprite_shader, make_v2i(1366, 768));

	logic_store->ui = new_ui_context(sprite_shader, main_window, load_font("res/DejaVuSans.ttf", 14.0f));

	set_window_uptr(main_window, logic_store->ui);
	set_on_text_input(main_window, on_text_input);

	struct world* world = new_world();
	logic_store->world = world;

	preload_sprites();

	entity player = new_player_entity(world);

	logic_store->room = new_room(16, 16);
	logic_store->selected_tile = tile_empty;
	logic_store->room->tile_set = get_tile_set(tilesetid_blue);
	logic_store->selected_layer = new_tile_layer(logic_store->room);

	set_tile(logic_store->room, logic_store->selected_layer, 4, 4, tsbluetile_centre);
	set_tile(logic_store->room, logic_store->selected_layer, 3, 3, tsbluetile_corner_top_left);
	set_tile(logic_store->room, logic_store->selected_layer, 4, 3, tsbluetile_top);
	set_tile(logic_store->room, logic_store->selected_layer, 5, 3, tsbluetile_corner_top_right);
	set_tile(logic_store->room, logic_store->selected_layer, 3, 4, tsbluetile_wall_left);
	set_tile(logic_store->room, logic_store->selected_layer, 5, 4, tsbluetile_wall_right);
	set_tile(logic_store->room, logic_store->selected_layer, 3, 5, tsbluetile_corner_bot_left);
	set_tile(logic_store->room, logic_store->selected_layer, 4, 5, tsbluetile_bot);
	set_tile(logic_store->room, logic_store->selected_layer, 5, 5, tsbluetile_corner_bot_right);
}

API void CALL on_update(double ts) {
	struct ui_context* ui = logic_store->ui;
	struct renderer* renderer = logic_store->renderer;
	struct world* world = logic_store->world;

	logic_store->fps_timer += ts;
	if (logic_store->fps_timer > 1.0) {
		sprintf(logic_store->fps_buf, "FPS: %g", 1.0 / ts);
		logic_store->fps_timer = 0.0;
	}

	draw_room(logic_store->room, renderer);

	player_system(world, ts);

	render_system(world, renderer, ts);
	renderer_flush(renderer);

	ui_begin_frame(ui);
	
	if (ui_begin_window(ui, "Tilemap", make_v2i(0, 0))) {
		struct room* room = logic_store->room;

		if (ui_button(ui, "New Layer")) {
			new_tile_layer(logic_store->room);
		}

		for (u32 i = 0; i < room->layer_count; i++) {
			char name[64];
			sprintf(name, "Layer %d", i);
			if (ui_button(ui, name)) {
				logic_store->selected_layer = &room->layers[i];
			}
		}

		if (ui_button(ui, "Eraser")) {
			logic_store->selected_tile = tile_empty;
		}

		for (u32 i = tile_empty + 1; i < sizeof(room->tile_set.tiles) / sizeof(*room->tile_set.tiles); i++) {
			if (room->tile_set.tiles[i].w == 0 && room->tile_set.tiles[i].h == 0) {
				break;
			}

			if (ui_image(ui, room->tile_set.texture, room->tile_set.tiles[i])) {
				logic_store->selected_tile = i;
			}
		}

		struct tile_layer* layer = logic_store->selected_layer;

		if (mouse_btn_pressed(main_window, MOUSE_BTN_LEFT)) {
			v2i mp = get_mouse_position(main_window);
			v2i mprs = v2i_div(make_v2i((i32)mp.x, (i32)mp.y), make_v2i(
				sprite_scale * room->tile_set.tile_size,
				sprite_scale * room->tile_set.tile_size
			));
			
			if (mprs.x < room->width && mprs.y < room->height) {
				set_tile(logic_store->room, logic_store->selected_layer, mprs.x, mprs.y, logic_store->selected_tile);
			}
		}

		ui_end_window(ui);
	}

	ui_end_frame(ui);
}

API void CALL on_deinit() {
	free_room(logic_store->room);

	free_renderer(logic_store->renderer);
	free_ui_context(logic_store->ui);
	free_world(logic_store->world);

	keymap_deinit();
}
