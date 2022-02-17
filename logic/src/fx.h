#pragma once

#include "entity.h"
#include "maths.h"

struct jetpack_fx {
	f64 timer;
};

entity new_jetpack_particle(struct world* world, v2f position);

void fx_system(struct world* world, f64 ts);
