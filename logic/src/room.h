#pragma once

#include "common.h"
#include "logic_common.h"
#include "entity.h"
#include "video.h"

/* There's an interesting bug with rooms where the memory usage
 * reported by core_get_memory_usage decreases every time a room
 * is freed... I'm not sure why this is the case. I don't think it
 * is a bug with the memory tracking, since it doesn't happen in
 * other areas of the codebase. */

struct room;
struct player;

struct path {
	v2f* points;
	u32 count;
};

struct room* load_room(struct world* world, const char* path);
void free_room(struct room* room);
void update_room(struct room* room, f64 ts, f64 actual_ts);
void update_room_light(struct room* room, struct renderer* renderer);
void draw_room(struct room* room, struct renderer* renderer, f64 ts);
void draw_room_forground(struct room* room, struct renderer* renderer, struct renderer* transition_renderer);
bool handle_body_collisions(struct room* room, struct rect rect, v2f* position, v2f* velocity);
void handle_body_interactions(struct room** room, struct rect rect, entity body, bool body_on_ground);
void room_transition_to(struct room** room, entity body, struct rect collider, const char* path, const char* entrance);
bool rect_room_overlap(struct room* room, struct rect rect, v2i* normal);

typedef void (*dialogue_ask_submit_func)(bool, void*);

void dialogue_message(const char* text, void* ctx);
void dialogue_ask(const char* text, dialogue_ask_submit_func on_submit, void* ctx);

struct player* get_player();

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

enum {
	spawn_type_broken_robot = 0
};

struct entity_spawner {
	u32 spawn_type;
	f64 next_spawn;
	f64 max_increment;
	f64 min_increment;
};

struct fall {
	f64 mul;
	v2f velocity;
};

struct lava_interact {
	u32 i;
};

struct lava {
	struct rect collider;
};

struct lava_particle {
	v2f velocity;
	f64 lifetime;
	f32 rotation_inc;
};
