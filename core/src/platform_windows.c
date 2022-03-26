#include <stdio.h>

#include <windows.h>
#include "util/glad.h"

#include "common.h"
#include "core.h"
#include "platform.h"
#include "keytable.h"

u64 global_freq;
bool global_has_pc;

void init_time() {
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&global_freq)) {
		global_has_pc = true;
	}
	else {
		global_freq = 1000;
		global_has_pc = false;
	}
}

u64 get_frequency() {
	return global_freq;
}

u64 get_time() {
	u64 now;

	if (global_has_pc) {
		QueryPerformanceCounter((LARGE_INTEGER*)&now);
	}
	else {
		now = (u64)timeGetTime();
	}

	return now;
}

struct window {
	i32 w, h;

	bool open;
	bool resizable;
	bool repeat;
	bool mouse_locked;

	HWND hwnd;
	HDC device_context;
	HGLRC render_context;

	struct key_table keymap;

	v2i mouse_pos;
	v2i mouse_delta;
	v2i last_mouse;

	bool held_keys[key_count];
	bool pressed_keys[key_count];
	bool released_keys[key_count];

	bool held_btns[mouse_btn_count];
	bool pressed_btns[mouse_btn_count];
	bool released_btns[mouse_btn_count];

	u32 cursor;

	i32 scroll;

	void* uptr;

	on_text_input_func on_text_input;
};

static LRESULT CALLBACK win32_event_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	LONG_PTR user_data = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	struct window* window = (struct window*)user_data;

	switch (msg) {
	case WM_DESTROY:
		window->open = false;
		return 0;
	case WM_SIZE: {
		if (!window) { break; }
		i32 nw = lparam & 0xFFFF;
		i32 nh = (lparam >> 16) & 0xFFFF;

		glViewport(0, 0, nw, nh);

		window->w = nw;
		window->h = nh;
		return 0;
	};
	case WM_KEYDOWN: {
		if (!window->repeat && ((29llu << 30llu) & lparam) != 0) { /* Ignore repeat */
			return 0;
		}
		i32 key = search_key_table(&window->keymap, (i32)wparam);
		window->held_keys[key] = true;
		window->pressed_keys[key] = true;

		if (window->on_text_input) {
			char state[256];
			char buf[256];
			GetKeyboardState(state);

			unsigned int scan_code = (lparam >> 16) & 0xFF;
			int i = ToAscii((u32)wparam, scan_code, state, (LPWORD)buf, 0);
			buf[i] = 0;

			u32 len = (u32)strlen(buf);
			for (u32 i = 0; i < len; i++) {
				if (buf[i] < ' ' || buf[i] > '~') {
					buf[i] = '\0';
				}
			}
			len = (u32)strlen(buf);
			if (len > 0) {
				window->on_text_input(window, buf, window->uptr);
			}
		}

		return 0;
	}
	case WM_KEYUP: {
		i32 key = search_key_table(&window->keymap, (i32)wparam);
		window->held_keys[key] = false;
		window->released_keys[key] = true;

		return 0;
	}
	case WM_MOUSEMOVE: {
		u32 x = lparam & 0xFFFF;
		u32 y = (lparam >> 16) & 0xFFFF;

		window->mouse_pos = make_v2i((i32)x, (i32)y);
		window->mouse_delta = v2i_sub(window->last_mouse, window->mouse_pos);
		window->last_mouse = window->mouse_pos;
		return 0;
	}
	case WM_LBUTTONDOWN: {
		window->held_btns[mouse_btn_left] = true;
		window->pressed_btns[mouse_btn_left] = true;
		return 0;
	}
	case WM_LBUTTONUP: {
		window->held_btns[mouse_btn_left] = false;
		window->released_btns[mouse_btn_left] = true;
		return 0;
	}
	case WM_MBUTTONDOWN: {
		window->held_btns[mouse_btn_middle] = true;
		window->pressed_btns[mouse_btn_middle] = true;
		return 0;
	}
	case WM_MBUTTONUP: {
		window->held_btns[mouse_btn_middle] = false;
		window->released_btns[mouse_btn_middle] = true;
		return 0;
	}
	case WM_RBUTTONDOWN: {
		window->held_btns[mouse_btn_right] = true;
		window->pressed_btns[mouse_btn_right] = true;
		return 0;
	}
	case WM_RBUTTONUP: {
		window->held_btns[mouse_btn_right] = false;
		window->released_btns[mouse_btn_right] = true;
		return 0;
	}
	case WM_MOUSEWHEEL: {
		window->scroll += GET_WHEEL_DELTA_WPARAM(wparam) > 1 ? 1 : -1;
		return 0;
	}
	default: break;
	};

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

