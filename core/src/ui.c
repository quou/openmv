#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "table.h"
#include "ui.h"

typedef bool (*input_filter_func)(char c);

bool text_input_filter(char c) {
	return c >= ' ' && c <= '~';
}

static bool mouse_over_rect(struct rect r) {
	v2i mouse_pos = get_mouse_position(main_window);

	return (mouse_pos.x > r.x &&
			mouse_pos.y > r.y &&
			mouse_pos.x < r.x + r.w &&
			mouse_pos.y < r.y + r.h);
}

static bool clicked() {
	return mouse_btn_just_released(main_window, MOUSE_BTN_LEFT);
}

static bool held() {
	return mouse_btn_pressed(main_window, MOUSE_BTN_LEFT);
}

enum {
	ui_el_text = 0,
	ui_el_button,
	ui_el_text_input,
	ui_el_image
};

enum {
	ui_col_window_background = 0,
	ui_col_background,
	ui_col_hovered,
	ui_col_text,
	ui_col_hot,
	ui_col_image,
	ui_col_image_hovered,
	ui_col_image_hot,
	ui_col_count
};

struct ui_element {
	v2i position;

	u32 type;

	union {
		struct {
			const char* text;
		} text;

		struct {
			const char* text;
			v2i dimentions;
		} button;

		struct {
			const char* label;
			char* buf;
			u32 buf_size;
			v2i input_dimentions;
			v2i input_pos;
		} text_input;

		struct {
			struct texture* texture;
			struct rect rect;
			v2i dimentions;
		} image;
	} as;
};

struct ui_window {
	v2i position;
	v2i dimentions;

	i32 z;

	struct ui_element* elements;
	u32 element_count;
	u32 element_capacity;

	const char* title;
};

static struct ui_element* ui_window_add_item(struct ui_window* w, struct ui_element el) {
	if (w->element_count >= w->element_capacity) {
		w->element_capacity = w->element_capacity < 8 ? 8 : w->element_capacity * 2;
		w->elements = realloc(w->elements, w->element_capacity * sizeof(struct ui_element));
	}

	w->elements[w->element_count] = el;

	return &w->elements[w->element_count++];
}

/* For persistent data. */
struct window_meta {
	v2i position;
	i32 scroll;
	i32 z;
};

struct ui_context {
	struct ui_window* windows;
	u32 window_count;
	u32 window_capacity;

	struct table* window_meta;

	struct ui_element* hovered;
	struct ui_element* hot;
	struct ui_element* active;

	char* input_buf;
	u32 input_buf_size;

	struct ui_window* current_window;
	struct ui_window* top_window;

	struct ui_window* dragging;
	v2i drag_offset;

	struct window* window;
	struct font* font;
	struct renderer* renderer;

	i32 input_scroll;

	i32 padding;
	i32 column_size;

	i32 window_max_height;

	v2i cursor_pos;

	input_filter_func input_filter;

	struct color style_colors[ui_col_count];
};

static void ui_draw_rect(struct ui_context* ui, struct rect rect, u32 col_idx) {
	struct textured_quad quad = {
		.texture = null,

		.position = make_v2i(rect.x, rect.y),
		.dimentions = make_v2i(rect.w, rect.h),
		.rect = { 0, 0, 0, 0 },
		.rotation = 0.0f,
		.origin = make_v2f(0.0f, 0.0f),
		.color = ui->style_colors[col_idx]
	};

	renderer_push(ui->renderer, &quad);
}

struct ui_context* new_ui_context(struct shader* shader, struct window* window, struct font* font) {
	struct ui_context* ui = calloc(1, sizeof(struct ui_context));

	ui->font = font;
	ui->window = window;
	ui->renderer = new_renderer(shader, make_v2i(800, 600));
	ui->renderer->clip_enable = true;

	ui->style_colors[ui_col_window_background] = make_color(0x1a1a1a, 250);
	ui->style_colors[ui_col_background] = make_color(0x212121, 255);
	ui->style_colors[ui_col_hovered] = make_color(0x242533, 255);
	ui->style_colors[ui_col_hot] = make_color(0x393d5b, 255);
	ui->style_colors[ui_col_text] = make_color(0xffffff, 255);
	ui->style_colors[ui_col_image_hovered] = make_color(0xaaaeeb, 255);
	ui->style_colors[ui_col_image_hot] = make_color(0x7686ff, 255);
	ui->style_colors[ui_col_image] = make_color(0xffffff, 255);

	ui->padding = 3;
	ui->column_size = 150;
	ui->window_max_height = 300;

