#pragma once

#include "entity.h"
#include "maths.h"

struct player {
	v2f position;
	v2f velocity;
};

entity new_player_entity(struct world* world);
void player_system(struct world* world, double ts);
