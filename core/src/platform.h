#pragma once

#include "common.h"
#include "maths.h"

/* Global platform. */
API void init_time();
API u64 get_frequency();
API u64 get_time();

/* State-based platform */
struct window;

typedef void (*on_text_input_func)(struct window* window, const char* text, void* udata);

API struct window* new_window(i32 width, i32 height, const char* title);
API void free_window(struct window* window);
API void swap_window(struct window* window);
API void update_events(struct window* window);
API void query_window(struct window* window, i32* width, i32* height);
API bool window_should_close(struct window* window);

API void set_on_text_input(struct window* window, on_text_input_func func);
API void set_window_uptr(struct window* window, void* uptr);

API bool key_pressed(struct window* window, i32 key);
API bool key_just_pressed(struct window* window, i32 key);
API bool key_just_released(struct window* window, i32 key);

API i32 get_scroll(struct window* window);

API bool mouse_btn_pressed(struct window* window, i32 key);
API bool mouse_btn_just_pressed(struct window* window, i32 key);
API bool mouse_btn_just_released(struct window* window, i32 key);
API v2i get_mouse_position(struct window* window);

enum {
	KEY_UNKNOWN = 0,
	KEY_SPACE,
	KEY_APOSTROPHE,
	KEY_COMMA,
	KEY_MINUS,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_SEMICOLON,
	KEY_EQUAL,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_BACKSLASH,
	KEY_GRAVE_ACCENT,
	KEY_ESCAPE,
	KEY_RETURN,
	KEY_TAB,
	KEY_BACKSPACE,
	KEY_INSERT,
	KEY_DELETE,
	KEY_RIGHT,
	KEY_LEFT,
	KEY_DOWN,
	KEY_UP,
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
	KEY_HOME,
	KEY_END,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_SHIFT,
	KEY_CONTROL,
	KEY_ALT,
	KEY_SUPER,
	KEY_MENU,

	/* A little extra space is allocated
	 * for keys that may have multiple binds */
	KEY_COUNT = KEY_MENU + 32
};

enum {
	MOUSE_BTN_LEFT = 0,
	MOUSE_BTN_MIDDLE,
	MOUSE_BTN_RIGHT,

	MOUSE_BTN_COUNT
};
