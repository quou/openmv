#pragma once

#include "common.h"
#include "video.h"

struct tile_set {
	struct texture* texture;
	u32 tile_size;
	struct rect tiles[255];
};

struct tile_layer {
	u8* data;
};

struct room {
	struct tile_layer* layers;
	u32 layer_count;
	u32 layer_capacity;

	u32 width, height;

	struct tile_set tile_set;
};

struct room* new_room(u32 w, u32 h);
void free_room(struct room* room);
void draw_room(struct room* room, struct renderer* renderer);

struct tile_layer* new_tile_layer(struct room* room);
void set_tile(struct room* room, struct tile_layer* layer, u32 x, u32 y, u8 tile_id);
