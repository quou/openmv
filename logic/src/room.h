#pragma once

#include "common.h"
#include "video.h"

struct room;
struct player;

struct path {
	v2f* points;
	u32 count;
};

struct room* load_room(struct world* world, const char* path);
void free_room(struct room* room);
void update_room(struct room* room, double ts);
void draw_room(struct room* room, struct renderer* renderer, double ts);
void draw_room_forground(struct room* room, struct renderer* renderer);
void handle_body_collisions(struct room** room, struct rect rect, v2f* position, v2f* velocity);
void handle_body_interactions(struct room** room, struct rect rect, entity body, bool body_on_ground);
bool rect_room_overlap(struct room* room, struct rect rect, v2i* normal);

char* get_room_path(struct room* room);

v2i get_spawn(struct room* room);
struct rect room_get_camera_bounds(struct room* room);
struct path* get_path(struct room* room, const char* name);

struct room_child {
	struct room* parent;
};

struct save_point {
	struct rect rect;
};

entity new_save_point(struct world* world, struct room* room, struct rect rect);
