#pragma once

#include "common.h"
#include "maths.h"
#include "platform.h"
#include "video.h"

/* Immediate mode, state based GUI. */

struct ui_context;

API struct ui_context* new_ui_context(struct shader* shader, struct window* window, struct font* font);
API void free_ui_context(struct ui_context* ui);
API void ui_text_input_event(struct ui_context* ui, const char* text);
API void ui_begin_frame(struct ui_context* ui);
API void ui_end_frame(struct ui_context* ui);

API bool ui_begin_window(struct ui_context* ui, const char* name, v2i position);
API void ui_end_window(struct ui_context* ui);

API void ui_text(struct ui_context* ui, const char* text);
API bool ui_button(struct ui_context* ui, const char* text);
API void ui_text_input(struct ui_context* ui, const char* label, char* buf, u32 buf_size);
API bool ui_image(struct ui_context* ui, struct texture* texture, struct rect rect);
