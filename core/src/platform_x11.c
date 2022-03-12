#include <stdlib.h>
#include <string.h>

#include "util/glad.h"

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xcursor/Xcursor.h>
#include <GL/glx.h>

#include "common.h"
#include "core.h"
#include "platform.h"
#include "keytable.h"

typedef GLXContext (*create_context_func)
	(Display*, GLXFBConfig, GLXContext, Bool, const int*);

struct window {
	i32 w, h;

	bool open;
	bool resizable;

	Display* display;
	Window window;
	XVisualInfo* visual;
	XSetWindowAttributes attribs;
	GLXContext context;
	Colormap colormap;

	struct key_table keymap;

	v2i mouse_pos;

	bool held_keys[KEY_COUNT];
	bool pressed_keys[KEY_COUNT];
	bool released_keys[KEY_COUNT];

	bool held_btns[MOUSE_BTN_COUNT];
	bool pressed_btns[MOUSE_BTN_COUNT];
	bool released_btns[MOUSE_BTN_COUNT];

	i32 scroll;

	u32 cursor;

	bool fullscreen;
	bool repeat;

	void* uptr;

	on_text_input_func on_text_input;
};

struct window* new_window(v2i size, const char* title, bool resizable) {
	struct window* window = core_calloc(1, sizeof(struct window));

	window->resizable = resizable;

	window->uptr = null;
	window->open = false;

	window->display = XOpenDisplay(null);
	Window root = DefaultRootWindow(window->display);

