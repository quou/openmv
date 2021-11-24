#include <stdio.h>

#include "bootstrapper.h"
#include "core.h"
#include "res.h"
#include "video.h"
#include "platform.h"

int main() {
	init_time();

	main_window = new_window(1366, 768, "OpenMV");

	video_init();
	res_init();

	init_time();

#if defined(PLATFORM_LINUX)
	const char* lib_path = "./liblogic.so";
#else
	const char* lib_path = "logic.dll";
#endif

	/* Loading screen */
	{
		struct shader sprite_shader = load_shader("res/shaders/sprite.glsl");
		struct renderer* renderer = new_renderer(sprite_shader, make_v2i(1366, 768));

		u8* raw;
		u32 raw_size;
		read_raw("res/DejaVuSansMono.ttf", &raw, &raw_size, false);

		struct font* font = load_font_from_memory(raw, raw_size, 20.0f);

		struct textured_quad quad = {
			.texture = null,
			.rect = { 0 },
			.position = { 0, 0 },
			.dimentions = { 1366, 768 },
			.color = make_color(0x6cafb5, 100),
		};

		const char* text = "Loading...";

		i32 w = text_width(font, text);
		i32 h = text_height(font);

		renderer_push(renderer, &quad);
		render_text(renderer, font, text, (1366 / 2) - (w / 2), (768 / 2) - (h - 2), make_color(0xffffff, 255));

		video_clear();
		renderer_flush(renderer);

		swap_window(main_window);

		free_renderer(renderer);
		free_font(font);
	}

	struct script_context* scripts = new_script_context(lib_path);
	call_on_init(scripts);

	u64 now = get_time(), last = now; 
	double timestep = 0.0;

	while (!window_should_close(main_window)) {
		update_events(main_window);

		video_clear();

		script_context_update(scripts, timestep);
		call_on_update(scripts, timestep);

		swap_window(main_window);

		now = get_time();
		timestep = (double)(now - last) / (double)get_frequency();
		last = now;

		if (timestep > 0.1) {
			timestep = 0.1;
		}
	}

	call_on_deinit(scripts);
	free_script_context(scripts);

	res_deinit();

	free_window(main_window);
}
