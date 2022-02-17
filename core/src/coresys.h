#pragma once

#include "entity.h"
#include "maths.h"
#include "res.h"

struct transform {
	v2f position;
	v2i dimentions;
	i32 z;

	f32 rotation;
};

struct collider {
	struct rect rect;
};

struct sprite {
	struct texture* texture;
	struct rect rect;
	v2f origin;
	struct color color;

	bool hidden;
	bool inverted;
	bool unlit;
};

#define animated_sprite_max_frames 16

struct animated_sprite {
	u32 id;
	struct texture* texture;
	struct rect frames[animated_sprite_max_frames];
	u32 frame_count;
	u32 current_frame;
	f64 speed;
	f64 timer;
	v2f origin;
	struct color color;

	bool hidden;
	bool inverted;
	bool unlit;

	f32 rotation;
};

API void apply_lights(struct world* world, struct renderer* renderer);

/* Process all the entities in the world that have a sprite and a transform,
 * or an animated sprite and a transform. */
API void render_system(struct world* world, struct renderer* renderer, f64 ts);