struct window* new_window(v2i size, const char* title, bool resizable) {
	struct window* window = core_calloc(1, sizeof(struct window));

	window->uptr = null;
	window->open = false;

	WNDCLASSW wc = { 0 };
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = win32_event_callback;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpszMenuName = NULL;
	wc.hbrBackground = NULL;
	wc.lpszClassName = L"openmv";
	RegisterClassW(&wc);

	DWORD dw_ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dw_style = WS_CAPTION | WS_SYSMENU |
		WS_MINIMIZEBOX | WS_VISIBLE;

	window->resizable = resizable;
	if (resizable) {
		dw_style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
	}

	window->w = size.x;
	window->h = size.y;

	RECT win_rect = { 0, 0, size.x, size.y };
	AdjustWindowRectEx(&win_rect, dw_style, FALSE, dw_ex_style);
	i32 create_width = win_rect.right - win_rect.left;
	i32 create_height = win_rect.bottom - win_rect.top;

	window->hwnd = CreateWindowExA(dw_ex_style, "openmv", "", dw_style, 0, 0,
		create_width, create_height, null, null, GetModuleHandle(null), window);
	SetWindowLongPtr(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);
	SetWindowTextA(window->hwnd, title);

	window->device_context = GetDC(window->hwnd);

	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nVersion = 1;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	i32 pf;
	if (!(pf = ChoosePixelFormat(window->device_context, &pfd))) {
		printf("Error choosing pixel format\n");
		core_free(window);
		return 0;
	}
	SetPixelFormat(window->device_context, pf, &pfd);

	if (!(window->render_context = wglCreateContext(window->device_context))) {
		fprintf(stderr, "Failed to create OpenGL context.\n");
		core_free(window);
		return 0;
	}

	wglMakeCurrent(window->device_context, window->render_context);

	window->open = true;