	i32 gl_attribs[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	window->visual = glXChooseVisual(window->display, 0, gl_attribs);
	window->colormap = XCreateColormap(window->display, root, window->visual->visual, AllocNone);
	window->attribs.colormap = window->colormap;

	window->attribs.event_mask =
			ExposureMask        |
			KeyPressMask        |
			KeyReleaseMask      |
			ButtonPressMask     |
			ButtonReleaseMask   |
			PointerMotionMask   |
			StructureNotifyMask;

	window->window = XCreateWindow(window->display, root, 0, 0, size.x, size.y, 0,
			window->visual->depth, InputOutput, window->visual->visual,
			CWColormap | CWEventMask, &window->attribs);

	if (!resizable) {
		/* This works by setting the miniumum and maximum heights of the window
		 * to the input width and height. I'm not sure if this is the correct
		 * way to do it, but it works, and even removes the maximise button */
		XSizeHints* hints = XAllocSizeHints();
		hints->flags = PMinSize | PMaxSize;
		hints->min_width = size.x;
		hints->min_height = size.y;
		hints->max_width = size.x;
		hints->max_height = size.y;

		XSetWMNormalHints(window->display, window->window, hints);

		XFree(hints);
	}

	Atom delete_window = XInternAtom(window->display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(window->display, window->window, &delete_window, 1);

	XStoreName(window->display, window->window, title);

	XClearWindow(window->display, window->window);
	XMapRaised(window->display, window->window);

	/* Create the OpenGL context. */
	int visual_attribs[] = {
		GLX_RENDER_TYPE,   GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_DOUBLEBUFFER,  true,
		GLX_RED_SIZE,      8,
		GLX_GREEN_SIZE,    8,
		GLX_BLUE_SIZE,     8,
		GLX_DEPTH_SIZE,    24,
		GLX_STENCIL_SIZE,  8,
		None
	};

	i32 num_fbc = 0;
	GLXFBConfig* fbc = glXChooseFBConfig(window->display, DefaultScreen(window->display),
			visual_attribs, &num_fbc);
	if (!fbc) {
		abort();
	}

	create_context_func create_context = (create_context_func)
			glXGetProcAddress((const u8*)"glXCreateContextAttribsARB");
	
	if (!create_context) {
		abort();
	}

	i32 context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 3,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	window->context = create_context(window->display, fbc[0], null, true, context_attribs);
	glXMakeCurrent(window->display, window->window, window->context);

	gladLoadGL();

	XFree(fbc);

	void (*swap_interval_ext)(Display*, Window, int)
		= glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT");
	if (swap_interval_ext) {
		swap_interval_ext(window->display, window->window, 0);
	}

	XWindowAttributes gwa;
	XGetWindowAttributes(window->display, window->window, &gwa);
	glViewport(0, 0, gwa.width, gwa.height);
	window->w = gwa.width;
	window->h = gwa.height;

	window->open = true;

	init_key_table(&window->keymap);
	key_table_insert(&window->keymap, 0x00, KEY_UNKNOWN);
	key_table_insert(&window->keymap, 0x61, KEY_A);
	key_table_insert(&window->keymap, 0x62, KEY_B);
	key_table_insert(&window->keymap, 0x63, KEY_C);
	key_table_insert(&window->keymap, 0x64, KEY_D);
	key_table_insert(&window->keymap, 0x65, KEY_E);
	key_table_insert(&window->keymap, 0x66, KEY_F);
	key_table_insert(&window->keymap, 0x67, KEY_G);
	key_table_insert(&window->keymap, 0x68, KEY_H);
	key_table_insert(&window->keymap, 0x69, KEY_I);
	key_table_insert(&window->keymap, 0x6A, KEY_J);
	key_table_insert(&window->keymap, 0x6B, KEY_K);
	key_table_insert(&window->keymap, 0x6C, KEY_L);
	key_table_insert(&window->keymap, 0x6D, KEY_M);
	key_table_insert(&window->keymap, 0x6E, KEY_N);
	key_table_insert(&window->keymap, 0x6F, KEY_O);
	key_table_insert(&window->keymap, 0x70, KEY_P);
	key_table_insert(&window->keymap, 0x71, KEY_Q);
	key_table_insert(&window->keymap, 0x72, KEY_R);
	key_table_insert(&window->keymap, 0x73, KEY_S);
	key_table_insert(&window->keymap, 0x74, KEY_T);
	key_table_insert(&window->keymap, 0x75, KEY_U);
	key_table_insert(&window->keymap, 0x76, KEY_V);
	key_table_insert(&window->keymap, 0x77, KEY_W);
	key_table_insert(&window->keymap, 0x78, KEY_X);
	key_table_insert(&window->keymap, 0x79, KEY_Y);
	key_table_insert(&window->keymap, 0x7A, KEY_Z);
	key_table_insert(&window->keymap, XK_F1, KEY_F1);
	key_table_insert(&window->keymap, XK_F2, KEY_F2);
	key_table_insert(&window->keymap, XK_F3, KEY_F3);
	key_table_insert(&window->keymap, XK_F4, KEY_F4);
	key_table_insert(&window->keymap, XK_F5, KEY_F5);
	key_table_insert(&window->keymap, XK_F6, KEY_F6);
	key_table_insert(&window->keymap, XK_F7, KEY_F7);
	key_table_insert(&window->keymap, XK_F8, KEY_F8);
	key_table_insert(&window->keymap, XK_F9, KEY_F9);
	key_table_insert(&window->keymap, XK_F10, KEY_F10);
	key_table_insert(&window->keymap, XK_F11, KEY_F11);
	key_table_insert(&window->keymap, XK_F12, KEY_F12);
	key_table_insert(&window->keymap, XK_Down, KEY_DOWN);
	key_table_insert(&window->keymap, XK_Left, KEY_LEFT);
	key_table_insert(&window->keymap, XK_Right, KEY_RIGHT);
	key_table_insert(&window->keymap, XK_Up, KEY_UP);
	key_table_insert(&window->keymap, XK_Escape, KEY_ESCAPE);
	key_table_insert(&window->keymap, XK_Return, KEY_RETURN);
	key_table_insert(&window->keymap, XK_BackSpace, KEY_BACKSPACE);
	key_table_insert(&window->keymap, XK_Linefeed, KEY_RETURN);
	key_table_insert(&window->keymap, XK_Tab, KEY_TAB);
	key_table_insert(&window->keymap, XK_Delete, KEY_DELETE);
	key_table_insert(&window->keymap, XK_Home, KEY_HOME);
	key_table_insert(&window->keymap, XK_End, KEY_END);
	key_table_insert(&window->keymap, XK_Page_Up, KEY_PAGE_UP);
	key_table_insert(&window->keymap, XK_Page_Down, KEY_PAGE_DOWN);
	key_table_insert(&window->keymap, XK_Insert, KEY_INSERT);
	key_table_insert(&window->keymap, XK_Shift_L, KEY_SHIFT);
	key_table_insert(&window->keymap, XK_Shift_R, KEY_SHIFT);
	key_table_insert(&window->keymap, XK_Control_L, KEY_CONTROL);
	key_table_insert(&window->keymap, XK_Control_R, KEY_CONTROL);
	key_table_insert(&window->keymap, XK_Super_L, KEY_SUPER);
	key_table_insert(&window->keymap, XK_Super_R, KEY_SUPER);
	key_table_insert(&window->keymap, XK_Alt_L, KEY_ALT);
	key_table_insert(&window->keymap, XK_Alt_R, KEY_ALT);
	key_table_insert(&window->keymap, XK_space, KEY_SPACE);
	key_table_insert(&window->keymap, XK_period, KEY_PERIOD);
	key_table_insert(&window->keymap, XK_0, KEY_0);
	key_table_insert(&window->keymap, XK_1, KEY_1);
	key_table_insert(&window->keymap, XK_2, KEY_2);
	key_table_insert(&window->keymap, XK_3, KEY_3);
	key_table_insert(&window->keymap, XK_4, KEY_4);
	key_table_insert(&window->keymap, XK_5, KEY_5);
	key_table_insert(&window->keymap, XK_6, KEY_6);
	key_table_insert(&window->keymap, XK_7, KEY_7);
	key_table_insert(&window->keymap, XK_8, KEY_8);
	key_table_insert(&window->keymap, XK_9, KEY_9);
	key_table_insert(&window->keymap, XK_apostrophe, KEY_APOSTROPHE);
	key_table_insert(&window->keymap, XK_comma, KEY_COMMA);
	key_table_insert(&window->keymap, XK_minus, KEY_MINUS);
	key_table_insert(&window->keymap, XK_slash, KEY_SLASH);
	key_table_insert(&window->keymap, XK_semicolon, KEY_SEMICOLON);
	key_table_insert(&window->keymap, XK_equal, KEY_EQUAL);
	key_table_insert(&window->keymap, XK_backslash, KEY_BACKSLASH);
	key_table_insert(&window->keymap, XK_grave, KEY_GRAVE_ACCENT);

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
	glXDestroyContext(window->display, window->context);

	XFreeColormap(window->display, window->colormap);

	XFree(window->visual);

	XDestroyWindow(window->display, window->window);
	XCloseDisplay(window->display);

	core_free(window);
}

void swap_window(struct window* window) {
	memset(window->pressed_keys, 0, KEY_COUNT * sizeof(bool));
	memset(window->released_keys, 0, KEY_COUNT * sizeof(bool));
	memset(window->pressed_btns, 0, MOUSE_BTN_COUNT * sizeof(bool));
	memset(window->released_btns, 0, MOUSE_BTN_COUNT * sizeof(bool));

	window->scroll = 0;

	glXSwapBuffers(window->display, window->window);
}

void window_make_context_current(struct window* window) {
	glXMakeCurrent(window->display, window->window, window->context);
}

i32 get_scroll(struct window* window) {
	return window->scroll;
}

void update_events(struct window* window) {
	KeySym sym;

	while (XPending(window->display)) {
		XEvent e;
		XNextEvent(window->display, &e);

		switch (e.type) {
			case ClientMessage:
				window->open = false;
				break;
			case Expose: {
				XWindowAttributes gwa;
				XGetWindowAttributes(window->display, window->window, &gwa);
				glViewport(0, 0, gwa.width, gwa.height);
				window->w = gwa.width;
				window->h = gwa.height;
				break;
			}
			case KeyPress: {
				sym = XLookupKeysym(&e.xkey, 0);
				i32 key = search_key_table(&window->keymap, sym);
				window->held_keys[key] = true;
				window->pressed_keys[key] = true;

				if (window->on_text_input) {
					char buf[64] = "";
					XLookupString(&e.xkey, buf, sizeof(buf), null, null);
					
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

				break;
			}
			case KeyRelease: {
				bool repeat = false;

				if (XEventsQueued(window->display, QueuedAfterReading)) {
					XEvent nev;
					XPeekEvent(window->display, &nev);

					if (nev.type == KeyPress && nev.xkey.time == e.xkey.time && nev.xkey.keycode == e.xkey.keycode) {
						repeat = true;
						XNextEvent(window->display, &e);
					}
				}

				if (window->repeat) { repeat = false; }

				if (!repeat) {
					sym = XLookupKeysym(&e.xkey, 0);
					i32 key = search_key_table(&window->keymap, sym);
					window->held_keys[key] = false;
					window->released_keys[key] = true;
				}
				break;
			}
			case MotionNotify: {
				window->mouse_pos = make_v2i(e.xmotion.x, e.xmotion.y);
				break;
			}
			case ButtonPress: {
				switch (e.xbutton.button) {
					case 1:
					case 2:
					case 3:
						window->held_btns[e.xbutton.button - 1] = true;
						window->pressed_btns[e.xbutton.button - 1] = true;
						break;
					case 4:
						window->scroll++;
						break;
					case 5:
						window->scroll--;
						break;
				}
				break;
			}
			case ButtonRelease: {
				switch (e.xbutton.button) {
					case 1:
					case 2:
					case 3:
						window->held_btns[e.xbutton.button - 1] = false;
						window->released_btns[e.xbutton.button - 1] = true;
						break;
				}
				break;
			}
		}
	}
}

void set_window_size(struct window* window, v2i size) {
	if (!window->resizable) {
		/* This works by setting the miniumum and maximum heights of the window
		 * to the input width and height. I'm not sure if this is the correct
		 * way to do it, but it works, and even removes the maximise button */
		XSizeHints* hints = XAllocSizeHints();
		hints->flags = PMinSize | PMaxSize;
		hints->min_width = size.x;
		hints->min_height = size.y;
		hints->max_width = size.x;
		hints->max_height = size.y;

		XSetWMNormalHints(window->display, window->window, hints);

		XFree(hints);
	}

	XResizeWindow(window->display, window->window, size.x, size.y);
}

void set_window_fullscreen(struct window* window, bool fullscreen) {
	Atom wm_state = XInternAtom(window->display, "_NET_WM_STATE", False);
	Atom fs = XInternAtom(window->display, "_NET_WM_STATE_FULLSCREEN", False);
	XEvent xev = { 0 };
	xev.type = ClientMessage;
	xev.xclient.window = window->window;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = (fullscreen ? 1 : 0);
	xev.xclient.data.l[1] = fs;
	xev.xclient.data.l[2] = 0;
	xev.xclient.data.l[3] = 0;
	XMapWindow(window->display, window->window);
	XSendEvent(window->display, DefaultRootWindow(window->display), False,
		SubstructureRedirectMask | SubstructureNotifyMask, &xev);
	XFlush(window->display);
	XWindowAttributes gwa;
	XGetWindowAttributes(window->display, window->window, &gwa);
	window->w = gwa.width;
	window->h = gwa.height;
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

void set_window_should_close(struct window* window, bool close) {
	window->open = !close;
}

u32 get_window_cursor(struct window* window) {
	return window->cursor;
}

void set_window_cursor(struct window* window, u32 id) {
	window->cursor = id;

	Cursor c;
	
	switch (id) {
		case CURSOR_HAND:
			c = XcursorLibraryLoadCursor(window->display, "hand2");
			break;
		case CURSOR_RESIZE:
			c = XcursorLibraryLoadCursor(window->display, "bottom_right_corner");
			break;
		case CURSOR_MOVE:
			c = XcursorLibraryLoadCursor(window->display, "fleur");
			break;
		case CURSOR_POINTER:
		default:
			c = XcursorLibraryLoadCursor(window->display, "left_ptr");
			break;
	}

	
	XDefineCursor (window->display, window->window, c);
}

void window_enable_repeat(struct window* window, bool enable) {
	window->repeat = enable;
}
