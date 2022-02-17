#include "consts.h"
#include "core.h"
#include "keymap.h"
#include "logic_store.h"
#include "menu.h"
#include "platform.h"
#include "shop.h"
#include "sprites.h"

struct {
	bool shopping;

	struct font* big_font;
	struct font* font;
} shop;

void shopping() {
	logic_store->frozen = true;
	shop.shopping = true;
}

void shops_init() {
	shop.shopping = false;
	shop.big_font = load_font("res/CourierPrime.ttf", 35.0f);
	shop.font = load_font("res/CourierPrime.ttf", 25.0f);
}

void shops_deinit() {
	shop.shopping = false;
}

void shops_update(f64 ts) {
	if (shop.shopping) {

	}
}
