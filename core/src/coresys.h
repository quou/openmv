#pragma once

#include "entity.h"
#include "maths.h"
#include "res.h"

struct transform {
	const char* name;
	v2f position;
	v2i dimentions;
	i32 z;
};

struct sprite {
	struct texture* texture;
	struct rect rect;
	v2f origin;
	struct color color;

	bool hidden;
};

#define animated_sprite_max_frames 16

struct animated_sprite {
	u32 id;
	struct texture* texture;
	struct rect frames[animated_sprite_max_frames];
	u32 frame_count;
	u32 current_frame;
	double speed;
	double timer;
	v2f origin;
	struct color color;

	bool hidden;
};

/* Process all the entities in the world that have a sprite and a transform,
 * or an animated sprite and a transform. */
API void render_system(struct world* world, struct renderer* renderer, double ts);
