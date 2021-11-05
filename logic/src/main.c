#include <stdio.h>

#include "core.h"
#include "coresys.h"
#include "entity.h"
#include "res.h"
#include "ui.h"

#include "player.h"

struct logic_store {
	double fps_timer;
	char fps_buf[256];

	struct renderer* renderer;
	struct ui_context* ui;

	struct world* world;
};

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
	struct shader* sprite_shader = load_shader("res/shaders/sprite.glsl");
	logic_store->renderer = new_renderer(sprite_shader, make_v2i(1366, 768));

	logic_store->ui = new_ui_context(sprite_shader, main_window, load_font("res/DejaVuSans.ttf", 14.0f));

	set_window_uptr(main_window, logic_store->ui);
	set_on_text_input(main_window, on_text_input);

	struct world* world = new_world();
	logic_store->world = world;

	entity player = new_player_entity(world);
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

	player_system(world, ts);

	render_system(world, renderer, ts);
	renderer_flush(renderer);

	ui_begin_frame(ui);
	
	if (ui_begin_window(ui, "Test Window", make_v2i(100, 30))) {
		ui_text(ui, logic_store->fps_buf);

		static char buf[256] = "Hello, world!";
		ui_text_input(ui, "Test Input", buf, 256);

		static char buf2[256] = "Hello, another input";
		ui_text_input(ui, "Test Input 2", buf2, 256);

		for (u32 i = 0; i < 25; i++) {
			if (ui_button(ui, "Button")) {
				printf("hi\n");
			}
		}

		ui_end_window(ui);
	}

	if (ui_begin_window(ui, "Another Window", make_v2i(400, 30))) {
		ui_text(ui, "Hello, world!");

		ui_end_window(ui);
	}

	ui_end_frame(ui);
}

API void CALL on_deinit() {
	free_renderer(logic_store->renderer);
	free_ui_context(logic_store->ui);
	free_world(logic_store->world);
}
