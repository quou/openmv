#pragma once

#include "entity.h"
#include "maths.h"
#include "room.h"

struct player {
	v2f position;
	v2f velocity;

	struct rect collider;
};

entity new_player_entity(struct world* world);
void player_system(struct world* world, struct room* room, double ts);