	init_key_table(&window->keymap);
	key_table_insert(&window->keymap, 0x00, key_unknown);
	key_table_insert(&window->keymap, 0x41, key_A);
	key_table_insert(&window->keymap, 0x42, key_B);
	key_table_insert(&window->keymap, 0x43, key_C);
	key_table_insert(&window->keymap, 0x44, key_D);
	key_table_insert(&window->keymap, 0x45, key_E);
	key_table_insert(&window->keymap, 0x46, key_F);
	key_table_insert(&window->keymap, 0x47, key_G);
	key_table_insert(&window->keymap, 0x48, key_H);
	key_table_insert(&window->keymap, 0x49, key_I);
	key_table_insert(&window->keymap, 0x4A, key_J);
	key_table_insert(&window->keymap, 0x4B, key_K);
	key_table_insert(&window->keymap, 0x4C, key_L);
	key_table_insert(&window->keymap, 0x4D, key_M);
	key_table_insert(&window->keymap, 0x4E, key_N);
	key_table_insert(&window->keymap, 0x4F, key_O);
	key_table_insert(&window->keymap, 0x50, key_P);
	key_table_insert(&window->keymap, 0x51, key_Q);
	key_table_insert(&window->keymap, 0x52, key_R);
	key_table_insert(&window->keymap, 0x53, key_S);
	key_table_insert(&window->keymap, 0x54, key_T);
	key_table_insert(&window->keymap, 0x55, key_U);
	key_table_insert(&window->keymap, 0x56, key_V);
	key_table_insert(&window->keymap, 0x57, key_W);
	key_table_insert(&window->keymap, 0x58, key_X);
	key_table_insert(&window->keymap, 0x59, key_Y);
	key_table_insert(&window->keymap, 0x5A, key_Z);
	key_table_insert(&window->keymap, VK_F1, key_f1);
	key_table_insert(&window->keymap, VK_F2, key_f2);
	key_table_insert(&window->keymap, VK_F3, key_f3);
	key_table_insert(&window->keymap, VK_F4, key_f4);
	key_table_insert(&window->keymap, VK_F5, key_f5);
	key_table_insert(&window->keymap, VK_F6, key_f6);
	key_table_insert(&window->keymap, VK_F7, key_f7);
	key_table_insert(&window->keymap, VK_F8, key_f8);
	key_table_insert(&window->keymap, VK_F9, key_f9);
	key_table_insert(&window->keymap, VK_F10, key_f10);
	key_table_insert(&window->keymap, VK_F11, key_f11);
	key_table_insert(&window->keymap, VK_F12, key_f12);
	key_table_insert(&window->keymap, VK_DOWN, key_down);
	key_table_insert(&window->keymap, VK_LEFT, key_left);
	key_table_insert(&window->keymap, VK_RIGHT, key_right);
	key_table_insert(&window->keymap, VK_UP, key_up);
	key_table_insert(&window->keymap, VK_ESCAPE, key_escape);
	key_table_insert(&window->keymap, VK_RETURN, key_return);
	key_table_insert(&window->keymap, VK_BACK, key_backspace);
	key_table_insert(&window->keymap, VK_RETURN, key_return);
	key_table_insert(&window->keymap, VK_TAB, key_tab);
	key_table_insert(&window->keymap, VK_DELETE, key_delete);
	key_table_insert(&window->keymap, VK_HOME, key_home);
	key_table_insert(&window->keymap, VK_END, key_end);
	key_table_insert(&window->keymap, VK_PRIOR, key_page_up);
	key_table_insert(&window->keymap, VK_NEXT, key_page_down);
	key_table_insert(&window->keymap, VK_INSERT, key_insert);
	key_table_insert(&window->keymap, VK_LSHIFT, key_shift);
	key_table_insert(&window->keymap, VK_RSHIFT, key_shift);
	key_table_insert(&window->keymap, VK_LCONTROL, key_control);
	key_table_insert(&window->keymap, VK_RCONTROL, key_control);
	key_table_insert(&window->keymap, VK_LWIN, key_super);
	key_table_insert(&window->keymap, VK_RWIN, key_super);
	key_table_insert(&window->keymap, VK_MENU, key_alt);
	key_table_insert(&window->keymap, VK_SPACE, key_space);
	key_table_insert(&window->keymap, VK_OEM_PERIOD, key_period);
	key_table_insert(&window->keymap, 0x30, key_0);
	key_table_insert(&window->keymap, 0x31, key_1);
	key_table_insert(&window->keymap, 0x32, key_2);
	key_table_insert(&window->keymap, 0x33, key_3);
	key_table_insert(&window->keymap, 0x34, key_4);
	key_table_insert(&window->keymap, 0x35, key_5);
	key_table_insert(&window->keymap, 0x36, key_6);
	key_table_insert(&window->keymap, 0x37, key_7);
	key_table_insert(&window->keymap, 0x38, key_8);
	key_table_insert(&window->keymap, 0x39, key_9);

	window->mouse_pos = make_v2i(0, 0);

	memset(window->held_keys, 0, key_count * sizeof(bool));
	memset(window->pressed_keys, 0, key_count * sizeof(bool));
	memset(window->released_keys, 0, key_count * sizeof(bool));

	memset(window->held_btns, 0, mouse_btn_count * sizeof(bool));
	memset(window->pressed_btns, 0, mouse_btn_count * sizeof(bool));
	memset(window->released_btns, 0, mouse_btn_count * sizeof(bool));

	return window;
}

void free_window(struct window* window) {
	PostQuitMessage(0);
	DestroyWindow(window->hwnd);
	wglDeleteContext(window->render_context);

	core_free(window);
}

void swap_window(struct window* window) {
	memset(window->pressed_keys, 0, key_count * sizeof(bool));
	memset(window->released_keys, 0, key_count * sizeof(bool));
	memset(window->pressed_btns, 0, mouse_btn_count * sizeof(bool));
	memset(window->released_btns, 0, mouse_btn_count * sizeof(bool));

	SwapBuffers(window->device_context);
}

void window_make_context_current(struct window* window) {
	wglMakeCurrent(window->device_context, window->render_context);
}

void update_events(struct window* window) {
	window->scroll = 0;

	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (window->mouse_locked) {
		i32 w, h;
		query_window(window, &w, &h);

		SetCursorPos(w / 2, h / 2);
	}
}

void query_window(struct window* window, i32* width, i32* height) {
	if (width) {
		*width = window->w;
	}

	if (height) {
		*height = window->h;
	}

}

