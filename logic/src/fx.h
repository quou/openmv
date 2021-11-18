#pragma once

#include "entity.h"
#include "maths.h"

struct jetpack_fx {
	double timer;
};

entity new_jetpack_particle(struct world* world, v2i position);

void fx_system(struct world* world, double ts);