	ui->window_meta = new_table(sizeof(struct window_meta));

	return ui;
}

void free_ui_context(struct ui_context* ui) {
	if (ui->windows) {
		for (u32 i = 0; i < ui->window_count; i++) {
			if (ui->windows[i].elements) {
				free(ui->windows[i].elements);
			}
		}

		free(ui->windows);
	}

	free_renderer(ui->renderer);

	free_table(ui->window_meta);

	free(ui);
}

void ui_text_input_event(struct ui_context* ui, const char* text) {
	if (!ui->input_buf) { return; }

	u32 len = strlen(text);
	u32 buf_len = strlen(ui->input_buf);

	if (buf_len + len < ui->input_buf_size) {
		for (u32 i = 0; i < len; i++) {
			if (ui->input_filter(text[i])) {
				strncat(ui->input_buf, text + i, 1);
			}
		}
	}
}

void ui_begin_frame(struct ui_context* ui) {
	ui->window_count = 0;

	i32 w, h;

	query_window(ui->window, &w, &h);

	ui->hovered = null;
	ui->hot = null;

	ui->window_count = 0;

	ui->renderer->camera = m4f_orth(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);
	ui->renderer->dimentions = make_v2i(w, h);
}

static i32 cmp_window_z(const struct ui_window* a, const struct ui_window* b) {
	return a->z < b->z;
}

