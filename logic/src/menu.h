#pragma once

#include "common.h"
#include "video.h"

struct menu;

typedef void (*menu_on_select)(struct menu* menu);

struct menu* new_menu(struct font* font);
void free_menu(struct menu* menu);

void menu_update(struct menu* menu);
void menu_reset_selection(struct menu* menu);

void menu_add_selectable(struct menu* menu, const char* label, menu_on_select on_select);
void menu_add_label(struct menu* menu, const char* label);

typedef void (*prompt_submit_func)(bool yes, void*);
typedef void (*prompt_finish_func)(void*);

void prompts_init(struct font* font);
void prompts_deinit();
void message_prompt(const char* text);
void message_prompt_ex(const char* text, prompt_finish_func on_finish, void* udata);
void prompt_ask(const char* text, prompt_submit_func on_submit, void* udata);
void prompts_update(f64 ts);
