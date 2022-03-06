#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "tinyfiledialogs.h"

#include "core.h"
#include "platform.h"
#include "res.h"
#include "imui.h"
#include "video.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

static void on_text_input(struct window* window, const char* text, void* udata) {
	ui_text_input_event(udata, text);
}

#pragma pack(push, 1)
struct bitmap {
	u8 signature[2];
	u32 file_size;
	u32 reserved;
	u32 pixel_offset;
	u32 header_size;
	u32 width, height;
	u16 planes;
	u16 bits_per_pixel;
	u32 compression;
	u32 image_size;
	u32 y_pixel_per_metre;
	u32 x_pixel_per_metre;
	u32 color_pallatte;
	u32 most_imp_color;
	u32 red_mask;
	u32 green_mask;
	u32 blue_mask;
	u32 alpha_mask;
};
#pragma pack(pop)

static char* print_error(char** error, const char* fmt, ...) {
	if (*error) { core_free(*error); }

	va_list list;
	va_start(list, fmt);

	u32 len = vsnprintf(null, 0, fmt, list);

	va_end(list);

	va_start(list, fmt);
	char* str = core_alloc(len + 1);
	vsprintf(str, fmt, list);
	va_end(list);

	*error = str;
	
	return str;
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

	u32 bitmap_width = 64, bitmap_height = 64;
	struct color* bitmap = core_calloc(1, bitmap_width * bitmap_height * sizeof(struct color));
	struct color* clipboard = null;
	struct rect clipboard_rect = { 0, 0, 0, 0 };

	struct texture texture;
	init_texture_no_bmp(&texture, (u8*)bitmap, bitmap_width, bitmap_height, false);

	i32 texture_scale = 8;
	v2i mouse_pos = { 0 };
	v2i pan_offset = { 0 };
	v2i pan_drag_offset = { 0 };

	bool selecting = false;
	v2i selection_start;
	v2i selection_end;

	struct color colors[32];

	char r_buf[32] = "255";
	char g_buf[32] = "255";
	char b_buf[32] = "255";
	char a_buf[32] = "255";
	struct color cur_color = { 255, 255, 255, 255 };

	char* cur_save_path = null;
	char* error = null;

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

		struct textured_quad selection = {
			.position = {
				.x = quad.position.x + selection_start.x * texture_scale,
				.y = quad.position.y + selection_start.y * texture_scale,
			},
			.dimentions = {
				.x = (quad.position.x + selection_end.x * texture_scale) - (quad.position.x + selection_start.x * texture_scale),
				.y = (quad.position.y + selection_end.y * texture_scale) - (quad.position.y + selection_start.y * texture_scale)
			},
			.color = make_color(0xaaaaaa, 100)
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
			} else if (key_pressed(main_window, KEY_SHIFT)) {
				if (mouse_btn_just_pressed(main_window, MOUSE_BTN_LEFT)) {
					selecting = !selecting;
					selection_start = mouse_pos;
				}

				if (selecting && mouse_btn_pressed(main_window, MOUSE_BTN_LEFT)) {	
					selection_end = mouse_pos;
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

		if (selecting) {
			if (key_just_pressed(main_window, KEY_Y)) {
				v2i s = make_v2i(MIN(selection_start.x, selection_end.x), MIN(selection_start.y, selection_end.y));
				v2i e = make_v2i(MAX(selection_start.x, selection_end.x), MAX(selection_start.y, selection_end.y));

				clipboard_rect.x = s.x;
				clipboard_rect.y = s.y;
				clipboard_rect.w = e.x - s.x;
				clipboard_rect.h = e.y - s.y;

				clipboard = core_realloc(clipboard, bitmap_width * bitmap_height * sizeof(struct color));

				for (i32 y = clipboard_rect.y; y < e.y; y++) {
					for (i32 x = clipboard_rect.x; x < e.x; x++) {
						clipboard[x + y * clipboard_rect.w] = bitmap[x + y * bitmap_width];
					}
				}
			}
		}

		if (clipboard && key_just_pressed(main_window, KEY_P)) {
			v2i p_offset = { 0 };

			if (selecting) {
				p_offset = make_v2i(MIN(selection_start.x, selection_end.x), MIN(selection_start.y, selection_end.y));
				selecting = false;
			}

			for (i32 y = clipboard_rect.y, yy = p_offset.y; y < clipboard_rect.y + clipboard_rect.h && yy < bitmap_height; y++, yy++) {
				for (i32 x = clipboard_rect.x, xx = p_offset.x; x < clipboard_rect.x + clipboard_rect.w && xx < bitmap_width; x++, xx++) {
					bitmap[xx + yy * bitmap_width] = clipboard[x + y * clipboard_rect.w];
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
			if (error) {
				ui_color(ui, make_color(0xff0000, 255));
				ui_text(ui, error);
				ui_reset_color(ui);
			}

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

			ui_columns(ui, 2, 100);

			if (ui_button(ui, "Write") || key_just_pressed(main_window, KEY_W)) {
				if (!cur_save_path || key_pressed(main_window, KEY_SHIFT)) {
					const char* filters[] = {
						"*.bmp"
					};	

					const char* path = tinyfd_saveFileDialog("Write",
						cur_save_path ? cur_save_path : "", 1, filters, "Bitmap Files");

					if (path) {
						if (cur_save_path) {
							core_free(cur_save_path);
						}

						cur_save_path = copy_string(path);
					}
				}
				if (cur_save_path) {
					FILE* file = fopen(cur_save_path, "wb");
					if (file) {
						struct bitmap header = {
							.signature = "BM",
							.file_size = sizeof(struct bitmap) + texture.width * texture.height * sizeof(struct color),
							.pixel_offset = sizeof(struct bitmap),
							.header_size = 56,
							.width = texture.width,
							.height = texture.height,
							.planes = 1,
							.bits_per_pixel = 32,
							.image_size = texture.width * texture.height * sizeof(struct color),
							.y_pixel_per_metre = 0x130B,
							.x_pixel_per_metre = 0x130B,
							.most_imp_color = 0,
							.red_mask = 0x00ff0000,
							.green_mask = 0x0000ff00,
							.blue_mask = 0x000000ff,
							.alpha_mask = 0xff000000
						};

						struct color* dst = core_alloc(header.image_size);

						for (u32 y = 0; y < header.height; y++) {
							for (u32 x = 0; x < header.width; x++) {
								dst[(header.height - y - 1) * header.width + x] = bitmap[y * header.width + x];
							}
						}

						fwrite(&header, sizeof(header), 1, file);
						fwrite(dst, header.image_size, 1, file);

						fclose(file);

						core_free(dst);
					} else {
						print_error(&error, "Failed to fopen file: `%s'.", cur_save_path);
					}
				}
			}

			if (ui_button(ui, "Load") || key_just_pressed(main_window, KEY_Q)) {
				const char* filters[] = {
					"*.bmp"
				};

				const char* path = tinyfd_openFileDialog("Load", cur_save_path ? cur_save_path : "", 1, filters, "Bitmap Files", false);

				if (path) {
					if (cur_save_path) {
						core_free(cur_save_path);
						cur_save_path = copy_string(path);
					}

					FILE* file = fopen(path, "rb");

					if (file) {
						u8* buf;
						u64 size;

						read_raw_no_pck(path, &buf, &size, false);

						if (size < sizeof(struct bitmap)) {
							print_error(&error, "Not a valid Bitmap image.");
							goto end;
						}

						struct bitmap* header = (struct bitmap*)buf;

						if (header->signature[0] != 'B' || header->signature[1] != 'M') {
							print_error(&error, "Not a valid Bitmap image.");
							goto end;
						}

						core_free(bitmap);
						core_free(clipboard);

						struct color* src = (struct color*)(buf + header->pixel_offset);
						struct color* dst = core_alloc(header->image_size);

						for (u32 y = 0; y < header->height; y++) {
							for (u32 x = 0; x < header->width; x++) {
								dst[(header->height - y - 1) * header->width + x] = src[y * header->width + x];
							}
						}

						bitmap_width = header->width;
						bitmap_height = header->height;
						bitmap = dst;
	end:
						core_free(buf);
					} else {
						print_error(&error, "Failed to fopen file: `%s'.", path);
					}
				}
			}
			
			ui_columns(ui, 1, 100);

			ui_end_window(ui);
		}

		update_texture_no_bmp(&texture, (u8*)bitmap, bitmap_width, bitmap_height, false);

		renderer_push(ui_get_renderer(ui), &background);
		renderer_push(ui_get_renderer(ui), &quad);

		if (selecting) {
			renderer_push(ui_get_renderer(ui), &selection);
		}

		ui_end_frame(ui);

		swap_window(main_window);
	}

	deinit_texture(&texture);

	if (clipboard) {
		core_free(clipboard);
	}

	core_free(bitmap);

	free_ui_context(ui);

	res_deinit();

	free_window(main_window);
}
