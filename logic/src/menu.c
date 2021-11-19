#include "core.h"
#include "keymap.h"
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

struct menu* new_menu(struct shader* shader, struct font* font) {
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
