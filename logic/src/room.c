#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "consts.h"
#include "res.h"
#include "room.h"

struct tile {
	i32 id;
	i32 tileset_id;
};

struct tileset {
	struct texture* image;
	char* name;
	i32 tile_w, tile_h;
};

enum {
	layer_unknown = -1,
	layer_tiles,
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
	} as;
};

struct room {
	struct layer* layers;
	u32 layer_count;

	struct tileset* tilesets;
	u32 tileset_count;

	struct rect* box_colliders;
	u32 box_collider_count;
};

struct room* load_room(const char* path) {
	struct room* room = calloc(1, sizeof(struct room));

	FILE* file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "Failed to fopen file `%s'.", path);
		return null;
	}

	/* Load the tilesets. */
	fread(&room->tileset_count, sizeof(room->tileset_count), 1, file);
	room->tilesets = calloc(room->tileset_count, sizeof(struct tileset));

	for (u32 i = 0; i < room->tileset_count; i++) {
		struct tileset* current = room->tilesets + i;

		/* Read the name. */
		u32 name_len;
		fread(&name_len, sizeof(name_len), 1, file);
		current->name = malloc(name_len + 1);
		current->name[name_len] = '\0';
		fread(current->name, 1, name_len, file);

		/* Read the tileset image. */
		u32 path_len;
		fread(&path_len, sizeof(path_len), 1, file);
		char* path = malloc(path_len + 1);
		path[path_len] = '\0';
		fread(path, 1, path_len, file);
		current->image = load_texture(path);
		free(path);

		/* Read the tile width and height */
		fread(&current->tile_w, sizeof(current->tile_w), 1, file);
		fread(&current->tile_h, sizeof(current->tile_h), 1, file);
	}

	/* Load the tile layers */	
	fread(&room->layer_count, sizeof(room->layer_count), 1, file);
	room->layers = calloc(room->layer_count, sizeof(struct layer));

	for (u32 i = 0; i < room->layer_count; i++) {
		struct layer* layer = room->layers + i;

		/* Read the name. */
		u32 name_len;
		fread(&name_len, sizeof(name_len), 1, file);
		layer->name = malloc(name_len + 1);
		layer->name[name_len] = '\0';
		fread(layer->name, 1, name_len, file);

		fread(&layer->type, sizeof(layer->type), 1, file);

		switch (layer->type) {
			case layer_tiles: {
				fread(&layer->as.tile_layer.w, sizeof(layer->as.tile_layer.w), 1, file);
				fread(&layer->as.tile_layer.h, sizeof(layer->as.tile_layer.h), 1, file);

				layer->as.tile_layer.tiles = malloc(sizeof(struct tile) * layer->as.tile_layer.w * layer->as.tile_layer.h);

				u32 w = layer->as.tile_layer.w;
				u32 h = layer->as.tile_layer.h;
				for (u32 y = 0; y < h; y++) {
					for (u32 x = 0; x < w; x++) {
						i32 id, tileset_idx = 0;

						fread(&id, sizeof(id), 1, file);

						if (id != -1) {
							for (i32 ii = 0; ii < room->tileset_count; ii++) {
								struct tileset* tileset = room->tilesets + ii;
								i32 tile_count = (tileset->image->width / tileset->tile_w) *
									(tileset->image->height / tileset->tile_h);

								if (id >= tile_count) {
									id -= tile_count;
									tileset_idx++;
								} else {
									break;
								}
							}
						}

						layer->as.tile_layer.tiles[x + y * w] = (struct tile) {
							.id = id,
							.tileset_id = tileset_idx
						};
					}
				}
			} break;
			case layer_objects: {
				u32 object_count;
				fread(&object_count, sizeof(object_count), 1, file);

				if (strcmp(layer->name, "collisions") == 0) {
					room->box_collider_count = object_count;
					room->box_colliders = malloc(sizeof(*room->box_colliders) * object_count);
					for (u32 ii = 0; ii < object_count; ii++) {
						fread(room->box_colliders + ii, sizeof(*room->box_colliders), 1, file);
					}
				}
			} break;
			default: break;
		}
	}

	fclose(file);

	return room;
}

void free_room(struct room* room) {
	if (room->tilesets) {
		for (u32 i = 0; i < room->tileset_count; i++) {
			free(room->tilesets[i].name);
		}

		free(room->tilesets);
	}

	if (room->layers) {
		for (u32 i = 0; i < room->layer_count; i++) {
			free(room->layers[i].name);

			switch (room->layers[i].type) {
				case layer_tiles:
					free(room->layers[i].as.tile_layer.tiles);
					break;
				default: break;
			}
		}

		free(room->layers);
	}

	if (room->box_colliders) {
		free(room->box_colliders);
	}

	free(room);
}

void draw_room(struct room* room, struct renderer* renderer) {
	for (u32 i = 0; i < room->layer_count; i++) {
		struct layer* layer = room->layers + i;

		if (layer->type == layer_tiles) {
			for (u32 y = 0; y < layer->as.tile_layer.h; y++) {
				for (u32 x = 0; x < layer->as.tile_layer.w; x++) {
					struct tile tile = layer->as.tile_layer.tiles[x + y * layer->as.tile_layer.w];
					if (tile.id != -1) {
						struct tileset* set = room->tilesets + tile.tileset_id;

						struct textured_quad quad = {
							.texture = set->image,
							.position = { x * set->tile_w * sprite_scale, y * set->tile_h * sprite_scale },
							.dimentions = { set->tile_w * sprite_scale, set->tile_h * sprite_scale },
							.rect = {
								.x = ((tile.id % (set->image->width / set->tile_w)) * set->tile_w),
								.y = ((tile.id / (set->image->height / set->tile_h)) * set->tile_h),
								.w = set->tile_w,
								.h = set->tile_h
							},
							.color = { 255, 255, 255, 255 }
						};

						renderer_push(renderer, &quad);
					}
				}
			}
		}
	}
}
