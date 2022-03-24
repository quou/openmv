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

API struct window* new_window(v2i size, const char* title, bool resizable);
API void free_window(struct window* window);
API void swap_window(struct window* window);
API void update_events(struct window* window);
API void query_window(struct window* window, i32* width, i32* height);
API bool window_should_close(struct window* window);
API void set_window_should_close(struct window* window, bool close);
API void window_make_context_current(struct window* window);

API void set_window_size(struct window* window, v2i size);
API void set_window_fullscreen(struct window* window, bool fullscreen);

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

API u32 get_window_cursor(struct window* window);
API void set_window_cursor(struct window* window, u32 id);

API void window_enable_repeat(struct window* window, bool enable);

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

enum {
	CURSOR_POINTER = 0,
	CURSOR_HAND,
	CURSOR_RESIZE,
	CURSOR_MOVE
};

/* File system */
struct dir_iter;
struct dir_entry {
	char name[1024];

	struct dir_iter* iter;
};

API const char* get_root_dir();

API struct dir_iter* new_dir_iter(const char* dir_name);
API void free_dir_iter(struct dir_iter* it);
API struct dir_entry* dir_iter_cur(struct dir_iter* it);
API bool dir_iter_next(struct dir_iter* it);

API bool file_exists(const char* name);
API bool file_is_regular(const char* name);
API bool file_is_dir(const char* name);
API u64 file_mod_time(const char* name);

API const char* get_file_name(const char* path);
API const char* get_file_extension(const char* name);
API char* get_file_path(const char* name);

/* Multi-threading */

struct thread;

typedef void (*thread_worker)(struct thread* thread);

API struct thread* new_thread(thread_worker worker);
API void free_thread(struct thread* thread);
API void thread_execute(struct thread* thread);
API void thread_join(struct thread* thread);
API bool thread_active(struct thread* thread);
API void* get_thread_uptr(struct thread* thread);
API void  set_thread_uptr(struct thread* thread, void* ptr);

struct mutex;

API struct mutex* new_mutex(u64 size);
API void free_mutex(struct mutex* mutex);
API void lock_mutex(struct mutex* mutex);
API void unlock_mutex(struct mutex* mutex);
API void* mutex_get_ptr(struct mutex* mutex);
