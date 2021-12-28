#include <stdio.h>
#include <time.h>
#include <setjmp.h>

#include "audio.h"
#include "bootstrapper.h"
#include "core.h"
#include "platform.h"
#include "res.h"
#include "video.h"

#if defined(PLATFORM_LINUX)
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif

struct screen {
	struct shader sprite_shader;
	struct renderer* renderer;
	struct font* font;

	char* text;
};

void init_screen(struct screen* screen, const char* text) {
	screen->sprite_shader = load_shader("res/shaders/sprite.glsl");
	screen->renderer = new_renderer(screen->sprite_shader, make_v2i(1366, 768));

	u8* raw;
	u64 raw_size;
	read_raw("res/CourierPrime.ttf", &raw, &raw_size, false);

	screen->font = load_font_from_memory(raw, raw_size, 20.0f);

	screen->text = copy_string(text);
}

void deinit_screen(struct screen* screen) {
	core_free(screen->text);

	free_renderer(screen->renderer);
	free_font(screen->font);
}

void update_screen(struct screen* screen) {
	struct textured_quad quad = {
		.texture = null,
		.rect = { 0 },
		.position = { 0, 0 },
		.dimentions = { 1366, 768 },
		.color = make_color(0x6cafb5, 100),
	};

	i32 w = text_width(screen->font, screen->text);
	i32 h = text_height(screen->font);

	renderer_push(screen->renderer, &quad);
	render_text(screen->renderer, screen->font, screen->text, (1366 / 2) - (w / 2), (768 / 2) - (h - 2), make_color(0xffffff, 255));

	video_clear();
	renderer_flush(screen->renderer);

	swap_window(main_window);
}

struct screen sigsegv_screen;
jmp_buf env_buffer;

bool segfault;

void sigsegv_handle(i32 sig) {
	fprintf(stderr, "Process recieved SIGSEV!\n");
	segfault = true;

	init_screen(&sigsegv_screen, "Segmentation Fault!");

	longjmp(env_buffer, 0);
}

int main() {
	srand(time(null));

	init_time();

	main_window = new_window(1366, 768, "OpenMV");

	video_init();
	audio_init();
	res_init();

	init_time();

#if defined(PLATFORM_LINUX)
	const char* lib_path = "./liblogic.so";
#else
	const char* lib_path = "logic.dll";
#endif

	/* Loading screen */
	{
		struct screen s;
		init_screen(&s, "Loading...");
		update_screen(&s);
		deinit_screen(&s);
	}

	struct script_context* scripts = new_script_context(lib_path);

#if defined(PLATFORM_LINUX)
	signal(SIGSEGV, sigsegv_handle);
#endif

	segfault = false;

	setjmp(env_buffer);

	if (!segfault) {
		call_on_init(scripts);
	}

	u64 now = get_time(), last = now; 
	double timestep = 0.0;

	while (!window_should_close(main_window)) {
		update_events(main_window);

		if (!segfault) {
			video_clear();

			script_context_update(scripts, timestep);
			call_on_update(scripts, timestep);

			swap_window(main_window);

			audio_update();

			now = get_time();
			timestep = (double)(now - last) / (double)get_frequency();
			last = now;

			if (timestep > 0.1) {
				timestep = 0.1;
			}
		} else {
			update_screen(&sigsegv_screen);
		}
	}

	if (segfault) {
		kill(getpid(), SIGSEGV);
	}

	call_on_deinit(scripts);
	free_script_context(scripts);

	audio_deinit();

	res_deinit();

	free_window(main_window);
}
