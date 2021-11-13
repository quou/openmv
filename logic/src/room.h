#pragma once

#include "common.h"
#include "video.h"

struct room;

struct room* load_room(const char* path);
void free_room(struct room* room);
void draw_room(struct room* room, struct renderer* renderer);
void handle_body_collisions(struct room** room, struct rect rect, v2f* position, v2f* velocity);
