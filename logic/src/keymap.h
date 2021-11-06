#pragma once

void keymap_init();
void keymap_deinit();
void default_keymap();
void save_keymap();
void load_keymap();

i32 mapped_key(const char* name);
