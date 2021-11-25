#include "coresys.h"

struct render_queue_item {
	struct textured_quad quad;
	i32 z;
};

struct render_queue {
	struct render_queue_item* items;
	u32 count;
	u32 capacity;
};

static void init_render_queue(struct render_queue* queue) {
	*queue = (struct render_queue) { 0 };
}

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

	if (queue->items) {
		core_free(queue->items);
	}
}

void render_system(struct world* world, struct renderer* renderer, double ts) {
	struct render_queue queue = { 0 };

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

		render_queue_push(&queue, (struct render_queue_item) {
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

		struct textured_quad quad = {
			.texture = s->texture,
			.rect = s->frames[s->current_frame],
			.position = t->position,
			.dimentions = t->dimentions,
			.color = s->color,
			.rotation = t->rotation,
			.origin = s->origin
		};

		render_queue_push(&queue, (struct render_queue_item) {
			.quad = quad,
			.z = t->z
		});
	}

	render_queue_flush(&queue, renderer);
}
