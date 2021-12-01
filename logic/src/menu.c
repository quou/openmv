#include <string.h>

#include "core.h"
#include "keymap.h"
#include "logic_store.h"
#include "menu.h"
#include "platform.h"
#include "sprites.h"

enum {
	menu_item_selectable = 0,
	menu_item_label
};

struct menu_item {
	u32 type;

	union {
		struct {
			char* text;
			menu_on_select on_select;
		} selectable;

		struct {
			char* text;
		} label;
	} as;
};

struct menu {
	struct renderer* renderer;
	struct font* font;

	struct menu_item* items;
	u32 item_count;
	u32 item_capacity;

	i32 selected_item;
};

struct menu* new_menu(struct shader shader, struct font* font) {
	struct menu* menu = core_calloc(1, sizeof(struct menu));

	menu->font = font;
	menu->renderer = new_renderer(shader, make_v2i(800, 600));

	return menu;
}

void free_menu(struct menu* menu) {
	free_renderer(menu->renderer);

	if (menu->items) {
		for (u32 i = 0; i < menu->item_count; i++) {
			struct menu_item* item = menu->items + i;

			switch (item->type) {
				case menu_item_selectable: {
					core_free(item->as.selectable.text);
				} break;
				default: break;
			}
		}

		core_free(menu->items);
	}

	core_free(menu);
}

void menu_update(struct menu* menu) {
	if (key_just_pressed(main_window, mapped_key("up"))) {
		menu->selected_item--;

		if (menu->selected_item < 0) {
			menu->selected_item = menu->item_count - 1;
		}
	}

	if (key_just_pressed(main_window, mapped_key("down"))) {
		menu->selected_item++;

		if (menu->selected_item >= menu->item_count) {
			menu->selected_item = 0;
		}
	}

	if (menu->items[menu->selected_item].type != menu_item_selectable) {
		for (u32 i = menu->selected_item; i < menu->item_count; i++) {
			struct menu_item* item = menu->items + i;

			if (item->type == menu_item_selectable) {
				menu->selected_item = i;
				break;
			}
		}
	}

	if (key_just_pressed(main_window, mapped_key("submit")) && menu->items[menu->selected_item].type == menu_item_selectable) {
		menu->items[menu->selected_item].as.selectable.on_select(menu);
	}

	i32 win_w, win_h;
	query_window(main_window, &win_w, &win_h);

	menu->renderer->camera = m4f_orth(0.0f, (float)win_w, (float)win_h, 0.0f, -1.0f, 1.0f);

	i32 y = (win_h / 2) - (menu->item_count * text_height(menu->font) / 2);

	struct textured_quad background_quad = {
		.texture = null,
		.rect = { 0 },
		.position = { 0, 0 },
		.dimentions = { win_w, win_h },
		.color = make_color(0x000000, 200)
	};

	renderer_push(menu->renderer, &background_quad);

	for (u32 i = 0; i < menu->item_count; i++) {
		struct menu_item* item = menu->items + i;

		i32 item_w = 0;
		i32 item_h = 0;

		switch (item->type) {
			case menu_item_selectable: {
				i32 text_w = text_width(menu->font, item->as.selectable.text);
				item_w = text_w;
				item_h = text_height(menu->font);

				i32 x = (win_w / 2) - (text_w / 2);

				render_text(menu->renderer, menu->font, item->as.selectable.text, x, y, make_color(0xffffff, 255));

				y += text_height(menu->font);
			} break;
			case menu_item_label: {
				i32 text_w = text_width(menu->font, item->as.label.text);
				item_w = text_w;
				item_h = text_height(menu->font);

				i32 x = (win_w / 2) - (text_w / 2);

				render_text(menu->renderer, menu->font, item->as.label.text, x, y, make_color(0x6cafb5, 255));

				y += text_height(menu->font);
			} break;
			default: break;
		}

		if (menu->selected_item == i) {
			struct sprite sprite = get_sprite(sprid_icon_ptr);

			i32 w = sprite.rect.w * 4;
			i32 h = sprite.rect.h * 4;

			struct textured_quad quad = {
				.texture = sprite.texture,
				.position = make_v2i((win_w / 2) - (item_w / 2) - w - 5, y - (item_h) - (h / 4)),
				.dimentions = make_v2i(w, h),
				.rect = sprite.rect,
				.color = sprite.color
			};

			renderer_push(menu->renderer, &quad);
		}
	}

	renderer_flush(menu->renderer);
}

static struct menu_item* add_menu_item(struct menu* menu) {
	if (menu->item_count >= menu->item_capacity) {
		menu->item_capacity = menu->item_capacity < 8 ? 8 : menu->item_capacity * 2;
		menu->items = core_realloc(menu->items, menu->item_capacity * sizeof(struct menu_item));
	}

	return &menu->items[menu->item_count++];
}

void menu_add_selectable(struct menu* menu, const char* label, menu_on_select on_select) {
	struct menu_item* item = add_menu_item(menu);

	*item = (struct menu_item) { 0 };

	item->type = menu_item_selectable;

	item->as.selectable.text = copy_string(label);
	item->as.selectable.on_select = on_select;
}

