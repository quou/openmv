#pragma once

#include "entity.h"
#include "maths.h"
#include "room.h"

struct enemy {
	i32 hp;
	i32 damage;
	i32 money_drop;

	bool invul;
	double invul_timer;
};

struct bat {
	v2f old_position;
	double offset;

	char* path_name;
};

struct spider {
	v2f velocity;

	bool triggered;

	struct room* room;
};

struct scav {
	v2f velocity;

	bool triggered;

	double shoot_cooldown;

	struct room* room;
};

struct drill {
	v2f velocity;

	bool triggered;

	struct room* room;
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
entity new_spider(struct world* world, struct room* room, v2f position);
entity new_drill(struct world* world, struct room* room, v2f position);
entity new_scav(struct world* world, struct room* room, v2f position);

void enemy_system(struct world* world, struct room* room, double ts);
