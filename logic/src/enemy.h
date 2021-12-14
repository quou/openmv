#pragma once

#include "entity.h"
#include "maths.h"
#include "room.h"

struct enemy {
	i32 hp;
	i32 damage;
	i32 money_drop;
	struct rect collider;
};

struct bat {
	v2f old_position;
	double offset;

	char* path_name;
};

struct path_follow {
	char* path_name;
	struct room* room;

	i32 node;
	float speed;
	bool reverse;

	bool first_frame;
};

entity new_bat(struct world* world, struct room* room, v2f position, char* path_name);

void enemy_system(struct world* world, struct room* room, double ts);
