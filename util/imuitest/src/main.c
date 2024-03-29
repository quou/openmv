#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "core.h"
#include "platform.h"
#include "res.h"
#include "imui.h"
#include "video.h"

static void on_text_input(struct window* window, const char* text, void* udata) {
	ui_text_input_event(udata, text);
}

i32 main() {
	srand((u32)time(null));

	init_time();

	main_window = new_window(make_v2i(1366, 768), "Immediate Mode UI", true);

	video_init();
	res_init();

	struct ui_context* ui = new_ui_context(load_shader("res/shaders/sprite.glsl"), main_window, load_font("res/DejaVuSans.ttf", 14.0f));
	set_window_uptr(main_window, ui);
	set_on_text_input(main_window, on_text_input);

	ui_enable_docking(ui, true);

	struct font* big_font = load_font("res/CourierPrime.ttf", 25.0f);

	struct texture* player_texture = load_texture("res/bmp/char.bmp", sprite_texture);
	struct texture* logo_texture = load_texture_no_pck("util/imuitest/res/logo.bmp", sprite_texture);

	u64 now = get_time(), last = now; 
	f64 timestep = 0.0;

	f32 float_val = 0.0f;

	char texts[256][256];
	u32 text_count = 0;
	char buffer[256] = "";

	bool toggle = false;

	bool a_open = false;
	bool b_open = false;

	f64 slider_val = 0.0;

	while (!window_should_close(main_window)) {
		update_events(main_window);

		video_clear();

		ui_begin_frame(ui);

		if (ui_begin_window(ui, "Test Window A", make_v2i(0, 0), &a_open)) {
			ui_textf(ui, "Memory Usage (KIB): %g", round(((f64)core_get_memory_usage() / 1024.0) * 100.0) / 100.0);
			ui_textf(ui, "FPS: %g", 1.0 / timestep);

			ui_columns(ui, 1, ui_max_column_size(ui));

			if (b_open) {
				if (ui_button(ui, "Close B window")) {
					b_open = false;
				}
			} else {
				if (ui_button(ui, "Open B window")) {
					b_open = true;
				}
			}

			ui_text_wrapped(ui,
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. "
				"This is some wrapped text. iaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

			ui_toggle(ui, &toggle);

			ui_columns(ui, 4, 80);
			ui_text(ui, "Input");
			if (ui_text_input(ui, buffer, sizeof(buffer)) || ui_button(ui, "Submit")) {
				strcpy(texts[text_count++], buffer);
				buffer[0] = '\0';
			}

			if (ui_button(ui, "Clear")) {
				text_count = 0;
			}

			ui_columns(ui, 1, 100);

			ui_color(ui, make_color(0xff0000, 255));
			ui_text(ui, "Red Text!");
			ui_color(ui, make_color(0x00ff00, 255));
			ui_text(ui, "Green Text!");
			ui_color(ui, make_color(0xffff00, 255));
			ui_text(ui, "Yellow Text!");
			ui_color(ui, make_color(0xff00ff, 255));
			ui_text(ui, "Pink Text!");
			ui_reset_color(ui);

			for (u32 i = 0; i < text_count; i++) {
				ui_text(ui, texts[i]);
			}

			ui_end_window(ui);
		}

		if (ui_begin_window(ui, "Test Window B", make_v2i(0, 300), &b_open)) {
			ui_columns(ui, 2, 100);
			if (ui_button(ui, "Save Layout")) {
				ui_save_layout(ui, "util/imuitest/lay.out");
			}

			if (ui_button(ui, "Load Layout")) {
				ui_load_layout(ui, "util/imuitest/lay.out");
			}

			ui_image(ui, player_texture, make_rect(0, 0, 16, 16), make_v2i(100, 100));
			ui_image_button(ui, player_texture, make_rect(0, 16, 16, 16), make_v2i(100, 100));
			ui_columns(ui, 3, 100);

			ui_text(ui, "Slider");
			ui_slider(ui, &slider_val, -64.0, 23.0);
			ui_textf(ui, "%g", slider_val);

			ui_columns(ui, 1, 100);

			struct font* ori_font = ui_get_font(ui);
			for (u32 i = 0; i < 25; i++) {
				if (i % 3 == 0) {
					ui_color(ui, make_color(0xff0000, 255));
				} else {
					ui_reset_color(ui);
				}

				if (i % 5 == 0) {
					ui_set_font(ui, big_font);
				} else {
					ui_set_font(ui, ori_font);
				}

				ui_button(ui, "Button");
			}
			ui_set_font(ui, ori_font);
			ui_reset_color(ui);

			ui_end_window(ui);
		}

		struct font* ori_font = ui_get_font(ui);
		ui_set_font(ui, big_font);

		if (ui_floating_button(ui, "Window B")) {
			b_open = true;
		}

		if (ui_floating_button(ui, "Window A")) {
			a_open = true;
		}

		ui_floating_image(ui, logo_texture, make_rect(0, 0, logo_texture->width, logo_texture->height),
			make_v2i(logo_texture->width, logo_texture->height));

		ui_set_font(ui, ori_font);

		ui_end_frame(ui);

		swap_window(main_window);

		now = get_time();
		timestep = (f64)(now - last) / (f64)get_frequency();
		last = now;

		if (timestep > 0.1) {
			timestep = 0.1;
		}
	}

	free_ui_context(ui);

	res_deinit();

	free_window(main_window);
}
