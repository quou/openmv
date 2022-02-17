#pragma once

#include "core.h"
#include "platform.h"

struct script_context;

struct script_context* new_script_context(const char* lib_path);
void free_script_context(struct script_context* ctx);
void script_context_update(struct script_context* ctx, f64 ts);

void scripts_allocate_storage(struct script_context* ctx);

struct window* script_call_create_window(struct script_context* ctx);

void call_on_init(struct script_context* ctx);
void call_on_update(struct script_context* ctx, f64 ts);
void call_on_deinit(struct script_context* ctx);
