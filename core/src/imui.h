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
	ui_col_background2,
	ui_col_hovered,
	ui_col_text,
	ui_col_hot,
	ui_col_image,
	ui_col_image_hovered,
	ui_col_image_hot,
	ui_col_dock,
	ui_col_close,
	ui_col_close_hover,
	ui_col_close_active,
	ui_col_floating_btn,
	ui_col_floating_btn_hover,
	ui_col_floating_btn_active,
	ui_col_count,
};

struct float_rect {
	f32 x, y, w, h;
};

API struct ui_context* new_ui_context(struct shader shader, struct window* window, struct font* font);
API void free_ui_context(struct ui_context* ui);
API void ui_set_color(struct ui_context* ui, u32 id, struct color color);
API void ui_text_input_event(struct ui_context* ui, const char* text);
API void ui_begin_frame(struct ui_context* ui);
API void ui_end_frame(struct ui_context* ui);

API void ui_set_root_dockspace(struct ui_context* ui, struct float_rect rect);

API v2i ui_get_cursor_pos(struct ui_context* ui);
API void ui_set_cursor_pos(struct ui_context* ui, v2i pos);

API i32 ui_max_column_size(struct ui_context* ui);

API void ui_enable_docking(struct ui_context* ui, bool enable);

API bool ui_any_window_hovered(struct ui_context* ui);
API bool ui_any_item_hovered(struct ui_context* ui);
API bool ui_anything_hovered(struct ui_context* ui);
API bool ui_any_item_active(struct ui_context* ui);
API bool ui_any_item_hot(struct ui_context* ui);
API bool ui_any_windows_dragging(struct ui_context* ui);

API bool ui_begin_window(struct ui_context* ui, const char* name, v2i position, bool* open);
API void ui_end_window(struct ui_context* ui);

API void ui_text(struct ui_context* ui, const char* text);
API void ui_text_wrapped(struct ui_context* ui, const char* text);
API void ui_textf(struct ui_context* ui, const char* fmt, ...);
API bool ui_button(struct ui_context* ui, const char* text);
API bool ui_text_input(struct ui_context* ui, char* buf, u32 buf_size);
API bool ui_slider(struct ui_context* ui, f64* val, f64 min, f64 max);
API void ui_image(struct ui_context* ui, struct texture* texture, struct rect rect, v2i dimentions);
API bool ui_image_button(struct ui_context* ui, struct texture* texture, struct rect rect, v2i dimentions);
API bool ui_toggle(struct ui_context* ui, bool* value);
API void ui_rect(struct ui_context* ui, v2i dimentions, struct color color);
API void ui_loading_bar(struct ui_context* ui, const char* text, i32 percentage);
API bool ui_floating_button(struct ui_context* ui, const char* text);
API bool ui_floating_image(struct ui_context* ui, struct texture* texture, struct rect rect, v2i dimentions);

API void ui_columns(struct ui_context* ui, u32 count, i32 width);
API void ui_color(struct ui_context* ui, struct color color);
API void ui_reset_color(struct ui_context* ui);

API struct renderer* ui_get_renderer(struct ui_context* ui);

API struct font* ui_get_font(struct ui_context* ui);
API void ui_set_font(struct ui_context* ui, struct font* font);

API i32 ui_get_padding(struct ui_context* ui);
API void ui_set_padding(struct ui_context* ui, i32 padding);
API i32 ui_get_scrollbar_width(struct ui_context* ui);
API void ui_set_scrollbar_width(struct ui_context* ui, i32 width);

API bool ui_can_scroll(struct ui_context* ui);

API void ui_save_layout(struct ui_context* ui, const char* path);
API void ui_load_layout(struct ui_context* ui, const char* path);
