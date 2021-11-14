#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "consts.h"
#include "core.h"
#include "logic_store.h"
#include "res.h"
#include "room.h"
#include "table.h"

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

struct transition_trigger {
	struct rect rect;
	char* change_to;
	char* entrance;
};

struct room {
	struct layer* layers;
	u32 layer_count;

	struct tileset* tilesets;
	u32 tileset_count;

	struct rect* box_colliders;
	u32 box_collider_count;

	struct table* entrances;

	struct transition_trigger* transition_triggers;
	u32 transition_trigger_count;
};

struct room* load_room(const char* path) {
	struct room* room = calloc(1, sizeof(struct room));

	FILE* file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "Failed to fopen file `%s'.", path);
		return null;
	}

	room->entrances = new_table(sizeof(v2i));

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
				} else if (strcmp(layer->name, "entrances") == 0) {
					struct rect r;
					for (u32 ii = 0; ii < object_count; ii++) {
						fread(&r, sizeof(r), 1, file);

						u32 obj_name_len;
						fread(&obj_name_len, sizeof(obj_name_len), 1, file);
						char* obj_name = malloc(obj_name_len + 1);
						obj_name[obj_name_len] = '\0';
						fread(obj_name, 1, obj_name_len, file);

						table_set(room->entrances, obj_name, &r);

						free(obj_name);
					}
				} else if (strcmp(layer->name, "transition_triggers") == 0) {
					room->transition_triggers = malloc(sizeof(struct transition_trigger) * object_count);
					room->transition_trigger_count = object_count;

					struct rect r;
					for (u32 ii = 0; ii < object_count; ii++) {
						fread(&r, sizeof(r), 1, file);

						u32 change_to_len;
						fread(&change_to_len, sizeof(change_to_len), 1, file);
						char* change_to = malloc(change_to_len + 1);
						change_to[change_to_len] = '\0';
						fread(change_to, 1, change_to_len, file);

						u32 entrance_len;
						fread(&entrance_len, sizeof(entrance_len), 1, file);
						char* entrance = malloc(entrance_len + 1);
						entrance[entrance_len] = '\0';
						fread(entrance, 1, entrance_len, file);

						room->transition_triggers[ii] = (struct transition_trigger) {
							.rect = r,
							.change_to = change_to,
							.entrance = entrance
						};
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

	if (room->transition_triggers) {
		for (u32 i = 0; i < room->transition_trigger_count; i++) {
			free(room->transition_triggers[i].change_to);
			free(room->transition_triggers[i].entrance);
		}

		free(room->transition_triggers);
	}

	free_table(room->entrances);

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

static bool rect_overlap(struct rect a, struct rect b, v2i* normal) {
	if (!(
		a.x + a.w > b.x &&
		a.y + a.h > b.y &&
		a.x < b.x + b.w &&
		a.y < b.y + b.h)) {
		*normal = make_v2i(0, 0);
		return false;
	}

	i32 right  = (a.x + a.w) - b.x;
	i32 left   = (b.x + b.w) - a.x;
	i32 top    = (b.y + b.h) - a.y;
	i32 bottom = (a.y + a.h) - b.y;

	i32 overlap[] = { right, left, top, bottom };

	qsort(overlap, sizeof(overlap) / sizeof(*overlap), sizeof(i32),
		lambda(i32 _(const void* a, const void* b) {
			return *(i32*)a > *(i32*)b;
		}));

	*normal = make_v2i(0, 0);
	if (overlap[0]        == abs(right)) {
		normal->x =  1;
	} else if (overlap[0] == abs(left)) {
		normal->x = -1;
	} else if (overlap[0] == abs(bottom)) {
		normal->y =  1;
	} else if (overlap[0] == abs(top)) {
		normal->y = -1;
	}

	return true;
}

void handle_body_collisions(struct room** room_ptr, struct rect collider, v2f* position, v2f* velocity) {
	struct room* room = *room_ptr;

	struct rect body_rect = {
		.x = collider.x + position->x,
		.y = collider.y + position->y,
		.w = collider.w, .h = collider.h
	};

	for (u32 i = 0; i < room->box_collider_count; i++) {
		struct rect rect = {
			.x = room->box_colliders[i].x * sprite_scale,
			.y = room->box_colliders[i].y * sprite_scale,
			.w = room->box_colliders[i].w * sprite_scale,
			.h = room->box_colliders[i].h * sprite_scale,
		};

		v2i normal;
		if (rect_overlap(body_rect, rect, &normal)) {
			if (normal.x == 1) {
				position->x = ((float)rect.x - body_rect.w) - collider.x;
				velocity->x = 0.0f;
			} else if (normal.x == -1) {
				position->x = ((float)rect.x + rect.w) - collider.x;
				velocity->x = 0.0f;
			} else if (normal.y == 1) {
				position->y = ((float)rect.y - body_rect.h) - collider.y;
				velocity->y = 0.0f;
			} else if (normal.y == -1) {
				position->y = ((float)rect.y + rect.h) - collider.y;
				velocity->y = 0.0f;
			}
		}
	}
}

void handle_body_transitions(struct room** room_ptr, struct rect collider, v2f* position) {
	struct room* room = *room_ptr;

	struct rect body_rect = {
		.x = collider.x + position->x,
		.y = collider.y + position->y,
		.w = collider.w, .h = collider.h
	};

	struct transition_trigger* transition = null;
	for (u32 i = 0; i < room->transition_trigger_count; i++) {
		struct rect rect = {
			.x = room->transition_triggers[i].rect.x * sprite_scale,
			.y = room->transition_triggers[i].rect.y * sprite_scale,
			.w = room->transition_triggers[i].rect.w * sprite_scale,
			.h = room->transition_triggers[i].rect.h * sprite_scale,
		};

		v2i normal;
		if (rect_overlap(body_rect, rect, &normal)) {
			transition = room->transition_triggers + i;
			break;
		}
	}

	if (transition) {
		char* change_to = copy_string(transition->change_to);
		char* entrance = copy_string(transition->entrance);

		free_room(room);
		*room_ptr = load_room(change_to);
		room = *room_ptr;

		v2i* entrance_pos = (v2i*)table_get(room->entrances, entrance);
		if (entrance_pos) {
			position->x = entrance_pos->x * sprite_scale - (collider.w / 2);
			position->y = entrance_pos->y * sprite_scale - collider.h;

			logic_store->camera_position = *position;
		} else {
			fprintf(stderr, "Failed to locate entrance with name `%s'\n", entrance);
		}

		free(change_to);
		free(entrance);
	}
}

bool rect_room_overlap(struct room* room, struct rect rect, v2i* normal) {
	for (u32 i = 0; i < room->box_collider_count; i++) {
		struct rect r = {
			.x = room->box_colliders[i].x * sprite_scale,
			.y = room->box_colliders[i].y * sprite_scale,
			.w = room->box_colliders[i].w * sprite_scale,
			.h = room->box_colliders[i].h * sprite_scale,
		};

		if (rect_overlap(rect, r, normal)) {
			return true;
		}
	}

	return false;
}