void ui_end_frame(struct ui_context* ui) {
	if (ui->input_buf && key_just_released(main_window, KEY_BACKSPACE) && strlen(ui->input_buf) > 0) {
		ui->input_buf[strlen(ui->input_buf) - 1] = '\0';
	}

	if (!ui->hovered && clicked()) {
		ui->input_buf = null;
		ui->active = null;
	}

	if (mouse_btn_just_released(main_window, MOUSE_BTN_LEFT)) {
		ui->dragging = null;
	}

	ui->top_window = ui->windows;
	for (u32 i = 0; i < ui->window_count; i++) {
		if (ui->windows[i].z == 0) {
			ui->top_window = ui->windows + i;
			break;
		}
	}

	if (ui->top_window && !mouse_over_rect(make_rect(
			ui->top_window->position.x, ui->top_window->position.y,
			ui->top_window->dimentions.x, ui->top_window->dimentions.y))) {
		for (u32 i = 0; i < ui->window_count; i++) {
			struct ui_window* window = ui->windows + i;

			struct window_meta* meta = table_get(ui->window_meta, window->title);

			if (mouse_over_rect(make_rect(window->position.x, window->position.y,
						window->dimentions.x, window->dimentions.y)) &&
					mouse_btn_pressed(main_window, MOUSE_BTN_LEFT)) {
				meta->z = 0;

				for (u32 ii = 0; ii < ui->window_count; ii++) {
					struct ui_window* w = ui->windows + ii;
					if (w == window) { continue; }

					struct window_meta* m = table_get(ui->window_meta, w->title);
					if (m) {
						m->z++;
					}
				}

				break;
			}
		}
	}

	struct ui_window* sorted_windows = malloc(ui->window_count * sizeof(struct ui_window));
	memcpy(sorted_windows, ui->windows, ui->window_count * sizeof(struct ui_window));

	qsort(sorted_windows, ui->window_count, sizeof(struct ui_window),
		(int(*)(const void*, const void*))cmp_window_z);

	for (u32 i = 0; i < ui->window_count; i++) {
		struct ui_window* window = sorted_windows + i;

		struct rect window_rect = make_rect(
			window->position.x, window->position.y,
			window->dimentions.x, window->dimentions.y);

		renderer_clip(ui->renderer, window_rect);

		ui_draw_rect(ui, window_rect,
			ui_col_window_background);

		i32 title_w = text_width(ui->font, window->title);
		i32 title_h = text_height(ui->font) + ui->padding * 2;
		render_text(ui->renderer, ui->font, window->title,
			window->position.x + ((window->dimentions.x / 2) - (title_w / 2)),
			window->position.y + ui->padding, ui->style_colors[ui_col_text]);

		struct rect clip_rect = make_rect(window_rect.x, window_rect.y + title_h,
			window_rect.w, window_rect.h - title_h);

		renderer_clip(ui->renderer, clip_rect);

		for (u32 i = 0; i < window->element_count; i++) {
			struct ui_element* el = window->elements + i;

			if (el->position.y > window->position.y + window->dimentions.y) {
				break;
			}

			switch (el->type) {
				case ui_el_text:
					render_text(ui->renderer, ui->font, el->as.text.text, el->position.x, el->position.y, ui->style_colors[ui_col_text]);
					break;
				case ui_el_button: {
					u32 c = ui_col_background;
					if (ui->hovered == el) {
						c = ui_col_hovered;
					}

					if (ui->hot == el) {
						c = ui_col_hot;
					}

					ui_draw_rect(ui, make_rect(
						el->position.x, el->position.y,
						el->as.button.dimentions.x, el->as.button.dimentions.y
					), c);
					render_text(ui->renderer, ui->font, el->as.button.text,
						el->position.x + ui->padding,
						el->position.y + ui->padding,
						ui->style_colors[ui_col_text]);
					break;
				}
				case ui_el_text_input: {
					u32 c = ui_col_background;
					if (ui->hovered == el) {
						c = ui_col_hovered;
					}

					if (ui->hot == el) {
						c = ui_col_hot;
					}

					ui_draw_rect(ui, make_rect(
						el->as.text_input.input_pos.x, el->as.text_input.input_pos.y,
						el->as.text_input.input_dimentions.x, el->as.text_input.input_dimentions.y
					), c);

					i32 w = 0, h = 0;
					if (el->as.text_input.buf) {
						w = text_width(ui->font, el->as.text_input.buf);
						h = text_height(ui->font);
					}

					bool want_reset = false;
					i32 input_scroll = 0;
					if (w > el->as.text_input.input_dimentions.x - ui->padding - h) {
						renderer_clip(ui->renderer, make_rect(
							el->as.text_input.input_pos.x + ui->padding,
							el->as.text_input.input_pos.y + ui->padding > clip_rect.y ? 
								el->as.text_input.input_pos.y + ui->padding : clip_rect.y,
							el->as.text_input.input_dimentions.x - ui->padding * 2,
							el->as.text_input.input_dimentions.y - ui->padding * 2));
						want_reset = true;

						if (ui->active == el) {
							input_scroll = el->as.text_input.input_dimentions.x - w - ui->padding - h;
						}
					}

					render_text(ui->renderer, ui->font, el->as.text_input.buf,
						(el->as.text_input.input_pos.x + ui->padding) + input_scroll,
						el->as.text_input.input_pos.y + ui->padding,
						ui->style_colors[ui_col_text]);	

					if (want_reset) {
						renderer_clip(ui->renderer, clip_rect);
					}

					render_text(ui->renderer, ui->font, el->as.text_input.label,
						el->position.x + ui->padding,
						el->position.y + ui->padding,
						ui->style_colors[ui_col_text]);	
					if (ui->active == el) {
						ui_draw_rect(ui, make_rect(
							el->as.text_input.input_pos.x + w + ui->padding + input_scroll,
							el->as.text_input.input_pos.y + ui->padding,
							1, h
						), ui_col_text);
					}	

					break;
				}
				case ui_el_image: {
					u32 c = ui_col_image;
					if (ui->hovered == el) {
						c = ui_col_image_hovered;
					}

					if (ui->hot == el) {
						c = ui_col_image_hot;
					}

					struct textured_quad quad = {
						.texture = el->as.image.texture,
						.position = el->position,
						.dimentions = el->as.image.dimentions,
						.rect = el->as.image.rect,
						.color = ui->style_colors[c],
					};

					renderer_push(ui->renderer, &quad);
					break;
				}
			}
		}
	}

	free(sorted_windows);

	renderer_flush(ui->renderer);
}

bool ui_begin_window(struct ui_context* ui, const char* name, v2i position) {
	if (ui->window_count >= ui->window_capacity) {
		ui->window_capacity = ui->window_capacity < 8 ? 8 : ui->window_capacity * 2;
		ui->windows = realloc(ui->windows, ui->window_capacity * sizeof(struct ui_window));
		
		for (u32 i = ui->window_count; i < ui->window_capacity; i++) {
			ui->windows[i] = (struct ui_window) { 0 };
		}
	}	

	struct ui_window* window = &ui->windows[ui->window_count++];

	ui->current_window = window;

	window->element_count = 0;
	window->title = name;

	window->position = position;
	struct window_meta* meta = table_get(ui->window_meta, name);
	if (!meta) {
		struct window_meta new_meta = { position, 0, 0 };
		table_set(ui->window_meta, name, &new_meta);

		meta = table_get(ui->window_meta, name);
	} else {
		window->position = meta->position;
		window->z = meta->z;
	}

	window->dimentions = make_v2i(300, ui->window_max_height);

	ui->cursor_pos = make_v2i(
		window->position.x + ui->padding,
		window->position.y + meta->scroll + text_height(ui->font) + ui->padding * 2);

	return true;
}

