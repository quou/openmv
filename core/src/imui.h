#pragma once

#include "common.h"
#include "maths.h"
#include "platform.h"
#include "video.h"

/* Immediate mode, state based GUI. */

struct ui_context;

enum {
	ui_col_window_background = 0,
	ui_col_window_border,
	ui_col_background,
	ui_col_hovered,
	ui_col_text,
	ui_col_hot,
	ui_col_image,
	ui_col_image_hovered,
	ui_col_image_hot,
	ui_col_dock,
	ui_col_count
};

API struct ui_context* new_ui_context(struct shader shader, struct window* window, struct font* font);
API void free_ui_context(struct ui_context* ui);
API void ui_set_color(struct ui_context* ui, u32 id, struct color color);
API void ui_text_input_event(struct ui_context* ui, const char* text);
API void ui_begin_frame(struct ui_context* ui);
API void ui_end_frame(struct ui_context* ui);

API bool ui_begin_window(struct ui_context* ui, const char* name, v2i position);
API void ui_end_window(struct ui_context* ui);

API void ui_text(struct ui_context* ui, const char* text);
API bool ui_button(struct ui_context* ui, const char* text);
API bool ui_text_input(struct ui_context* ui, char* buf, u32 buf_size);
API void ui_image(struct ui_context* ui, struct texture* texture, struct rect rect);
API bool ui_image_button(struct ui_context* ui, struct texture* texture, struct rect rect);
API void ui_loading_bar(struct ui_context* ui, const char* text, i32 percentage);

API void ui_columns(struct ui_context* ui, u32 count, i32 width);
API void ui_color(struct ui_context* ui, struct color color);
API void ui_reset_color(struct ui_context* ui);

API struct renderer* ui_get_renderer(struct ui_context* ui);

API struct font* ui_get_font(struct ui_context* ui);
API void ui_set_font(struct ui_context* ui, struct font* font);

API void ui_save_layout(struct ui_context* ui, const char* path);
API void ui_load_layout(struct ui_context* ui, const char* path);
