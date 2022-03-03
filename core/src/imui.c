#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "table.h"
#include "imui.h"

#define max_dockspaces 32

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

static bool rect_over_rect(struct rect a, struct rect b) {
	return
		a.x + a.w > b.x &&
		a.y + a.h > b.y &&
		a.x < b.x + b.w &&
		a.y < b.y + b.h;
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

struct ui_element {
	v2i position;

	u32 type;

	struct font* font;
	struct color color;
	bool override_color;

	union {
		struct {
			char* text;
		} text;

		struct {
			char* text;
			v2i dimentions;
		} button;

		struct {
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

	struct font* font;

	char* title;
};

enum {
	ui_dock_dir_left = 0,
	ui_dock_dir_right,
	ui_dock_dir_up,
	ui_dock_dir_down
};

struct float_rect {
	f32 x, y, w, h;
};

static inline struct float_rect make_float_rect(f32 x, f32 y, f32 w, f32 h) {
	return (struct float_rect) { x, y, w, h };
}

struct ui_dockspace {
	struct ui_dockspace* parent;
	u32 parent_dir;

	bool occupied;

	struct float_rect rect;
};

static v2i get_dockspace_position(struct ui_dockspace* dock) {
	i32 w, h;
	query_window(main_window, &w, &h);

	return make_v2i((i32)(dock->rect.x * (f32)w), (i32)(dock->rect.y * (f32)h));
}

static v2i get_dockspace_dimentions(struct ui_dockspace* dock) {
	i32 w, h;
	query_window(main_window, &w, &h);

	return make_v2i((i32)(dock->rect.w * (f32)w), (i32)(dock->rect.h * (f32)h));
}

static struct rect get_dock_rect_screen(struct float_rect rect) {
	i32 w, h;
	query_window(main_window, &w, &h);

	return (struct rect) {
		(i32)(rect.x * (f32)w),
		(i32)(rect.y * (f32)h),
		(i32)(rect.w * (f32)w),
		(i32)(rect.h * (f32)h),
	};
}

/* For persistent data. */
struct window_meta {
	v2i position;
	v2i dimentions;
	i32 scroll;
	i32 z;
	struct ui_dockspace* dock;
};

struct ui_context {
	struct ui_window* windows;
	u32 window_count;
	u32 window_capacity;

	struct ui_dockspace dockspaces[max_dockspaces];
	u32 dockspace_count;
	struct ui_dockspace* current_dockspace;

	struct table* window_meta;
	struct table* strings;

	struct ui_element* hovered;
	struct ui_element* hot;
	struct ui_element* active;

	char* input_buf;
	u32 input_buf_size;
	u32 input_cursor;

	char* loading;
	i32 loading_p;
	struct font* loading_f;

	struct ui_window* current_window;
	struct ui_window* top_window;

	struct ui_window* dragging;
	v2i drag_offset;

	struct ui_window* resizing;

	struct window* window;
	struct font* font;
	struct renderer* renderer;

	v2i drag_start;

	i32 input_scroll;

	u32 columns;
	u32 item;

	i32 padding;
	i32 column_size;

	i32 window_max_height;

	v2i cursor_pos;

	input_filter_func input_filter;

	struct color style_colors[ui_col_count];

	struct color color_override;
	bool override_color;
};

struct text_entry {
	i32 life;
	char* ptr;
};

static char* ui_copy_string(struct ui_context* ui, const char* text) {
	struct text_entry* entry = table_get(ui->strings, text);

	if (!entry) {
		struct text_entry new_entry = {
			.life = 1,
			.ptr = copy_string(text)
		};

		entry = table_set(ui->strings, text, &new_entry);
	} else {
		entry->life++;
	}

	return entry->ptr;
}

static struct ui_element* ui_window_add_item(struct ui_context* ui, struct ui_window* w, struct ui_element el) {
	if (w->element_count >= w->element_capacity) {
		w->element_capacity = w->element_capacity < 8 ? 8 : w->element_capacity * 2;
		w->elements = core_realloc(w->elements, w->element_capacity * sizeof(struct ui_element));
	}

	w->elements[w->element_count] = el;

	struct ui_element* e = &w->elements[w->element_count++];

	e->font = ui->font;
	e->override_color = ui->override_color;
	if (e->override_color) {
		e->color = ui->color_override;
	} else {
		e->color = ui->style_colors[ui_col_text];
	}

	return e;
}

static u32 ui_get_hovered_windows(struct ui_context* ui, struct ui_window** windows, u32 max) {
	u32 count = 0;

	for (u32 i = 0; i < ui->window_count; i++) {
		struct ui_window* window = ui->windows + i;

		struct rect window_rect = make_rect(
			window->position.x, window->position.y,
			window->dimentions.x, window->dimentions.y);

		if (mouse_over_rect(window_rect)) {
			windows[count++] = window;
		}

		if (count >= max) {
			return count;
		}
	}

	return count;
}

static void ui_draw_rect(struct ui_context* ui, struct rect rect, u32 col_idx) {
	struct textured_quad quad = {
		.texture = null,

		.position = make_v2i(rect.x, rect.y),
		.dimentions = make_v2i(rect.w, rect.h),
		.rect = { 0, 0, 0, 0 },
		.origin = make_v2f(0.0f, 0.0f),
		.color = ui->style_colors[col_idx]
	};

	renderer_push(ui->renderer, &quad);
}

struct ui_context* new_ui_context(struct shader shader, struct window* window, struct font* font) {
	struct ui_context* ui = core_calloc(1, sizeof(struct ui_context));

	ui->font = font;
	ui->window = window;
	ui->renderer = new_renderer(shader, make_v2i(800, 600));
	ui->renderer->clip_enable = true;

	ui->style_colors[ui_col_window_background] = make_color(0x1a1a1a, 150);
	ui->style_colors[ui_col_window_border]     = make_color(0x0f0f0f, 200);
	ui->style_colors[ui_col_background]        = make_color(0x212121, 255);
	ui->style_colors[ui_col_hovered]           = make_color(0x242533, 255);
	ui->style_colors[ui_col_hot]               = make_color(0x393d5b, 255);
	ui->style_colors[ui_col_text]              = make_color(0xffffff, 255);
	ui->style_colors[ui_col_image_hovered]     = make_color(0xaaaeeb, 255);
	ui->style_colors[ui_col_image_hot]         = make_color(0x7686ff, 255);
	ui->style_colors[ui_col_image]             = make_color(0xffffff, 255);
	ui->style_colors[ui_col_dock]              = make_color(0xdb3d40, 150);

	ui->padding = 3;
	ui->column_size = 150;
	ui->window_max_height = 300;

	ui->window_meta = new_table(sizeof(struct window_meta));
	ui->strings = new_table(sizeof(struct text_entry));

	struct ui_dockspace* root_dockspace = &ui->dockspaces[ui->dockspace_count++];
	root_dockspace->rect = make_float_rect(0.0f, 0.0f, 1.0f, 1.0f);

	return ui;
}

void free_ui_context(struct ui_context* ui) {
	if (ui->windows) {
		for (u32 i = 0; i < ui->window_count; i++) {
			if (ui->windows[i].elements) {
				core_free(ui->windows[i].elements);
			}
		}

		core_free(ui->windows);
	}

	free_renderer(ui->renderer);

	free_table(ui->window_meta);

	for (table_iter(ui->strings, iter)) {
		core_free(((struct text_entry*)iter.value)->ptr);
	}
	free_table(ui->strings);

	core_free(ui);
}

void ui_set_color(struct ui_context* ui, u32 id, struct color color) {
	ui->style_colors[id] = color;
}

void ui_text_input_event(struct ui_context* ui, const char* text) {
	if (!ui->input_buf) { return; }

	u32 len = (u32)strlen(text);
	u32 buf_len = (u32)strlen(ui->input_buf);

	if (buf_len + len < ui->input_buf_size) {
		for (u32 i = 0; i < len; i++) {
			if (ui->input_filter(text[i])) {
				char after[256];
				memcpy(after, ui->input_buf + ui->input_cursor, buf_len - ui->input_cursor);
				ui->input_buf[ui->input_cursor] = '\0';
				strncat(ui->input_buf, text + i, 1);
				strncat(ui->input_buf, after, buf_len - ui->input_cursor);
				ui->input_cursor++;
			}
		}
	}
}

void ui_begin_frame(struct ui_context* ui) {
	ui->window_count = 0;

	ui->loading = null;

	i32 w, h;

	query_window(ui->window, &w, &h);

	renderer_clip(ui->renderer, make_rect(0, 0, w, h));

	ui->hovered = null;
	ui->hot = null;

	ui->window_count = 0;

	ui->current_dockspace = null;

	renderer_resize(ui->renderer, make_v2i(w, h));
}

static i32 cmp_window_z(const struct ui_window** a, const struct ui_window** b) {
	return (*b)->z - (*a)->z;
}

static void ui_window_change_dock(struct ui_context* ui, struct window_meta* meta, struct ui_dockspace* dock) {
	if (!meta) { return; }

	if (meta->dock) {
		meta->dock->occupied = false;

		if (meta->dock->parent) {
			switch (meta->dock->parent_dir) {
				case ui_dock_dir_left:
					meta->dock->parent->rect.w += meta->dock->rect.w;
					break;
				case ui_dock_dir_right:
					meta->dock->parent->rect.x -= meta->dock->rect.w;
					meta->dock->parent->rect.w += meta->dock->rect.w;
					break;
				case ui_dock_dir_up:
					meta->dock->parent->rect.h += meta->dock->rect.h;
					break;
				case ui_dock_dir_down:
					meta->dock->parent->rect.y -= meta->dock->rect.h;
					meta->dock->parent->rect.h += meta->dock->rect.h;
					break;
				default: break;
			}

			u32 idx = (u32)((u64)(meta->dock - ui->dockspaces));

			if (ui->dockspace_count > 1) {
				ui->dockspaces[idx] = ui->dockspaces[ui->dockspace_count - 1];
			}

			ui->dockspace_count--;
		}
	}

	meta->dock = dock;
}

void ui_end_frame(struct ui_context* ui) {
	if (ui->input_buf) {
		u32 buf_len = strlen(ui->input_buf);

		if (ui->input_cursor > buf_len) {
			ui->input_cursor = buf_len;
		}

		if (ui->input_cursor > 0) {
			if (key_just_pressed(main_window, KEY_BACKSPACE)) {
				char after[256];
				memcpy(after, ui->input_buf + ui->input_cursor, buf_len - ui->input_cursor);

				ui->input_buf[ui->input_cursor - 1] = '\0';
				strncat(ui->input_buf, after, buf_len - ui->input_cursor);
				ui->input_cursor--;
			}

			if (key_just_pressed(main_window, KEY_LEFT)) {
				ui->input_cursor--;
			}
		}

		if (ui->input_cursor < buf_len && key_just_released(main_window, KEY_RIGHT)) {
			ui->input_cursor++;
		}
	}

	if (!ui->hovered && clicked()) {
		ui->input_buf = null;
		ui->active = null;
	}

	struct ui_window* hovered[16];
	u32 hovered_count = ui_get_hovered_windows(ui, hovered, 16);

	qsort(hovered, hovered_count, sizeof(struct ui_window*),
		(int(*)(const void*, const void*))cmp_window_z);

	if (hovered_count > 0 && !ui->hovered) {
		struct ui_window* window = hovered[hovered_count - 1];

		v2i corner = v2i_add(window->position, window->dimentions);
		i32 dist = v2i_magnitude(v2i_sub(get_mouse_position(main_window), corner));

		if (dist < 20) {
			set_window_cursor(main_window, CURSOR_RESIZE);
		} else if (!ui->dragging) {
			set_window_cursor(main_window, CURSOR_POINTER);
		}

		if (mouse_btn_just_pressed(main_window, MOUSE_BTN_LEFT)) {
			ui->drag_start = get_mouse_position(main_window);
			ui->drag_offset = v2i_sub(ui->drag_start, window->position);

			ui->top_window = window;
			struct window_meta* meta = table_get(ui->window_meta, ui->top_window->title);
			meta->z = 0;

			if (!meta->dock && dist < 20) {
				ui->resizing = window;
			}

			for (u32 i = 0; i < ui->window_count; i++) {
				struct ui_window* w = ui->windows + i;
				if (w == ui->top_window) { continue; }
				
				struct window_meta* m = table_get(ui->window_meta, w->title);
				if (m) {
					m->z++;
				}
			}
		}

		i32 drag_start_dist = v2i_magnitude(v2i_sub(ui->drag_start, get_mouse_position(main_window)));
		if (dist > 20 && drag_start_dist > 10 && mouse_btn_pressed(main_window, MOUSE_BTN_LEFT)) {
			set_window_cursor(main_window, CURSOR_MOVE);
			ui->dragging = window;
		}
	} else if (hovered_count == 0) {
		set_window_cursor(main_window, CURSOR_POINTER);
	}

	struct ui_window** sorted_windows = core_alloc(ui->window_count * sizeof(struct ui_window*));
	for (u32 i = 0; i < ui->window_count; i++) {
		sorted_windows[i] = ui->windows + i;
	}

	qsort(sorted_windows, ui->window_count, sizeof(struct ui_window*),
		(i32(*)(const void*, const void*))cmp_window_z);

	for (u32 i = 0; i < ui->window_count; i++) {
		struct ui_window* window = sorted_windows[i];

		struct rect window_rect = make_rect(
			window->position.x, window->position.y,
			window->dimentions.x, window->dimentions.y);

		struct rect window_border_rect = make_rect(
			window->position.x - 1, window->position.y - 1,
			window->dimentions.x + 2, window->dimentions.y + 2);

		renderer_clip(ui->renderer, window_border_rect);

		ui_draw_rect(ui, window_border_rect,
			ui_col_window_border);
		ui_draw_rect(ui, window_rect,
			ui_col_window_background);

		i32 title_w = text_width(window->font, window->title);
		i32 title_h = text_height(window->font, window->title) + ui->padding * 2;
		render_text(ui->renderer, window->font, window->title,
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
					render_text(ui->renderer, el->font, el->as.text.text, el->position.x, el->position.y, el->color);
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
					render_text(ui->renderer, el->font, el->as.button.text,
						el->position.x + ui->padding,
						el->position.y + ui->padding,
						el->color);
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

					i32 w = 0, h = 0, c_w = 0;
					if (el->as.text_input.buf) {
						c_w = text_width_n(el->font, el->as.text_input.buf, ui->input_cursor);
						w = text_width(el->font, el->as.text_input.buf);
						h = text_height(el->font, el->as.text_input.buf);
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
							input_scroll = el->as.text_input.input_dimentions.x - c_w - ui->padding - h;

							if (input_scroll > 0) {
								input_scroll = 0;
							}
						}
					}

					render_text(ui->renderer, el->font, el->as.text_input.buf,
						(el->as.text_input.input_pos.x + ui->padding) + input_scroll,
						el->as.text_input.input_pos.y + ui->padding,
						el->color);	

					if (want_reset) {
						renderer_clip(ui->renderer, clip_rect);
					}

					if (ui->active == el) {
						ui_draw_rect(ui, make_rect(
							el->as.text_input.input_pos.x + c_w + ui->padding + input_scroll,
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

	/* Draw & update the window dock */
	bool docking = true;
	if (ui->current_dockspace) {
		v2i dock_pos = get_dockspace_position(ui->current_dockspace);
		v2i dock_dim = get_dockspace_dimentions(ui->current_dockspace);
		renderer_clip(ui->renderer, make_rect(dock_pos.x, dock_pos.y, dock_dim.x, dock_dim.y));

		i32 dock_handle_size = 80;
		struct rect current_dockspace_rect = get_dock_rect_screen(ui->current_dockspace->rect);
		v2i dock_centre = {
			current_dockspace_rect.x + current_dockspace_rect.w / 2,
			current_dockspace_rect.y + current_dockspace_rect.h / 2,
		};

		struct rect left = {
			.x = dock_centre.x - (dock_handle_size + dock_handle_size / 2) - 3,
			.y = dock_centre.y - dock_handle_size / 2,
			.w = dock_handle_size, .h = dock_handle_size
		};
		struct rect right = {
			.x = dock_centre.x + dock_handle_size / 2 + 3,
			.y = dock_centre.y - dock_handle_size / 2,
			.w = dock_handle_size, .h = dock_handle_size
		};
		struct rect top = {
			.x = dock_centre.x - dock_handle_size / 2,
			.y = dock_centre.y - dock_handle_size - dock_handle_size / 2 - 3,
			.w = dock_handle_size, .h = dock_handle_size
		};
		struct rect bottom = {
			.x = dock_centre.x - dock_handle_size / 2,
			.y = dock_centre.y + dock_handle_size / 2 + 3,
			.w = dock_handle_size, .h = dock_handle_size
		};
		struct rect middle = {
			.x = dock_centre.x - dock_handle_size / 2,
			.y = dock_centre.y - dock_handle_size / 2,
			.w = dock_handle_size, .h = dock_handle_size
		};
		ui_draw_rect(ui, left, ui_col_dock);
		ui_draw_rect(ui, right, ui_col_dock);
		ui_draw_rect(ui, top, ui_col_dock);
		ui_draw_rect(ui, bottom, ui_col_dock);
		ui_draw_rect(ui, middle, ui_col_dock);

		struct window_meta* meta = table_get(ui->window_meta, ui->dragging->title);

		if (!meta) {
			struct window_meta new_meta = { ui->dragging->position, ui->dragging->dimentions, 0, 0 };
			meta = table_set(ui->window_meta, ui->dragging->title, &new_meta);
		}

		struct rect split_preview = { 0 };
		if (mouse_over_rect(left)) {
			split_preview = (struct rect) {
				.x = current_dockspace_rect.x,
				.y = current_dockspace_rect.y,
				.w = current_dockspace_rect.w / 2,
				.h = current_dockspace_rect.h
			};

			if (mouse_btn_just_released(main_window, MOUSE_BTN_LEFT)) {
				struct ui_dockspace* new_dock = ui->dockspaces + ui->dockspace_count++;

				ui->current_dockspace->rect.w /= 2;
				new_dock->rect = ui->current_dockspace->rect;
				new_dock->parent = ui->current_dockspace;
				new_dock->occupied = true;

				new_dock->parent_dir = ui_dock_dir_right;

				ui->current_dockspace->rect.x += ui->current_dockspace->rect.w;

				ui_window_change_dock(ui, meta, new_dock);
			}
		} else if (mouse_over_rect(right)) {
			split_preview = (struct rect) {
				.x = current_dockspace_rect.x + current_dockspace_rect.w / 2.0f,
				.y = current_dockspace_rect.y,
				.w = current_dockspace_rect.w / 2.0f,
				.h = current_dockspace_rect.h
			};

			if (mouse_btn_just_released(main_window, MOUSE_BTN_LEFT)) {
				struct ui_dockspace* new_dock = ui->dockspaces + ui->dockspace_count++;

				ui->current_dockspace->rect.w /= 2;
				new_dock->rect = ui->current_dockspace->rect;
				new_dock->rect.x += ui->current_dockspace->rect.w;
				new_dock->parent = ui->current_dockspace;
				new_dock->occupied = true;

				new_dock->parent_dir = ui_dock_dir_left;

				ui_window_change_dock(ui, meta, new_dock);
			}
		} else if (mouse_over_rect(top)) {
			split_preview = (struct rect) {
				.x = current_dockspace_rect.x,
				.y = current_dockspace_rect.y,
				.w = current_dockspace_rect.w,
				.h = current_dockspace_rect.h / 2,
			};

			if (mouse_btn_just_released(main_window, MOUSE_BTN_LEFT)) {
				struct ui_dockspace* new_dock = ui->dockspaces + ui->dockspace_count++;

				ui->current_dockspace->rect.h /= 2;
				new_dock->rect = ui->current_dockspace->rect;
				new_dock->parent = ui->current_dockspace;
				new_dock->occupied = true;

				new_dock->parent_dir = ui_dock_dir_down;

				ui->current_dockspace->rect.y += ui->current_dockspace->rect.h;

				ui_window_change_dock(ui, meta, new_dock);
			}
		} else if (mouse_over_rect(bottom)) {
			split_preview = (struct rect) {
				.x = current_dockspace_rect.x,
				.y = current_dockspace_rect.y + current_dockspace_rect.h / 2,
				.w = current_dockspace_rect.w,
				.h = current_dockspace_rect.h / 2,
			};

			if (mouse_btn_just_released(main_window, MOUSE_BTN_LEFT)) {
				struct ui_dockspace* new_dock = ui->dockspaces + ui->dockspace_count++;

				ui->current_dockspace->rect.h /= 2;
				new_dock->rect = ui->current_dockspace->rect;
				new_dock->rect.y += ui->current_dockspace->rect.h;
				new_dock->parent = ui->current_dockspace;
				new_dock->occupied = true;

				new_dock->parent_dir = ui_dock_dir_up;

				ui_window_change_dock(ui, meta, new_dock);
			}
		} else if (mouse_over_rect(middle) && !ui->current_dockspace->occupied) {
			split_preview = current_dockspace_rect;

			if (mouse_btn_just_released(main_window, MOUSE_BTN_LEFT)) {
				ui_window_change_dock(ui, meta, ui->current_dockspace);
			}
		} else {
			docking = false;
		}

		ui_draw_rect(ui, split_preview, ui_col_dock);
	}

	if (mouse_btn_just_released(main_window, MOUSE_BTN_LEFT)) {
		if (ui->dragging && !docking) {
			struct window_meta* meta = table_get(ui->window_meta, ui->dragging->title);
			ui_window_change_dock(ui, meta, null);
		}

		ui->dragging = null;
		ui->resizing = null;
		set_window_cursor(main_window, CURSOR_POINTER);
	}

	core_free(sorted_windows);

	if (ui->loading) {
		i32 win_w, win_h;
		query_window(main_window, &win_w, &win_h);

		i32 text_h = text_height(ui->loading_f, ui->loading);

		i32 bar_w = 300;
		i32 bar_h = 16;

		struct rect container = {
			.x = (win_w / 2) - (bar_w / 2) - ui->padding,
			.y = (win_h / 2) - ((bar_h + text_h + ui->padding) / 2) - ui->padding, 
			.w = bar_w + ui->padding * 2,
			.h = bar_h + text_h + ui->padding * 3
		};

		struct rect outline = {
			.x = container.x - 1,
			.y = container.y - 1,
			.w = container.w + 2,
			.h = container.h + 2
		};

		renderer_clip(ui->renderer, outline);

		struct rect bar_rect = {
			.x = container.x + ui->padding,
			.y = container.y + text_h + ui->padding * 2,
			.w = container.w - ui->padding * 2,
			.h = container.h - (text_h + ui->padding * 3),
		};

		f32 p = (f32)ui->loading_p / 100.0f;
		if (p < 0.0f) { p = 0.0f; }
		if (p > 1.0f) { p = 1.0f; }

		struct rect bar = bar_rect;
		bar.w = (i32)(p * (f32)bar_rect.w);

		ui_draw_rect(ui, outline, ui_col_window_border);
		ui_draw_rect(ui, container, ui_col_window_background);
		ui_draw_rect(ui, bar_rect, ui_col_background);
		ui_draw_rect(ui, bar, ui_col_hot);
		render_text(ui->renderer, ui->loading_f, ui->loading, container.x + ui->padding,
			container.y + ui->padding, ui->style_colors[ui_col_text]);
	}

	for (table_iter(ui->strings, iter)) {
		struct text_entry* entry = iter.value;
		entry->life--;

		if (entry->life < 0) {
			char* ptr = entry->ptr;
			table_delete(ui->strings, entry->ptr);
			core_free(ptr);
		}
	}

	renderer_flush(ui->renderer);
	renderer_end_frame(ui->renderer);
}

bool ui_begin_window(struct ui_context* ui, const char* name, v2i position) {
	if (ui->window_count >= ui->window_capacity) {
		ui->window_capacity = ui->window_capacity < 8 ? 8 : ui->window_capacity * 2;
		ui->windows = core_realloc(ui->windows, ui->window_capacity * sizeof(struct ui_window));
		
		for (u32 i = ui->window_count; i < ui->window_capacity; i++) {
			ui->windows[i] = (struct ui_window) { 0 };
		}
	}	

	struct ui_window* window = &ui->windows[ui->window_count++];

	ui->current_window = window;

	window->font = ui->font;

	window->element_count = 0;
	window->title = ui_copy_string(ui, name);

	window->position = position;
	struct window_meta* meta = table_get(ui->window_meta, name);
	
	window->dimentions = make_v2i(300, ui->window_max_height);

	if (!meta) {
		struct window_meta new_meta = { position, window->dimentions, 0, 0 };
		table_set(ui->window_meta, name, &new_meta);

		meta = table_get(ui->window_meta, name);
	} else {
		if (ui->dragging != window && meta->dock) {
			window->position = get_dockspace_position(meta->dock);
			window->dimentions = get_dockspace_dimentions(meta->dock);
		} else {
			window->position = meta->position;
			window->dimentions = meta->dimentions;
		}
		window->z = meta->z;
	}

	ui->cursor_pos = make_v2i(
		window->position.x + ui->padding,
		window->position.y + meta->scroll + text_height(ui->font, window->title) + ui->padding * 2);

	return true;
}

void ui_end_window(struct ui_context* ui) {
	struct ui_window* window = ui->current_window;

	struct window_meta* meta = table_get(ui->window_meta, window->title);
	if (ui->dragging != window && meta->dock) {
		window->position = get_dockspace_position(meta->dock);
		window->dimentions = get_dockspace_dimentions(meta->dock);
	} else {
		window->position = meta->position;
		window->dimentions = meta->dimentions;
	}
	if (meta) {
		struct rect window_rect = make_rect(
			window->position.x, window->position.y,
			window->dimentions.x, window->dimentions.y);

		if (mouse_over_rect(window_rect)) {
			meta->scroll += get_scroll(main_window) * (text_height(ui->font, window->title) + ui->padding);

			i32 scroll_bottom = (ui->cursor_pos.y - window->position.y) - window->dimentions.y;
			if (scroll_bottom < 0) {
				meta->scroll -= scroll_bottom;
			}

			if (meta->scroll > 0) {
				meta->scroll = 0;
			}
		}

		if (window == ui->dragging) {
			meta->position = v2i_sub(get_mouse_position(main_window), ui->drag_offset);

			ui->current_dockspace = null;
			for (u32 i = 0; i < ui->dockspace_count; i++) {
				if (mouse_over_rect(get_dock_rect_screen(ui->dockspaces[i].rect))) {
					ui->current_dockspace = ui->dockspaces + i;
					break;
				}
			}
		}

		if (window == ui->resizing) {
			meta->dimentions = v2i_sub(get_mouse_position(main_window), window->position);

			if (meta->dimentions.x < 100) {
				meta->dimentions.x = 100;
			}

			if (meta->dimentions.y < 50) {
				meta->dimentions.y = 50;
			}
		}
	}

	ui->current_window = null;
}

void ui_columns(struct ui_context* ui, u32 count, i32 width) {
	if (count == 0) { count = 1; }

	ui->columns = count;
	ui->column_size = width;
}

void ui_color(struct ui_context* ui, struct color color) {
	ui->override_color = true;
	ui->color_override = color;
}

void ui_reset_color(struct ui_context* ui) {
	ui->override_color = false;
}

static void ui_advance(struct ui_context* ui, i32 el_height) {
	ui->item++;

	if (ui->item >= ui->columns) {
		ui->cursor_pos.x = ui->current_window->position.x + ui->padding;
		ui->cursor_pos.y += el_height;
		ui->item = 0;
	} else {
		ui->cursor_pos.x += ui->column_size;
	}
}

void ui_text(struct ui_context* ui, const char* text) {
	ui_window_add_item(ui, ui->current_window, (struct ui_element) {
		.type = ui_el_text,
		.position = ui->cursor_pos,
		.as.text = {
			.text = ui_copy_string(ui, text)
		}
	});

	ui_advance(ui, text_height(ui->font, text) + ui->padding);
}

bool ui_button(struct ui_context* ui, const char* text) {
	struct rect r = make_rect(ui->cursor_pos.x, ui->cursor_pos.y,
		text_width(ui->font, text) + ui->padding * 2,
		text_height(ui->font, text) + ui->padding * 2);

	struct ui_element* e = ui_window_add_item(ui, ui->current_window, (struct ui_element) {
		.type = ui_el_button,
		.position = ui->cursor_pos,
		.as.button = {
			.text = ui_copy_string(ui, text),
			.dimentions = make_v2i(r.w, r.h)
		}
	});

	ui_advance(ui, text_height(ui->font, text) + ui->padding * 3);

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

bool ui_text_input(struct ui_context* ui, char* buf, u32 buf_size) {
	struct rect r = make_rect(
		ui->cursor_pos.x,
		ui->cursor_pos.y,
		ui->column_size - ui->padding * 2,
		font_height(ui->font) + ui->padding * 2);

	struct ui_element* e = ui_window_add_item(ui, ui->current_window, (struct ui_element) {
		.type = ui_el_text_input,
		.position = ui->cursor_pos,
		.as.text_input = {
			.buf = buf,
			.buf_size = buf_size,
			.input_dimentions = make_v2i(r.w, r.h),
			.input_pos = make_v2i(r.x, r.y)
		}
	});

	ui_advance(ui, font_height(ui->font) + ui->padding * 4);

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
				ui->input_cursor = 0;
			}
		}
	}

	if (ui->active == e && key_just_released(main_window, KEY_RETURN)) {
		ui->active = null;
		ui->input_buf = null;
		return true;
	}

	return false;
}

void ui_image(struct ui_context* ui, struct texture* texture, struct rect rect) {
	struct rect r = make_rect(ui->cursor_pos.x, ui->cursor_pos.y, 100, 100);

	struct ui_element* e = ui_window_add_item(ui, ui->current_window, (struct ui_element) {
		.type = ui_el_image,
		.position = ui->cursor_pos,
		.as.image = {
			.texture = texture,
			.dimentions = make_v2i(100, 100),
			.rect = rect
		}
	});

	ui_advance(ui, e->as.image.dimentions.y + ui->padding);
}

bool ui_image_button(struct ui_context* ui, struct texture* texture, struct rect rect) {
	struct rect r = make_rect(ui->cursor_pos.x, ui->cursor_pos.y, 100, 100);

	struct ui_element* e = ui_window_add_item(ui, ui->current_window, (struct ui_element) {
		.type = ui_el_image,
		.position = ui->cursor_pos,
		.as.image = {
			.texture = texture,
			.dimentions = make_v2i(100, 100),
			.rect = rect
		}
	});

	ui_advance(ui, e->as.image.dimentions.y + ui->padding);

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

void ui_loading_bar(struct ui_context* ui, const char* text, i32 percentage) {
	ui->loading = ui_copy_string(ui, text);
	ui->loading_p = percentage;
	ui->loading_f = ui->font;
}

struct renderer* ui_get_renderer(struct ui_context* ui) {
	return ui->renderer;
}

struct font* ui_get_font(struct ui_context* ui) {
	return ui->font;
}

void ui_set_font(struct ui_context* ui, struct font* font) {
	ui->font = font;
}

void ui_save_layout(struct ui_context* ui, const char* path) {
	FILE* file = fopen(path, "wb");
	if (!file) {
		fprintf(stderr, "Failed to fopen `%s' for writing.\n", path);
		return;
	}

	u32 meta_count = get_table_count(ui->window_meta);
	fwrite(&meta_count, sizeof(meta_count), 1, file);

	for (table_iter(ui->window_meta, iter)) {
		const char* title = iter.key;
		struct window_meta* meta = iter.value;

		u32 title_len = strlen(title);
		fwrite(&title_len, sizeof(title_len), 1, file);
		fwrite(title, 1, title_len, file);

		fwrite(&meta->position.x, sizeof(meta->position.x), 1, file);
		fwrite(&meta->position.y, sizeof(meta->position.y), 1, file);
		fwrite(&meta->dimentions.x, sizeof(meta->dimentions.x), 1, file);
		fwrite(&meta->dimentions.y, sizeof(meta->dimentions.y), 1, file);
		fwrite(&meta->scroll, sizeof(meta->scroll), 1, file);
		fwrite(&meta->z, sizeof(meta->z), 1, file);

		i32 dock_idx;
		if (!meta->dock) {
			dock_idx = -1;
		} else {
			dock_idx = meta->dock - ui->dockspaces;
		}
		fwrite(&dock_idx, sizeof(dock_idx), 1, file);
	}

	fwrite(&ui->dockspace_count, sizeof(ui->dockspace_count), 1, file);

	for (u32 i = 0; i < ui->dockspace_count; i++) {
		struct ui_dockspace* dock = ui->dockspaces + i;

		i32 parent_idx = dock->parent - ui->dockspaces;
		if (dock->parent) {	
			parent_idx = dock->parent - ui->dockspaces;		
		} else {
			parent_idx = -1;
		}

		fwrite(&parent_idx, sizeof(parent_idx), 1, file);
		fwrite(&dock->parent_dir, sizeof(dock->parent_dir), 1, file);
		fwrite(&dock->occupied, sizeof(dock->occupied), 1, file);

		fwrite(&dock->rect.x, sizeof(dock->rect.x), 1, file);
		fwrite(&dock->rect.y, sizeof(dock->rect.y), 1, file);
		fwrite(&dock->rect.w, sizeof(dock->rect.w), 1, file);
		fwrite(&dock->rect.h, sizeof(dock->rect.h), 1, file);
	}

	fclose(file);
}

void ui_load_layout(struct ui_context* ui, const char* path) {
	FILE* file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "Failed to fopen `%s' for reading.\n", path);
		return;
	}

	u32 meta_count;
	fread(&meta_count, sizeof(meta_count), 1, file);

	for (u32 i = 0; i < meta_count; i++) {
		struct window_meta meta = { 0 };

		u32 title_len;
		fread(&title_len, sizeof(title_len), 1, file);

		char* title = core_alloc(title_len + 1);
		title[title_len] = '\0';
		fread(title, 1, title_len, file);

		fread(&meta.position.x, sizeof(meta.position.x), 1, file);
		fread(&meta.position.y, sizeof(meta.position.y), 1, file);
		fread(&meta.dimentions.x, sizeof(meta.dimentions.x), 1, file);
		fread(&meta.dimentions.y, sizeof(meta.dimentions.y), 1, file);
		fread(&meta.scroll, sizeof(meta.scroll), 1, file);
		fread(&meta.z, sizeof(meta.z), 1, file);

		i32 dock_idx;
		fread(&dock_idx, sizeof(dock_idx), 1, file);

		if (dock_idx >= 0) {
			meta.dock = ui->dockspaces + dock_idx;
		}

		table_set(ui->window_meta, title, &meta);

		core_free(title);
	}

	fread(&ui->dockspace_count, sizeof(ui->dockspace_count), 1, file);
	memset(ui->dockspaces, 0, ui->dockspace_count * sizeof(struct ui_dockspace));

	for (u32 i = 0; i < ui->dockspace_count; i++) {
		struct ui_dockspace* dock = ui->dockspaces + i;

		i32 parent_idx;

		fread(&parent_idx, sizeof(parent_idx), 1, file);
		fread(&dock->parent_dir, sizeof(dock->parent_dir), 1, file);
		fread(&dock->occupied, sizeof(dock->occupied), 1, file);

		fread(&dock->rect.x, sizeof(dock->rect.x), 1, file);
		fread(&dock->rect.y, sizeof(dock->rect.y), 1, file);
		fread(&dock->rect.w, sizeof(dock->rect.w), 1, file);
		fread(&dock->rect.h, sizeof(dock->rect.h), 1, file);

		if (parent_idx >= 0) {
			dock->parent = ui->dockspaces + parent_idx;
		}
	}

	fclose(file);
}
