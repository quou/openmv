#pragma once

#include "entity.h"
#include "maths.h"
#include "room.h"

enum {
	player_face_left = 0,
	player_face_right = 1
};

struct player {
	v2f position;
	v2f velocity;

	struct rect collider;

	bool on_ground;
	bool dashing;

	double jump_time;
	double dash_time;

	double dash_fx_time;

	i32 dash_count;

	v2i dash_dir;

	i32 face;

	u32 items;
};

entity new_player_entity(struct world* world);
void player_system(struct world* world, struct renderer* renderer, struct room** room, double ts);

enum {
	upgrade_jetpack = 1 << 0
};

struct upgrade {
	u32 id;
	struct rect collider;
};
