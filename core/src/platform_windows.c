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

	HWND hwnd;
	HDC device_context;
	HGLRC render_context;

	struct key_table keymap;

	v2i mouse_pos;

	bool held_keys[KEY_COUNT];
	bool pressed_keys[KEY_COUNT];
	bool released_keys[KEY_COUNT];

	bool held_btns[MOUSE_BTN_COUNT];
	bool pressed_btns[MOUSE_BTN_COUNT];
	bool released_btns[MOUSE_BTN_COUNT];

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
		return 0;
	}
	case WM_LBUTTONDOWN: {
		window->held_btns[MOUSE_BTN_LEFT] = true;
		window->pressed_btns[MOUSE_BTN_LEFT] = true;
		return 0;
	}
	case WM_LBUTTONUP: {
		window->held_btns[MOUSE_BTN_LEFT] = false;
		window->released_btns[MOUSE_BTN_LEFT] = true;
		return 0;
	}
	case WM_MBUTTONDOWN: {
		window->held_btns[MOUSE_BTN_MIDDLE] = true;
		window->pressed_btns[MOUSE_BTN_MIDDLE] = true;
		return 0;
	}
	case WM_MBUTTONUP: {
		window->held_btns[MOUSE_BTN_MIDDLE] = false;
		window->released_btns[MOUSE_BTN_MIDDLE] = true;
		return 0;
	}
	case WM_RBUTTONDOWN: {
		window->held_btns[MOUSE_BTN_RIGHT] = true;
		window->pressed_btns[MOUSE_BTN_RIGHT] = true;
		return 0;
	}
	case WM_RBUTTONUP: {
		window->held_btns[MOUSE_BTN_RIGHT] = false;
		window->released_btns[MOUSE_BTN_RIGHT] = true;
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
	key_table_insert(&window->keymap, 0x00, KEY_UNKNOWN);
	key_table_insert(&window->keymap, 0x41, KEY_A);
	key_table_insert(&window->keymap, 0x42, KEY_B);
	key_table_insert(&window->keymap, 0x43, KEY_C);
	key_table_insert(&window->keymap, 0x44, KEY_D);
	key_table_insert(&window->keymap, 0x45, KEY_E);
	key_table_insert(&window->keymap, 0x46, KEY_F);
	key_table_insert(&window->keymap, 0x47, KEY_G);
	key_table_insert(&window->keymap, 0x48, KEY_H);
	key_table_insert(&window->keymap, 0x49, KEY_I);
	key_table_insert(&window->keymap, 0x4A, KEY_J);
	key_table_insert(&window->keymap, 0x4B, KEY_K);
	key_table_insert(&window->keymap, 0x4C, KEY_L);
	key_table_insert(&window->keymap, 0x4D, KEY_M);
	key_table_insert(&window->keymap, 0x4E, KEY_N);
	key_table_insert(&window->keymap, 0x4F, KEY_O);
	key_table_insert(&window->keymap, 0x50, KEY_P);
	key_table_insert(&window->keymap, 0x51, KEY_Q);
	key_table_insert(&window->keymap, 0x52, KEY_R);
	key_table_insert(&window->keymap, 0x53, KEY_S);
	key_table_insert(&window->keymap, 0x54, KEY_T);
	key_table_insert(&window->keymap, 0x55, KEY_U);
	key_table_insert(&window->keymap, 0x56, KEY_V);
	key_table_insert(&window->keymap, 0x57, KEY_W);
	key_table_insert(&window->keymap, 0x58, KEY_X);
	key_table_insert(&window->keymap, 0x59, KEY_Y);
	key_table_insert(&window->keymap, 0x5A, KEY_Z);
	key_table_insert(&window->keymap, VK_F1, KEY_F1);
	key_table_insert(&window->keymap, VK_F2, KEY_F2);
	key_table_insert(&window->keymap, VK_F3, KEY_F3);
	key_table_insert(&window->keymap, VK_F4, KEY_F4);
	key_table_insert(&window->keymap, VK_F5, KEY_F5);
	key_table_insert(&window->keymap, VK_F6, KEY_F6);
	key_table_insert(&window->keymap, VK_F7, KEY_F7);
	key_table_insert(&window->keymap, VK_F8, KEY_F8);
	key_table_insert(&window->keymap, VK_F9, KEY_F9);
	key_table_insert(&window->keymap, VK_F10, KEY_F10);
	key_table_insert(&window->keymap, VK_F11, KEY_F11);
	key_table_insert(&window->keymap, VK_F12, KEY_F12);
	key_table_insert(&window->keymap, VK_DOWN, KEY_DOWN);
	key_table_insert(&window->keymap, VK_LEFT, KEY_LEFT);
	key_table_insert(&window->keymap, VK_RIGHT, KEY_RIGHT);
	key_table_insert(&window->keymap, VK_UP, KEY_UP);
	key_table_insert(&window->keymap, VK_ESCAPE, KEY_ESCAPE);
	key_table_insert(&window->keymap, VK_RETURN, KEY_RETURN);
	key_table_insert(&window->keymap, VK_BACK, KEY_BACKSPACE);
	key_table_insert(&window->keymap, VK_RETURN, KEY_RETURN);
	key_table_insert(&window->keymap, VK_TAB, KEY_TAB);
	key_table_insert(&window->keymap, VK_DELETE, KEY_DELETE);
	key_table_insert(&window->keymap, VK_HOME, KEY_HOME);
	key_table_insert(&window->keymap, VK_END, KEY_END);
	key_table_insert(&window->keymap, VK_PRIOR, KEY_PAGE_UP);
	key_table_insert(&window->keymap, VK_NEXT, KEY_PAGE_DOWN);
	key_table_insert(&window->keymap, VK_INSERT, KEY_INSERT);
	key_table_insert(&window->keymap, VK_LSHIFT, KEY_SHIFT);
	key_table_insert(&window->keymap, VK_RSHIFT, KEY_SHIFT);
	key_table_insert(&window->keymap, VK_LCONTROL, KEY_CONTROL);
	key_table_insert(&window->keymap, VK_RCONTROL, KEY_CONTROL);
	key_table_insert(&window->keymap, VK_LWIN, KEY_SUPER);
	key_table_insert(&window->keymap, VK_RWIN, KEY_SUPER);
	key_table_insert(&window->keymap, VK_MENU, KEY_ALT);
	key_table_insert(&window->keymap, VK_SPACE, KEY_SPACE);
	key_table_insert(&window->keymap, VK_OEM_PERIOD, KEY_PERIOD);
	key_table_insert(&window->keymap, 0x30, KEY_0);
	key_table_insert(&window->keymap, 0x31, KEY_1);
	key_table_insert(&window->keymap, 0x32, KEY_2);
	key_table_insert(&window->keymap, 0x33, KEY_3);
	key_table_insert(&window->keymap, 0x34, KEY_4);
	key_table_insert(&window->keymap, 0x35, KEY_5);
	key_table_insert(&window->keymap, 0x36, KEY_6);
	key_table_insert(&window->keymap, 0x37, KEY_7);
	key_table_insert(&window->keymap, 0x38, KEY_8);
	key_table_insert(&window->keymap, 0x39, KEY_9);

	window->mouse_pos = make_v2i(0, 0);

	memset(window->held_keys, 0, KEY_COUNT * sizeof(bool));
	memset(window->pressed_keys, 0, KEY_COUNT * sizeof(bool));
	memset(window->released_keys, 0, KEY_COUNT * sizeof(bool));

	memset(window->held_btns, 0, MOUSE_BTN_COUNT * sizeof(bool));
	memset(window->pressed_btns, 0, MOUSE_BTN_COUNT * sizeof(bool));
	memset(window->released_btns, 0, MOUSE_BTN_COUNT * sizeof(bool));

	return window;
}

void free_window(struct window* window) {
	PostQuitMessage(0);
	DestroyWindow(window->hwnd);
	wglDeleteContext(window->render_context);

	core_free(window);
}

void swap_window(struct window* window) {
	memset(window->pressed_keys, 0, KEY_COUNT * sizeof(bool));
	memset(window->released_keys, 0, KEY_COUNT * sizeof(bool));
	memset(window->pressed_btns, 0, MOUSE_BTN_COUNT * sizeof(bool));
	memset(window->released_btns, 0, MOUSE_BTN_COUNT * sizeof(bool));

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
		case CURSOR_HAND:
			c = LoadCursor(null, IDC_HAND);
			break;
		case CURSOR_MOVE:
			c = LoadCursor(null, IDC_SIZEALL);
			break;
		case CURSOR_RESIZE:
			c = LoadCursor(null, IDC_SIZENESW);
			break;
		case CURSOR_POINTER:
		default:
			c = LoadCursor(null, IDC_ARROW);
			break;
	}

	SetCursor(c);
}

void window_enable_repeat(struct window* window, bool enable) {
	window->repeat = enable;
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
