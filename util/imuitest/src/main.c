#include <stdio.h>
#include <time.h>

#include "core.h"
#include "platform.h"
#include "res.h"
#include "imui.h"
#include "video.h"

i32 main() {
	srand((u32)time(null));

	init_time();

	main_window = new_window(make_v2i(1366, 768), "Immediate Mode UI", true);

	video_init();
	res_init();

	struct ui_context* ui = new_ui_context(load_shader("res/shaders/sprite.glsl"), main_window, load_font("res/DejaVuSans.ttf", 14.0f));

	u64 now = get_time(), last = now; 
	f64 timestep = 0.0;

	while (!window_should_close(main_window)) {
		update_events(main_window);

		video_clear();

		ui_begin_frame(ui);

		if (ui_begin_window(ui, "Test Window A", make_v2i(0, 0))) {
			ui_end_window(ui);
		}

		if (ui_begin_window(ui, "Test Window B", make_v2i(0, 300))) {
			ui_end_window(ui);
		}

		ui_end_frame(ui);

		swap_window(main_window);
	}

	free_ui_context(ui);

	res_deinit();

	free_window(main_window);
}