void ui_end_window(struct ui_context* ui) {
	struct ui_window* window = ui->current_window;

	struct window_meta* meta = table_get(ui->window_meta, window->title);
	if (meta) {
		struct rect window_rect = make_rect(
			window->position.x, window->position.y,
			window->dimentions.x, window->dimentions.y);	

		if (mouse_over_rect(window_rect)) {
			if (
				!ui->hovered && 
				mouse_btn_just_pressed(main_window, MOUSE_BTN_LEFT) &&
				ui->top_window == window) {
				ui->dragging = window;
				ui->drag_offset = v2i_sub(get_mouse_position(main_window), window->position);
			}

			meta->scroll += get_scroll(main_window) * (text_height(ui->font) + ui->padding);

			if (meta->scroll > 0) {
				meta->scroll = 0;
			}	
		}

		if (window == ui->dragging) {
			meta->position = v2i_sub(get_mouse_position(main_window), ui->drag_offset);
		}
	}

	ui->current_window = null;
}

void ui_text(struct ui_context* ui, const char* text) {
	ui_window_add_item(ui->current_window, (struct ui_element) {
		.type = ui_el_text,
		.position = ui->cursor_pos,
		.as.text = {
			.text = text
		}
	});

	ui->cursor_pos.y += text_height(ui->font) + ui->padding;
}

bool ui_button(struct ui_context* ui, const char* text) {
	struct rect r = make_rect(ui->cursor_pos.x, ui->cursor_pos.y,
		text_width(ui->font, text) + ui->padding * 2,
		text_height(ui->font) + ui->padding * 2);

	struct ui_element* e = ui_window_add_item(ui->current_window, (struct ui_element) {
		.type = ui_el_button,
		.position = ui->cursor_pos,
		.as.button = {
			.text = text,
			.dimentions = make_v2i(r.w, r.h)
		}
	});

	ui->cursor_pos.y += text_height(ui->font) + ui->padding * 3;

	if (ui->top_window == ui->current_window &&
			e->position.y < ui->current_window->position.y + ui->current_window->dimentions.y) {
		if (mouse_over_rect(r)) {
			ui->hovered = e;

			if (held()) {
				ui->hot = e;
			}

			if (clicked()) {
				return true;
			}
		}
	}

	return false;
}

void ui_text_input(struct ui_context* ui, const char* label, char* buf, u32 buf_size) {
	struct rect r = make_rect(
		ui->cursor_pos.x + ui->column_size,
		ui->cursor_pos.y + ui->padding,
		ui->current_window->dimentions.x - ui->column_size - ui->padding * 2,
		text_height(ui->font) + ui->padding * 2);

	struct ui_element* e = ui_window_add_item(ui->current_window, (struct ui_element) {
		.type = ui_el_text_input,
		.position = ui->cursor_pos,
		.as.text_input = {
			.label = label,
			.buf = buf,
			.buf_size = buf_size,
			.input_dimentions = make_v2i(r.w, r.h),
			.input_pos = make_v2i(r.x, r.y)
		}
	});

	ui->cursor_pos.y += text_height(ui->font) + ui->padding * 4;

	ui->input_filter = text_input_filter;

	if (ui->top_window == ui->current_window &&
			e->position.y < ui->current_window->position.y + ui->current_window->dimentions.y) {
		if (mouse_over_rect(r)) {
			ui->hovered = e;

			if (held()) {
				ui->hot = e;
			}

			if (clicked()) {
				ui->active = e;
				ui->input_buf = buf;
				ui->input_buf_size = buf_size;
			}
		}
	}
}

bool ui_image(struct ui_context* ui, struct texture* texture, struct rect rect) {
	struct rect r = make_rect(ui->cursor_pos.x, ui->cursor_pos.y, 100, 100);

	struct ui_element* e = ui_window_add_item(ui->current_window, (struct ui_element) {
		.type = ui_el_image,
		.position = ui->cursor_pos,
		.as.image = {
			.texture = texture,
			.dimentions = make_v2i(100, 100),
			.rect = rect
		}
	});

	ui->cursor_pos.y += e->as.image.dimentions.y + ui->padding;

	if (ui->top_window == ui->current_window &&
			e->position.y < ui->current_window->position.y + ui->current_window->dimentions.y) {
		if (mouse_over_rect(r)) {
			ui->hovered = e;

			if (held()) {
				ui->hot = e;
			}

			if (clicked()) {
				return true;
			}
		}
	}

	return false;

}