void menu_add_label(struct menu* menu, const char* label) {
	struct menu_item* item = add_menu_item(menu);

	*item = (struct menu_item) { 0 };

	item->type = menu_item_label;

	item->as.label.text = copy_string(label);
}

void menu_reset_selection(struct menu* menu) {
	menu->selected_item = 0;
}

struct prompt_ctx {
	char* message;
	u32 message_len;
	u32 current_character;

	double type_speed;

	double timer;

	struct renderer* renderer;
	struct font* font;

	bool selected;

	prompt_submit_func on_submit;
};

void prompts_init(struct shader shader, struct font* font) {
	logic_store->prompt_ctx = core_calloc(1, sizeof(struct prompt_ctx));	

	struct prompt_ctx* ctx = (struct prompt_ctx*)logic_store->prompt_ctx;

	ctx->font = font;
	ctx->renderer = new_renderer(shader, make_v2i(800, 600));
}

void prompts_deinit() {
	core_free(logic_store->prompt_ctx);
}

void message_prompt(const char* text) {
	struct prompt_ctx* ctx = (struct prompt_ctx*)logic_store->prompt_ctx;

	if (ctx->message) {
		core_free(ctx->message);
	}

	ctx->message = copy_string(text);
	ctx->message_len = (u32)strlen(text);
	ctx->current_character = 0;
	ctx->on_submit = null;

	logic_store->frozen = true;
}

void prompt_ask(const char* text, prompt_submit_func on_submit) {
	struct prompt_ctx* ctx = (struct prompt_ctx*)logic_store->prompt_ctx;

	if (ctx->message) {
		core_free(ctx->message);
	}

	ctx->message = copy_string(text);
	ctx->message_len = (u32)strlen(text);
	ctx->current_character = 0;
	ctx->on_submit = on_submit;
	ctx->selected = false;

	logic_store->frozen = true;
}

void prompts_update(double ts) {
	struct prompt_ctx* ctx = (struct prompt_ctx*)logic_store->prompt_ctx;

	i32 win_w, win_h;
	query_window(main_window, &win_w, &win_h);
	ctx->renderer->camera = m4f_orth(0.0f, (float)win_w, (float)win_h, 0.0f, -1.0f, 1.0f);

	if (ctx->message) {
		struct textured_quad back = {
			.texture = null,
			.position = { 10, win_h - 150 },
			.dimentions = { win_w - 20, 140 },
			.rect = { 0 },
			.color = make_color(0x6d5e64, 255)
		};

		renderer_push(ctx->renderer, &back);

		if (key_pressed(main_window, mapped_key("submit")) || key_pressed(main_window, mapped_key("jump"))) {
			ctx->type_speed = 200.0;
		} else {
			ctx->type_speed = 20.0;
		}

		ctx->timer += ctx->type_speed * ts;
		if (ctx->timer > 1.0 && ctx->current_character < ctx->message_len) {
			ctx->timer = 0.0;
			ctx->current_character++;
		}

		i32 h = text_height(ctx->font);

		render_text_n(ctx->renderer, ctx->font, ctx->message,
			ctx->current_character, 20, win_h - h - (back.dimentions.y / 2), make_color(0xffffff, 255));

		if (ctx->on_submit && ctx->current_character >= ctx->message_len) {
			const char* text = "Yes / No";
			const char* yes_text = "Yes";
			const char* no_text = "No";
			const char* mid_text = " / ";

			i32 w = text_width(ctx->font, text);

			struct textured_quad back2 = {
				.texture = null,
				.position = { win_w - w + 5 * 2 - 40, win_h - 160 },
				.dimentions = { w + 5 * 2, h + 5 * 2 },
				.rect = { 0 },
				.color = make_color(0x6b5151, 255)
			};

			renderer_push(ctx->renderer, &back2);

			u32 normal_color = 0xffffff;
			u32 selected_color = 0xefe74f;

			if (key_just_pressed(main_window, mapped_key("left")) || key_just_pressed(main_window, mapped_key("right"))) {
				ctx->selected = !ctx->selected;
			}

			i32 offset = 3;

			i32 x = back2.position.x + 5;
			x = render_text(ctx->renderer, ctx->font, yes_text,
				x, (back2.position.y + 5) - (ctx->selected ? offset : 0),
				ctx->selected ? make_color(selected_color, 255) : make_color(normal_color, 255));
			x = render_text(ctx->renderer, ctx->font, mid_text,
				x, back2.position.y + 5, make_color(0xffffff, 255));
			x = render_text(ctx->renderer, ctx->font, no_text,
				x, (back2.position.y + 5) - (ctx->selected ? 0 : offset),
				ctx->selected ? make_color(normal_color, 255) : make_color(selected_color, 255));

			if (key_just_pressed(main_window, mapped_key("submit"))) {
				ctx->on_submit(ctx->selected);

				ctx->message = null;
				ctx->on_submit = null;
				logic_store->frozen = false;
			}
		}

		if (!ctx->on_submit && ctx->current_character >= ctx->message_len - 1 && 
				(key_just_pressed(main_window, mapped_key("submit")) || key_just_pressed(main_window, mapped_key("jump")))) {
			ctx->message = null;
			logic_store->frozen = false;
		}
	}

	renderer_flush(ctx->renderer);
}
