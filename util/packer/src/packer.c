#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "core.h"
#include "imui.h"
#include "platform.h"
#include "res.h"
#include "video.h"

#define buffer_size 1000000
u8* buffer;

#define max_files 1024
char** files;
u32 file_count;

char pack_file_buffer[256];
bool pack_file_ok;

char add_file_buffer[256];

char current_file[256];

static void on_text_input(struct window* window, const char* text, void* udata) {
	ui_text_input_event(udata, text);
}

void pack_files_worker(struct thread* thread) {
	*(i32*)get_thread_uptr(thread) = 0;

	FILE* out = fopen(pack_file_buffer, "wb");
	if (!out) {
		fprintf(stderr, "Failed to open `res.pck' for writing.\n");
		return;
	}

	u64 header_size = file_count * (sizeof(u64) * 3) + sizeof(u64);
	u64 cur_size = header_size;

	fwrite(&header_size, sizeof(header_size), 1, out);

	for (u32 i = 0; i < file_count; i++) {
		u64 hash = elf_hash((const u8*)files[i], (u32)strlen(files[i]));

		FILE* file = fopen(files[i], "rb");
		if (!file) { continue; }

		fseek(file, 0, SEEK_END);

		u64 size = ftell(file);

		fwrite(&hash, sizeof(hash), 1, out);
		fwrite(&cur_size, sizeof(cur_size), 1, out);
		fwrite(&size, sizeof(size), 1, out);

		cur_size += size;

		fclose(file);
	}

	for (u32 i = 0; i < file_count; i++) {
		*(i32*)get_thread_uptr(thread) = (i32)(((f32)i / (f32)file_count) * 100.0f);
		strcpy(current_file, files[i]);

		FILE* file = fopen(files[i], "rb");
		if (!file) { continue; }

		fseek(file, 0, SEEK_END);
		u64 file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		for (u64 ii = 0; ii < file_size; ii++) {
			u64 read = fread(buffer, 1, buffer_size, file);
			fwrite(buffer, read, 1, out);
		}

		fclose(file);
	}

	fclose(out);

	return;

}

void init_file_list() {
	for (u32 i = 0; i < file_count; i++) {
		core_free(files[i]);
	}
	
	FILE* list_f = fopen("packed.include", "r");
	if (!list_f) {
		fprintf(stderr, "Failed to read `packed.include' for reading.\n");
		return;
	}

	char line[256];
	u64 len = 0;
	u64 read = 0;

	file_count = 0;

	while (fgets(line, sizeof(line), list_f)) {
		line[strlen(line) - 1] = '\0';

		FILE* file = fopen(line, "r");
		if (!file) {
			fprintf(stderr, "Failed to open `%s' for reading.\n", line);
			continue;
		}

		fclose(file);

		files[file_count++] = copy_string(line);
	}

	fclose(list_f);
}

bool check_pack_ok() {
	FILE* pack = fopen(pack_file_buffer, "a");
	if (pack) {
		fclose(pack);
		return true;
	}
	return false;
}

i32 main() {
	buffer = core_calloc(1, buffer_size);
	files = core_calloc(1, max_files * sizeof(const char*));
	file_count = 0;

	strcpy(pack_file_buffer, "res.pck");
	pack_file_ok = check_pack_ok();

	srand((u32)time(null));

	init_time();

	main_window = new_window(make_v2i(640, 480), "Resource Packer", true);

	video_init();
	res_init();

	struct ui_context* ui = new_ui_context(load_shader_no_pck("res/shaders/sprite.glsl"), main_window, load_font_no_pck("res/DejaVuSans.ttf", 14.0f));
	set_window_uptr(main_window, ui);
	set_on_text_input(main_window, on_text_input);

	u64 now = get_time(), last = now; 
	f64 timestep = 0.0;

	i32* pack_progress = core_calloc(1, sizeof(i32));
	struct thread* worker = new_thread(pack_files_worker);
	set_thread_uptr(worker, pack_progress);

	init_file_list();

	ui_load_layout(ui, "util/packer/lay.out");

	while (!window_should_close(main_window)) {
		update_events(main_window);

		video_clear();

		ui_begin_frame(ui);

		if (ui_begin_window(ui, "Packer", make_v2i(0, 0))) {
			ui_columns(ui, 2, 200);
			
			ui_text(ui, "Package Path");
			if (ui_text_input(ui, pack_file_buffer, sizeof(pack_file_buffer)) && !thread_active(worker)) {
				pack_file_ok = check_pack_ok();
			}
			ui_columns(ui, 1, 0);

			if (!pack_file_ok) {	
				ui_color(ui, make_color(0xff0000, 255));
				char buf[256];
				sprintf(buf, "Cannot open `%s'.", pack_file_buffer);
				ui_text(ui, buf);
				ui_reset_color(ui);
			}

			ui_columns(ui, 2, 200);
			
			ui_text(ui, "Add File");
			if (ui_text_input(ui, add_file_buffer, sizeof(add_file_buffer)) && !thread_active(worker)) {
				FILE* f = fopen(add_file_buffer, "r");
				if (f) {
					files[file_count++] = copy_string(add_file_buffer);
					add_file_buffer[0] = '\0';
					fclose(f);
				}
			}
			ui_columns(ui, 1, 0);

			if (ui_button(ui, "Save to packed.include") && !thread_active(worker)) {
				FILE* f = fopen("packed.include", "w");

				if (f) {
					for (u32 i = 0; i < file_count; i++) {
						fprintf(f, "%s\n", files[i]);
					}

					fclose(f);
				}
			}

			if (!thread_active(worker)) {
				if (ui_button(ui, "Create Package") && pack_file_ok) {
					thread_join(worker);
					thread_execute(worker);
				}
			} else {
				ui_text(ui, "Packing...");
				ui_loading_bar(ui, current_file, *pack_progress);
			}

			ui_text(ui, "Files:");

			ui_columns(ui, 2, 300);
			for (u32 i = 0; i < file_count; i++) {
				ui_text(ui, files[i]);
				if (ui_button(ui, "Remove") && !thread_active(worker)) {
					if (file_count > 1) {
						core_free(files[i]);
						files[i] = files[file_count - 1];
					}
					file_count--;
				}
			}

			ui_columns(ui, 1, 0);

			ui_end_window(ui);
		}

		ui_end_frame(ui);

		swap_window(main_window);
	}

	core_free(pack_progress);
	free_thread(worker);

	core_free(buffer);
	core_free(files);

	free_ui_context(ui);

	res_deinit();

	free_window(main_window);
}
