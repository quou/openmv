#include <stdio.h>

#include <windows.h>
#include "util/gl.h"

#include "common.h"
#include "platform.h"
#include "keytable.h"

u64 global_freq;
bool global_has_pc;

void init_time() {
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&global_freq)) {
		global_has_pc = true;
	} else {
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
	} else {
		now = (u64)timeGetTime();
	}

	return now;
}

struct window {
	i32 w, h;

	bool open;

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

	i32 scroll;

	void* uptr;

	on_text_input_func on_text_input;
};

HDC g_device_context;
HGLRC g_render_context;

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
		case WM_CREATE: {
			g_device_context = GetDC(hwnd);
			
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
			if (!(pf = ChoosePixelFormat(g_device_context, &pfd))) {
				printf("Error choosing pixel format\n");
				core_free(window);
				return 0;
			}
			SetPixelFormat(g_device_context, pf, &pfd);

			if (!(g_render_context = wglCreateContext(g_device_context))) {
				fprintf(stderr, "Failed to create OpenGL context.\n");
				core_free(window);
				return 0;
			}

			wglMakeCurrent(g_device_context, g_render_context);

			if (!gladLoadGL((GLADloadfunc)wglGetProcAddress)) {
				fprintf(stderr, "Failed to load OpenGL.\n");
			}
			return 0;
		}
		default: break;
	};

end:
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

struct window* new_window(i32 width, i32 height, const char* title) {
	struct window* window = core_alloc(sizeof(struct window));

	window->uptr = null;
	window->open = false;

	WNDCLASS wc = { 0 };
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = win32_event_callback;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpszMenuName = NULL;
	wc.hbrBackground = NULL;
	wc.lpszClassName = "openmv";
	RegisterClass(&wc);

	DWORD dw_ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dw_style = WS_CAPTION | WS_SYSMENU |
		WS_MINIMIZEBOX | WS_VISIBLE |
		WS_THICKFRAME | WS_MAXIMIZEBOX;

	RECT win_rect = { 0, 0, width, height };
	AdjustWindowRectEx(&win_rect, dw_style, FALSE, dw_ex_style);
	i32 create_width = win_rect.right - win_rect.left;
	i32 create_height = win_rect.bottom - win_rect.top;

	window->hwnd = CreateWindowEx(dw_ex_style, "openmv", "", dw_style, 0, 0,
		create_width, create_height, null, null, GetModuleHandle(null), window);
	SetWindowLongPtr(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);
	SetWindowTextA(window->hwnd, title);

	window->render_context = g_render_context;
	window->device_context = g_device_context;

	return window;
}

void free_window(struct window* window) {
	PostQuitMessage(0);
	DestroyWindow(window->hwnd);
	wglDeleteContext(window->render_context);

	core_free(window);
}

void swap_window(struct window* window) {
	SwapBuffers(window->device_context);
}

void update_events(struct window* window) {
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
