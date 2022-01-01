#pragma once

/* Loads a Tiled map exported into the OpenMV binary format into a
 * generic data structure. */

#include "common.h"
#include "table.h"
#include "video.h"

#define anim_tile_frame_count 32

struct tile {
	i16 id;
	i16 tileset_id;
};

struct animated_tile {
	bool exists;

	i16 frames[anim_tile_frame_count];
	double durations[anim_tile_frame_count];

	u32 frame_count;
	u32 current_frame;
	double timer;
};

struct tileset {
	struct texture* image;
	char* name;
	i32 tile_w, tile_h;
	i32 tile_count;

	struct animated_tile* animations;
};

enum {
	object_shape_rect = 0,
	object_shape_point,
	object_shape_polygon
};

struct polygon {
	v2f* points;
	u32 count;
};

struct float_rect {
	float x, y, w, h;
};

struct object {
	i32 shape;
	u32 id;
	char* name;
	char* type;

	union {
		struct float_rect rect;
		v2f point;
		struct polygon polygon;
	} as;

	struct table* properties;
};

enum {
	prop_bool = 0,
	prop_number,
	prop_string
};

struct property {
	i32 type;

	union {
		bool boolean;
		double number;
		char* string;
	} as;
};

enum {
	layer_unknown = -1,
	layer_tiles = 0,
	layer_objects
};

struct layer {
	i32 type;
	char* name;

	union {
		struct {
			struct tile* tiles;
			u32 w, h;
		} tile_layer;

		struct {
			struct object* objects;
			u32 object_count;
		} object_layer;
	} as;

	struct table* properties;
};

struct tiled_map {
	struct layer* layers;
	u32 layer_count;

	struct tileset* tilesets;
	u32 tileset_count;

	struct table* properties;
};

struct tiled_map* load_map(const char* filename);
void free_map(struct tiled_map* map);
