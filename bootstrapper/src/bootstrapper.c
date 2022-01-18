#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bootstrapper.h"
#include "dynlib.h"

const char* working_script_name = "./workasm.dll";

typedef struct window* (*script_on_create_window_func)();

typedef void (*script_on_init_func)();
typedef void (*script_on_update_func)(double ts);
typedef void (*script_on_deinit_func)();

typedef void (*script_on_reload_func)(void*);
typedef u64 (*script_get_storage_size_func)();

struct script_context {
	void* handle;

	u64 lib_mod_time;
	const char* lib_path;

	script_on_init_func on_init;
	script_on_update_func on_update;
	script_on_deinit_func on_deinit;

	script_on_reload_func on_reload;
	script_get_storage_size_func get_storage_size;

	void* instance;
	u64 instance_size;

	double timer;
};

static void open_funcs(struct script_context* ctx) {
	ctx->handle = open_dynlib(working_script_name);
	if (!ctx->handle) {
		fprintf(stderr, "Failed to load script assembly: `%s': %s.\n",
			working_script_name, dynlib_get_error());
	}
	
	ctx->on_reload = (script_on_reload_func)dynlib_get_sym(ctx->handle, "on_reload");
	if (!ctx->on_reload) {
		fprintf(stderr, "Failed to locate function `on_reload'.\n");
	}

	ctx->get_storage_size = (script_get_storage_size_func)dynlib_get_sym(ctx->handle, "get_storage_size");
	if (!ctx->get_storage_size) {
		fprintf(stderr, "Failed to locate function `get_storage_size'.\n");
	}

	ctx->on_init = (script_on_init_func)dynlib_get_sym(ctx->handle, "on_init");
	if (!ctx->on_init) {
		fprintf(stderr, "Failed to locate function `on_init'.\n");
	}

	ctx->on_update = (script_on_update_func)dynlib_get_sym(ctx->handle, "on_update");
	if (!ctx->on_update) {
		fprintf(stderr, "Failed to locate function `on_update'.\n");
	}
	
	ctx->on_deinit = (script_on_deinit_func)dynlib_get_sym(ctx->handle, "on_deinit");
	if (!ctx->on_deinit) {
		fprintf(stderr, "Failed to locate function `on_deinit'.\n");
	}
}

static void copy_asm(const char* lib_path) {
	FILE* src = fopen(lib_path, "rb");
	if (!src) {
		fprintf(stderr, "Failed to open `%s'.\n", lib_path);
		return;
	}

	FILE* dst = fopen(working_script_name, "wb");
	if (!dst) {
		fprintf(stderr, "Failed to open `%s' for write.\n", working_script_name);
		return;
	}

	fseek(src, 0, SEEK_END);
	u64 size = ftell(src);
	fseek(src, 0, SEEK_SET);

	char buffer[2048];
	for (u64 i = 0; i < size; i += 2048) {
		u64 read = fread(buffer, 1, 2048, src);
		fwrite(buffer, read, 1, dst);
	}

	fclose(src);
	fclose(dst);
}

struct script_context* new_script_context(const char* lib_path) {
	struct script_context* ctx = core_calloc(1, sizeof(struct script_context));

	ctx->lib_path = lib_path;

	struct stat s;
	if (stat(lib_path, &s) == 0) {
		ctx->lib_mod_time = s.st_mtime;
	}

#ifdef RELEASE
	working_script_name = lib_path;
#else
	copy_asm(lib_path);
#endif

	open_funcs(ctx);

	return ctx;
}

void scripts_allocate_storage(struct script_context* ctx) {
	if (ctx->get_storage_size) {
		u64 size = ctx->get_storage_size();

		ctx->instance = core_calloc(1, size);

		ctx->instance_size = size;

		if (ctx->on_reload) {
			ctx->on_reload(ctx->instance);
		}
	}
}

void free_script_context(struct script_context* ctx) {
	if (ctx->instance) {
		core_free(ctx->instance);
	}

	if (ctx->handle) {
		close_dynlib(ctx->handle);
	}

	core_free(ctx);
}

struct window* script_call_create_window(struct script_context* ctx) {
	script_on_create_window_func f = (script_on_create_window_func)dynlib_get_sym(ctx->handle, "create_window");
	if (!f) {
		fprintf(stderr, "Failed to locate function `create_window'.\n");
	} else {
		return f();
	}

	return null;
}

void script_context_update(struct script_context* ctx, double ts) {
/* Don't hot reload in release mode, for performance reasons.
 *
 * It isn't required, either, unless someone is trying to mod the game,
 * since the only reason why the script assembly would be updated would be
 * if someone is changing game logic and recompiling it. That should be
 * done in a debug build anyhow. */
#ifdef DEBUG
	ctx->timer += ts;

	if (ctx->timer >= 1.0) {
		ctx->timer = 0.0;

		struct stat s;
		if (stat(ctx->lib_path, &s) == 0) {
			if (s.st_mtime > ctx->lib_mod_time) {
				ctx->lib_mod_time = s.st_mtime;

				if (ctx->handle) {
					close_dynlib(ctx->handle);
				}

				copy_asm(ctx->lib_path);
				open_funcs(ctx);

				if (ctx->get_storage_size) {
					u64 size = ctx->get_storage_size();

					if (size != ctx->instance_size) {
						ctx->instance = core_realloc(ctx->instance, size);
						if (size > ctx->instance_size) {
						memset((u8*)ctx->instance + ctx->instance_size, 0, size - ctx->instance_size);
						}

						ctx->instance_size = size;
					}

					if (ctx->on_reload) {
						ctx->on_reload(ctx->instance);
					}
				}
			}
		}
	}
#endif
}

void call_on_init(struct script_context* ctx) {
	if (ctx->on_init) {
		ctx->on_init();
	}
}

void call_on_update(struct script_context* ctx, double ts) {
	if (ctx->on_update) {
		ctx->on_update(ts);
	}
}

void call_on_deinit(struct script_context* ctx) {
	if (ctx->on_deinit) {
		ctx->on_deinit();
	}
}
