#include <stdio.h>

#include "bootstrapper.h"
#include "core.h"
#include "res.h"
#include "platform.h"

int main() {
	init_time();

	main_window = new_window(1366, 768, "OpenMV");

	video_init();
	res_init();

	init_time();

	struct script_context* scripts = new_script_context("./liblogic.so");
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
	}

	call_on_deinit(scripts);
	free_script_context(scripts);

	res_deinit();

	free_window(main_window);
}
