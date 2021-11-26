#pragma once

#include "entity.h"
#include "maths.h"
#include "room.h"

struct enemy {
	i32 hp;
	struct rect collider;
};

struct bat {
	v2f position;
	v2f old_position;
	double offset;
};

entity new_bat(struct world* world, struct room* room, v2i position);

void enemy_system(struct world* world, double ts);
