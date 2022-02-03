#include "coresys.h"

struct render_queue_item {
	struct textured_quad quad;
	i32 z;
};

struct render_queue {
	struct render_queue_item* items;
	u32 count;
	u32 capacity;
} render_queue = { 0 };

static void render_queue_push(struct render_queue* queue, struct render_queue_item item) {
	if (queue->count >= queue->capacity) {
		queue->capacity = queue->capacity < 8 ? 8 : queue->capacity * 2;
		queue->items = core_realloc(queue->items, queue->capacity * sizeof(struct render_queue_item));
	}

	queue->items[queue->count++] = item;
}

static i32 render_queue_item_cmp(const void* a, const void* b) {
	return ((struct render_queue_item*)a)->z > ((struct render_queue_item*)b)->z;
}

static void render_queue_flush(struct render_queue* queue, struct renderer* renderer) {
	qsort(queue->items, queue->count, sizeof(struct render_queue_item), render_queue_item_cmp);

	for (u32 i = 0; i < queue->count; i++) {
		renderer_push(renderer, &queue->items[i].quad);
	}

	queue->count = 0;
}

void apply_lights(struct world* world, struct renderer* renderer) {
	for (view(world, view, type_info(struct transform), type_info(struct light))) {
		struct transform* transform = view_get(&view, struct transform);
		struct light* light = view_get(&view, struct light);

		light->position = transform->position;

		renderer_push_light(renderer, *light);
	}
}

void render_system(struct world* world, struct renderer* renderer, double ts) {
	render_queue.count = 0;

	for (view(world, view, type_info(struct transform), type_info(struct sprite))) {
		struct transform* t = view_get(&view, struct transform);
		struct sprite* s = view_get(&view, struct sprite);

		if (s->hidden) { continue; }

		struct textured_quad quad = {
			.texture = s->texture,
			.rect = s->rect,
			.position = make_v2i(t->position.x, t->position.y),
			.dimentions = t->dimentions,
			.color = s->color,
			.origin = s->origin,
			.inverted = s->inverted,
			.unlit = s->unlit,
			.rotation = t->rotation
		};

		render_queue_push(&render_queue, (struct render_queue_item) {
			.quad = quad,
			.z = t->z
		});
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

		if (s->hidden) { continue; }

		struct textured_quad quad = {
			.texture = s->texture,
			.rect = s->frames[s->current_frame],
			.position = make_v2i(t->position.x, t->position.y),
			.dimentions = t->dimentions,
			.color = s->color,
			.origin = s->origin,
			.inverted = s->inverted,
			.unlit = s->unlit,
			.rotation = t->rotation
		};

		render_queue_push(&render_queue, (struct render_queue_item) {
			.quad = quad,
			.z = t->z
		});
	}

	render_queue_flush(&render_queue, renderer);
}