bool window_should_close(struct window* window) {
	return !window->open;
}

bool key_pressed(struct window* window, i32 key) {
	return window->held_keys[key];
}

bool key_just_pressed(struct window* window, i32 key) {
	return window->pressed_keys[key];
}

bool key_just_released(struct window* window, i32 key) {
	return window->released_keys[key];
}

bool mouse_btn_pressed(struct window* window, i32 btn) {
	return window->held_btns[btn];
}

bool mouse_btn_just_pressed(struct window* window, i32 btn) {
	return window->pressed_btns[btn];
}

bool mouse_btn_just_released(struct window* window, i32 btn) {
	return window->released_btns[btn];
}

v2i get_mouse_position(struct window* window) {
	return window->mouse_pos;
}

v2i get_mouse_delta(struct window* window) {
	return window->mouse_delta;
}

void set_on_text_input(struct window* window, on_text_input_func func) {
	window->on_text_input = func;
}

void set_window_uptr(struct window* window, void* uptr) {
	window->uptr = uptr;
}

i32 get_scroll(struct window* window) {
	return window->scroll;
}

void set_window_should_close(struct window* window, bool close) {
	window->open = !close;
}

void set_window_size(struct window* window, v2i size) {
	SetWindowPos(window->hwnd, 0, 0, 0, size.x, size.y, SWP_NOMOVE);
}

