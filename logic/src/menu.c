#include <string.h>

#include "consts.h"
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

	struct audio_clip* select_sound;
};

struct menu* new_menu(struct font* font) {
	struct menu* menu = core_calloc(1, sizeof(struct menu));

	menu->font = font;
	menu->renderer = logic_store->ui_renderer;

	menu->select_sound = load_audio_clip("res/aud/select.wav");

	return menu;
}

void free_menu(struct menu* menu) {
	if (menu->items) {
		for (u32 i = 0; i < menu->item_count; i++) {
			struct menu_item* item = menu->items + i;

			switch (item->type) {
				case menu_item_selectable: {
					core_free(item->as.selectable.text);
				} break;
				case menu_item_label: {
					core_free(item->as.label.text);
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
		play_audio_clip(menu->select_sound);
		menu->items[menu->selected_item].as.selectable.on_select(menu);
	}

	renderer_fit_to_main_window(menu->renderer);

	u32 win_w = menu->renderer->dimentions.x;
	u32 win_h = menu->renderer->dimentions.y;

	i32 y = (win_h / 2) - (menu->item_count * font_height(menu->font) / 2);

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
				item_h = font_height(menu->font);

				i32 x = (win_w / 2) - (text_w / 2);

				render_text(menu->renderer, menu->font, item->as.selectable.text, x, y, make_color(0xffffff, 255));

				y += font_height(menu->font);
			} break;
			case menu_item_label: {
				i32 text_w = text_width(menu->font, item->as.label.text);
				item_w = text_w;
				item_h = font_height(menu->font);

				i32 x = (win_w / 2) - (text_w / 2);

				render_text(menu->renderer, menu->font, item->as.label.text, x, y, make_color(0x6cafb5, 255));

				y += font_height(menu->font);
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

	f64 type_speed;

	f64 timer;

	struct renderer* renderer;
	struct font* font;

	bool selected;
	bool nullify;

	prompt_submit_func on_submit;
	prompt_finish_func on_finish;

	void *udata;

	struct audio_clip* type_sound;
	struct audio_clip* select_sound;
	struct audio_clip* decline_sound;
};

void prompts_init(struct font* font) {
	logic_store->prompt_ctx = core_calloc(1, sizeof(struct prompt_ctx));	

	struct prompt_ctx* ctx = (struct prompt_ctx*)logic_store->prompt_ctx;

	ctx->font = font;
	ctx->renderer = logic_store->ui_renderer;

	ctx->type_speed = 20.0;

	ctx->type_sound = load_audio_clip("res/aud/type.wav");
	ctx->select_sound = load_audio_clip("res/aud/select.wav");
	ctx->decline_sound = load_audio_clip("res/aud/decline.wav");
}

void prompts_deinit() {
	struct prompt_ctx* ctx = (struct prompt_ctx*)logic_store->prompt_ctx;

	if (ctx->message) {
		core_free(ctx->message);
	}

	core_free(logic_store->prompt_ctx);
}

void message_prompt(const char* text) {
	message_prompt_ex(text, null, null);
}

void message_prompt_ex(const char* text, prompt_finish_func on_finish, void* udata) {
	struct prompt_ctx* ctx = (struct prompt_ctx*)logic_store->prompt_ctx;

	if (ctx->message) {
		core_free(ctx->message);
	}

	if (ctx->nullify) {
		ctx->nullify = false;
	}

	ctx->message = copy_string(text);
	ctx->message_len = (u32)strlen(text);
	ctx->current_character = 0;
	ctx->on_submit = null;

	ctx->timer = 0.0;

	ctx->udata = udata;
	ctx->on_finish = on_finish;

	logic_store->frozen = true;
}

void prompt_ask(const char* text, prompt_submit_func on_submit, void* udata) {
	struct prompt_ctx* ctx = (struct prompt_ctx*)logic_store->prompt_ctx;

	if (ctx->message) {
		core_free(ctx->message);
	}

	if (ctx->nullify) {
		ctx->nullify = false;
	}

	ctx->message = copy_string(text);
	ctx->message_len = (u32)strlen(text);
	ctx->current_character = 0;
	ctx->on_submit = on_submit;
	ctx->on_finish = null;
	ctx->selected = false;
	ctx->udata = udata;

	ctx->timer = 0.0;

	logic_store->frozen = true;
}

void prompts_update(f64 ts) {
	struct prompt_ctx* ctx = (struct prompt_ctx*)logic_store->prompt_ctx;

	i32 win_w, win_h;
	query_window(main_window, &win_w, &win_h);
	ctx->renderer->camera = m4f_orth(0.0f, (f32)win_w, (f32)win_h, 0.0f, -1.0f, 1.0f);

	if (ctx->message) {
		struct textured_quad back = {
			.texture = null,
			.position = { 10, win_h - 160 },
			.dimentions = { win_w - 20, 150 },
			.rect = { 0 },
			.color = make_color(0x6d5e64, 255)
		};

		renderer_push(ctx->renderer, &back);

		ctx->timer += ctx->type_speed * ts;
		if (ctx->timer > 1.0 && ctx->current_character < ctx->message_len) {
			ctx->timer = 0.0;
			ctx->current_character++;

			play_audio_clip(ctx->type_sound);
		}

		i32 h = font_height(ctx->font);

		struct sprite coin_sprite = get_sprite(sprid_coin);
		struct textured_quad coin_quad = {
			.texture = coin_sprite.texture,
			.dimentions = { coin_sprite.rect.w * sprite_scale, coin_sprite.rect.h * sprite_scale },
			.rect = coin_sprite.rect,
			.color = { 255, 255, 255, 255 }
		};

		const char* line = ctx->message;
		u32 line_len = 0;

		u32 lines = 0;
		for (const char* c = ctx->message; (u32)(c - ctx->message) < ctx->current_character; c++) {
			if (*c == '\n') {
				lines++;
			}
		}

		i32 y = win_h - h - (back.dimentions.y / 2) - ((lines * font_height(ctx->font)) / 2);

		for (const char* c = ctx->message; (u32)(c - ctx->message) < ctx->current_character; c++) {
			line_len++;

			if (*c == '\n') {
				render_text_fancy(ctx->renderer, ctx->font, line,
					line_len, 20, y, make_color(0xffffff, 255), &coin_quad);

				y += font_height(ctx->font);

				line_len = 0;
				line = c + 1;
			}
		}

		render_text_fancy(ctx->renderer, ctx->font, line,
			line_len, 20, y, make_color(0xffffff, 255), &coin_quad);

		if (ctx->on_submit && ctx->current_character >= ctx->message_len) {
			const char* text = "Yes / No";
			const char* yes_text = "Yes";
			const char* no_text = "No";
			const char* mid_text = " / ";

			i32 w = text_width(ctx->font, text);

			struct textured_quad back2 = {
				.texture = null,
				.position = { win_w - w + 5 * 2 - 40, win_h - 170 },
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
				ctx->nullify = true;
				
				if (ctx->selected) {
					play_audio_clip(ctx->select_sound);
				} else {
					play_audio_clip(ctx->decline_sound);
				}

				ctx->on_submit(ctx->selected, ctx->udata);	

				if (ctx->nullify) {
					if (ctx->message) {
						core_free(ctx->message);
					}
					ctx->message = null;
					ctx->on_submit = null;
					logic_store->frozen = false;
				}
			}
		}

		if (!ctx->on_submit && ctx->current_character >= ctx->message_len - 1 && 
				(key_just_pressed(main_window, mapped_key("submit")) || key_just_pressed(main_window, mapped_key("jump")))) {

			if (ctx->on_finish) {
				ctx->on_finish(ctx->udata);
			}

			if (ctx->message) {
				core_free(ctx->message);
			}
			ctx->message = null;
			ctx->on_finish = null;
			logic_store->frozen = false;
		}
	}

	if (key_just_pressed(main_window, mapped_key("jump"))) {
		ctx->current_character = ctx->message_len - 1;
		return;
	}

	renderer_flush(ctx->renderer);
}
