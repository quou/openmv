#include "coresys.h"

void render_system(struct world* world, struct renderer* renderer) {
	for (view(world, view, type_info(struct transform), type_info(struct sprite))) {
		struct transform* t = view_get(&view, struct transform);
		struct sprite* r = view_get(&view, struct sprite);

		struct textured_quad quad = {
			.texture = r->texture,
			.rect = r->rect,
			.position = t->position,
			.dimentions = t->dimentions,
			.color = r->color,
			.rotation = t->rotation,
			.origin = r->origin
		};

		renderer_push(renderer, &quad); 
	}
}