void set_window_fullscreen(struct window* window, bool fullscreen) {
	if (fullscreen) {
		POINT Point = { 0 };
		HMONITOR Monitor = MonitorFromPoint(Point, MONITOR_DEFAULTTONEAREST);
		MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
		if (GetMonitorInfo(Monitor, &MonitorInfo)) {
			DWORD Style = WS_POPUP | WS_VISIBLE;
			SetWindowLongPtr(window->hwnd, GWL_STYLE, Style);
			SetWindowPos(window->hwnd, 0, MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
				MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
				MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		}
	}
	else {
		DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;

		if (window->resizable) {
			style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		}

		SetWindowLongPtr(window->hwnd, GWL_STYLE, style);
		SetWindowPos(window->hwnd, 0,
			0, 0, window->w, window->h,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	}
}

const char* get_root_dir() {
	return "C:/";
}

struct dir_iter {
	char root[1024];
	char original[1024];

	HANDLE find;

	u32 i;

	struct dir_entry entry;
};

struct dir_iter* new_dir_iter(const char* dir_name) {
	if (!file_is_dir(dir_name)) {
		fprintf(stderr, "Failed to open directory: `%s'.\n", dir_name);
		return null;
	}

	struct dir_iter* it = core_calloc(1, sizeof(struct dir_iter));

	u32 len = (u32)strlen(dir_name);
	strcpy(it->root, dir_name);
	strcpy(it->original, dir_name);

	if (it->original[len - 1] != '/') {
		strcat(it->original, "/");
	}

	for (u32 i = 0; i < len; i++) {
		if (it->root[i] == '/') {
			it->root[i] = '\\';
		}
	}

	if (it->root[len - 1] != '\\') {
		strcat(it->root, "\\");
	}

	strcat(it->root, "*");

	dir_iter_next(it);

	return it;
}

void free_dir_iter(struct dir_iter* it) {
	FindClose(it->find);
}

struct dir_entry* dir_iter_cur(struct dir_iter* it) {
	return &it->entry;
}

bool dir_iter_next(struct dir_iter* it) {
	WIN32_FIND_DATAA data = { 0 };

	if (it->i == 0) {
		it->find = FindFirstFileA(it->root, &data);

		if (it->find == INVALID_HANDLE_VALUE) { return false; }
	}
	else {
		if (!FindNextFileA(it->find, &data)) { return false; }
	}

	it->i++;

	if (strcmp(data.cFileName, ".") == 0) { return dir_iter_next(it); }
	if (strcmp(data.cFileName, "..") == 0) { return dir_iter_next(it); }

	strcpy(it->entry.name, it->original);
	strcat(it->entry.name, data.cFileName);

	return true;
}

bool file_exists(const char* name) {
	DWORD attribs = GetFileAttributesA(name);

	return attribs != INVALID_FILE_ATTRIBUTES;
}

bool file_is_regular(const char* name) {
	return !file_is_dir(name);
}

bool file_is_dir(const char* name) {
	DWORD attribs = GetFileAttributesA(name);

	return attribs & FILE_ATTRIBUTE_DIRECTORY;
}

u64 file_mod_time(const char* name) {
	HANDLE file = CreateFileA(name, GENERIC_READ, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);

	if (file == INVALID_HANDLE_VALUE) {
		return 0;
	}

	FILETIME ft;

	if (!GetFileTime(file, null, null, &ft)) {
		return 0;
	}

	CloseHandle(file);

	LARGE_INTEGER date, adjust;
	date.HighPart = ft.dwHighDateTime;
	date.LowPart = ft.dwLowDateTime;

	adjust.QuadPart = 11644473600000 * 10000;

	date.QuadPart -= adjust.QuadPart;

	return date.QuadPart / 10000000;
}

u32 get_window_cursor(struct window* window) {
	return window->cursor;
}

void set_window_cursor(struct window* window, u32 id) {
	window->cursor = id;

	HCURSOR c;

	switch (id) {
	case cursor_hand:
		c = LoadCursor(null, IDC_HAND);
		break;
	case cursor_move:
		c = LoadCursor(null, IDC_SIZEALL);
		break;
	case cursor_resize:
		c = LoadCursor(null, IDC_SIZENESW);
		break;
	case cursor_pointer:
	default:
		c = LoadCursor(null, IDC_ARROW);
		break;
	}

	SetCursor(c);
}

void window_enable_repeat(struct window* window, bool enable) {
	window->repeat = enable;
}

void lock_mouse(struct window* window) {
	window->mouse_locked = true;
	ShowCursor(false);
}

void unlock_mouse(struct window* window) {
	window->mouse_locked = false;
	ShowCursor(true);
}

bool is_mouse_locked(struct window* window) {
	return window->mouse_locked;
}

char* get_file_path(const char* name) {
	char* r = core_alloc(256);

	if (!GetFullPathNameA(name, 256, r, null)) {
		free(r);
		return null;
	}

	u32 len = (u32)strlen(r);

	char* cut = r + len;
	while (cut > r && *cut != '\\') {
		*cut = '\0';
		cut--;
	}

	return r;
}

struct thread {
	HANDLE handle;
	DWORD id;
	void* uptr;
	bool working;
	thread_worker worker;
};

DWORD WINAPI _thread_worker(LPVOID lparam) {
	struct thread* thread = (struct thread*)lparam;

	thread->working = true;

	thread->worker(thread);

	thread->working = false;

	return 0;
}

struct thread* new_thread(thread_worker worker) {
	struct thread* thread = core_calloc(1, sizeof(struct thread));

	thread->worker = worker;

	return thread;
}

void free_thread(struct thread* thread) {
	thread_join(thread);
	core_free(thread);
}

void thread_execute(struct thread* thread) {
	if (thread->working) {
		fprintf(stderr, "Warning: Thread already active!\n");
		return;
	}

	thread->handle = CreateThread(null, 0, _thread_worker, thread, 0, &thread->id);
}

void thread_join(struct thread* thread) {
	if (!thread->handle) { return; }

	WaitForSingleObject(thread->handle, INFINITE);
	CloseHandle(thread->handle);
	thread->handle = 0;
}

bool thread_active(struct thread* thread) {
	return thread->working;
}

void* get_thread_uptr(struct thread* thread) {
	return thread->uptr;
}

void set_thread_uptr(struct thread* thread, void* ptr) {
	thread->uptr = ptr;
}

struct mutex {
	HANDLE handle;

	void* data;
};

struct mutex* new_mutex(u64 size) {
	struct mutex* mutex = core_calloc(1, sizeof(struct mutex));

	mutex->handle = CreateMutex(null, false, null);

	if (size > 0) {
		mutex->data = core_calloc(1, size);
	}

	return mutex;
}

void free_mutex(struct mutex* mutex) {
	CloseHandle(mutex->handle);

	if (mutex->data) {
		core_free(mutex->data);
	}

	core_free(mutex);
}

void lock_mutex(struct mutex* mutex) {
	WaitForSingleObject(mutex->handle, INFINITE);
}

void unlock_mutex(struct mutex* mutex) {
	ReleaseMutex(mutex->handle);
}

void* mutex_get_ptr(struct mutex* mutex) {
	return mutex->data;
}