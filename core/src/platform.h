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
	key_unknown = 0,
	key_space,
	key_apostrophe,
	key_comma,
	key_minus,
	key_period,
	key_slash,
	key_0,
	key_1,
	key_2,
	key_3,
	key_4,
	key_5,
	key_6,
	key_7,
	key_8,
	key_9,
	key_semicolon,
	key_equal,
	key_A,
	key_B,
	key_C,
	key_D,
	key_E,
	key_F,
	key_G,
	key_H,
	key_I,
	key_J,
	key_K,
	key_L,
	key_M,
	key_N,
	key_O,
	key_P,
	key_Q,
	key_R,
	key_S,
	key_T,
	key_U,
	key_V,
	key_W,
	key_X,
	key_Y,
	key_Z,
	key_backslash,
	key_grave_accent,
	key_escape,
	key_return,
	key_tab,
	key_backspace,
	key_insert,
	key_delete,
	key_right,
	key_left,
	key_down,
	key_up,
	key_page_up,
	key_page_down,
	key_home,
	key_end,
	key_f1,
	key_f2,
	key_f3,
	key_f4,
	key_f5,
	key_f6,
	key_f7,
	key_f8,
	key_f9,
	key_f10,
	key_f11,
	key_f12,
	key_shift,
	key_control,
	key_alt,
	key_super,
	key_menu,

	/* A little extra space is allocated
	 * for keys that may have multiple binds */
	key_count = key_menu + 32
};

enum {
	mouse_btn_left = 0,
	mouse_btn_middle,
	mouse_btn_right,

	mouse_btn_count
};

enum {
	cursor_pointer = 0,
	cursor_hand,
	cursor_resize,
	cursor_move
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
