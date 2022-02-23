#include <stdio.h>
#include <time.h>

#include "core.h"
#include "platform.h"
#include "res.h"
#include "rui.h"
#include "video.h"

i32 main() {
	srand((u32)time(null));

	init_time();

	main_window = new_window(make_v2i(1366, 768), "Retained-mode UI", true);

	video_init();
	res_init();

	struct renderer* renderer = new_renderer(load_shader("res/shaders/sprite.glsl"), make_v2i(1366, 768));

	struct rui_ctx* ui = rui_new();

	u64 now = get_time(), last = now; 
	f64 timestep = 0.0;

	while (!window_should_close(main_window)) {
		update_events(main_window);

		renderer_fit_to_main_window(renderer);

		video_clear();

		rui_render(renderer);

		swap_window(main_window);
	}

	rui_free(ui);

	free_renderer(renderer);

	free_window(main_window);
}
