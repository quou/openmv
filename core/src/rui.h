/* Retained mode graphical user interface library. */

#pragma once

#include "common.h"
#include "video.h"

struct rui_ctx;

API struct rui_ctx* rui_new();
API void rui_free(struct rui_ctx* ctx);

API void rui_render(struct renderer* renderer);
