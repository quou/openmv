#include <stdlib.h>

#include "room.h"
#include "consts.h"

struct room* new_room(u32 w, u32 h) {
	struct room* room = calloc(1, sizeof(struct room));

	room->width = w;
	room->height = h;

	return room;
}

void free_room(struct room* room) {
	if (room->layers) {
		for (u32 i = 0; i < room->layer_count; i++) {
			free(room->layers[i].data);
		}

		free(room->layers);
	}

	free(room);
}

void draw_room(struct room* room, struct renderer* renderer) {
	for (u32 i = 0; i < room->layer_count; i++) {
		struct tile_layer* layer = room->layers + i;

		struct textured_quad quad = {
			.texture = room->tile_set.texture,
			.dimentions = {
				room->tile_set.tile_size * sprite_scale,
				room->tile_set.tile_size * sprite_scale
			},
			.color = { 255, 255, 255, 255 }
		};

		for (u32 y = 0; y < room->height; y++) {
			for (u32 x = 0; x < room->width; x++) {
				if (layer->data[x + y * room->width] != 0) {
					quad.position = (v2i) {
						x * room->tile_set.tile_size * sprite_scale,
						y * room->tile_set.tile_size * sprite_scale
					};
					quad.rect = room->tile_set.tiles[layer->data[x + y * room->width]];

					renderer_push(renderer, &quad);
				}
			}
		}
	}
}

struct tile_layer* new_tile_layer(struct room* room) {
	if (room->layer_count >= room->layer_capacity) {
		room->layer_capacity = room->layer_capacity < 8 ? 8 : room->layer_capacity * 2;
		room->layers = realloc(room->layers, room->layer_capacity * sizeof(struct tile_layer));
	}

	struct tile_layer* layer = &room->layers[room->layer_count++];

	layer->data = calloc(1, room->width * room->height * sizeof(u8));

	return layer;
}

void set_tile(struct room* room, struct tile_layer* layer, u32 x, u32 y, u8 tile_id) {
	layer->data[x + y * room->width] = tile_id;
}
