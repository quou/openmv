#pragma once

#include "common.h"
#include "video.h"

struct room;

struct room* load_room(const char* path);
void free_room(struct room* room);
void draw_room(struct room* room, struct renderer* renderer);
