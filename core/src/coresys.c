#include "coresys.h"

void render_system(struct world* world, struct renderer* renderer, double ts) {
	for (view(world, view, type_info(struct transform), type_info(struct sprite))) {
		struct transform* t = view_get(&view, struct transform);
		struct sprite* s = view_get(&view, struct sprite);

		struct textured_quad quad = {
			.texture = s->texture,
			.rect = s->rect,
			.position = t->position,
			.dimentions = t->dimentions,
			.color = s->color,
			.rotation = t->rotation,
			.origin = s->origin
		};

		renderer_push(renderer, &quad); 
	}

	for (view(world, view, type_info(struct transform), type_info(struct animated_sprite))) {
		struct transform *t = view_get(&view, struct transform);
		struct animated_sprite* s = view_get(&view, struct animated_sprite);

		s->timer += ts * s->speed;
		if (s->timer >= 1.0) {
			s->timer = 0.0;
			s->current_frame++;
			if (s->current_frame >= s->frame_count) {
				s->current_frame = 0;
			}
		}

		struct textured_quad quad = {
			.texture = s->texture,
			.rect = s->frames[s->current_frame],
			.position = t->position,
			.dimentions = t->dimentions,
			.color = s->color,
			.rotation = t->rotation,
			.origin = s->origin
		};

		renderer_push(renderer, &quad); 
	}
}
