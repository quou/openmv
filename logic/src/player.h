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

	double jump_time;

	i32 face;
};

entity new_player_entity(struct world* world);
void player_system(struct world* world, struct renderer* renderer, struct room** room, double ts);
