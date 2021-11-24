#pragma once

#include "common.h"
#include "video.h"

struct menu;

typedef void (*menu_on_select)(struct menu* menu);

struct menu* new_menu(struct shader shader, struct font* font);
void free_menu(struct menu* menu);

void menu_update(struct menu* menu);
void menu_reset_selection(struct menu* menu);

void menu_add_selectable(struct menu* menu, const char* label, menu_on_select on_select);
void menu_add_label(struct menu* menu, const char* label);
