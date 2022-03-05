#include <stdio.h>
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

	main_window = new_window(make_v2i(1366, 768), "Pix-Maker", true);

	video_init();
	res_init();

	struct ui_context* ui = new_ui_context(load_shader("res/shaders/sprite.glsl"), main_window, load_font("res/DejaVuSans.ttf", 14.0f));
	set_window_uptr(main_window, ui);
	set_on_text_input(main_window, on_text_input);

	struct color* bitmap = core_calloc(1, 64 * 64 * sizeof(struct color));

	struct texture texture;
	init_texture_no_bmp(&texture, (u8*)bitmap, 64, 64, false);

	i32 texture_scale = 8;
	v2i mouse_pos = { 0 };
	v2i pan_offset = { 0 };
	v2i pan_drag_offset = { 0 };

	struct color colors[32];

	char r_buf[32] = "255";
	char g_buf[32] = "255";
	char b_buf[32] = "255";
	char a_buf[32] = "255";
	struct color cur_color = { 255, 255, 255, 255 };

	while (!window_should_close(main_window)) {
		update_events(main_window);

		video_clear();

		i32 win_w, win_h;
		query_window(main_window, &win_w, &win_h);

		i32 tex_w = texture.width * texture_scale;
		i32 tex_h = texture.height * texture_scale;

		struct textured_quad quad = {
			.position = {
				pan_offset.x + ((win_w / 2) - (tex_w / 2)),
				pan_offset.y + ((win_h / 2) - (tex_h / 2)) 
			},
			.dimentions = { tex_w, tex_h },
			.texture = &texture,
			.color = { 255, 255, 255, 255 },
			.rect = { 0, 0, texture.width, texture.height },
		};

		struct textured_quad background = {
			.position = quad.position,
			.dimentions = quad.dimentions,
			.color = make_color(0x1e1e1e, 255)
		};

		mouse_pos = get_mouse_position(main_window);

		mouse_pos = v2i_sub(mouse_pos, quad.position);
		mouse_pos = v2i_div(mouse_pos, make_v2i(texture_scale, texture_scale));

		if (mouse_pos.x >= 0 && mouse_pos.x < texture.width &&
			mouse_pos.y >= 0 && mouse_pos.y < texture.height) {

			if (key_pressed(main_window, KEY_CONTROL)) {	
				if (mouse_btn_pressed(main_window, MOUSE_BTN_LEFT)) {
					cur_color = bitmap[mouse_pos.x + mouse_pos.y * texture.width];

					sprintf(r_buf, "%d", cur_color.b);
					sprintf(g_buf, "%d", cur_color.g);
					sprintf(b_buf, "%d", cur_color.r);
					sprintf(a_buf, "%d", cur_color.a);
				}
			} else {
				if (mouse_btn_pressed(main_window, MOUSE_BTN_LEFT)) {
					bitmap[mouse_pos.x + mouse_pos.y * texture.width] = cur_color;
				}
				
				if (mouse_btn_pressed(main_window, MOUSE_BTN_RIGHT)) {
					bitmap[mouse_pos.x + mouse_pos.y * texture.width] = make_color(0x000000, 0);
				}
			}
		}

		if (mouse_btn_just_pressed(main_window, MOUSE_BTN_MIDDLE)) {
			pan_drag_offset = v2i_sub(get_mouse_position(main_window), pan_offset);
		}

		if (mouse_btn_pressed(main_window, MOUSE_BTN_MIDDLE)) {
			pan_offset = v2i_sub(get_mouse_position(main_window), pan_drag_offset);
		}

		texture_scale += get_scroll(main_window);
		if (texture_scale < 1) { texture_scale = 1; }

		ui_begin_frame(ui);

		struct textured_quad preview_quad = { 0 };
		if (ui_begin_window(ui, "Toolbox", make_v2i(0, 0))) {
			ui_textf(ui, "%d, %d",
				mouse_pos.x < 0 ? 0 : mouse_pos.x >= texture.width ? texture.width - 1 : mouse_pos.x,
				mouse_pos.y < 0 ? 0 : mouse_pos.y >= texture.height ? texture.height - 1 : mouse_pos.y);
		
			ui_columns(ui, 5, 55);
			ui_rect(ui, make_v2i(50, 50), (struct color) { cur_color.b, cur_color.g, cur_color.r, cur_color.a } );

			if (ui_text_input(ui, r_buf, sizeof(r_buf))) {
				cur_color.b = (u8)strtod(r_buf, null);
			}

			if (ui_text_input(ui, g_buf, sizeof(g_buf))) {
				cur_color.g = (u8)strtod(g_buf, null);
			}

			if (ui_text_input(ui, b_buf, sizeof(b_buf))) {
				cur_color.r = (u8)strtod(b_buf, null);
			}

			if (ui_text_input(ui, a_buf, sizeof(a_buf))) {
				cur_color.a = (u8)strtod(a_buf, null);
			}
			ui_columns(ui, 1, 100);

			ui_end_window(ui);
		}

		update_texture_no_bmp(&texture, (u8*)bitmap, texture.width, texture.height, false);

		renderer_push(ui_get_renderer(ui), &background);
		renderer_push(ui_get_renderer(ui), &quad);

		ui_end_frame(ui);

		swap_window(main_window);
	}

	deinit_texture(&texture);

	core_free(bitmap);

	free_ui_context(ui);

	res_deinit();

	free_window(main_window);
}
