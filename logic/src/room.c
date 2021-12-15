#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "consts.h"
#include "core.h"
#include "coresys.h"
#include "enemy.h"
#include "keymap.h"
#include "logic_store.h"
#include "physics.h"
#include "player.h"
#include "res.h"
#include "room.h"
#include "savegame.h"
#include "sprites.h"
#include "table.h"

struct tile {
	i16 id;
	i16 tileset_id;
};

struct tileset {
	struct texture* image;
	char* name;
	i32 tile_w, tile_h;
	i32 tile_count;
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

struct door {
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

	v4i* slope_colliders;
	u32 slope_collider_count;

	struct table* entrances;
	struct table* paths;

	struct rect camera_bounds;

	struct transition_trigger* transition_triggers;
	u32 transition_trigger_count;

	struct door* doors;
	u32 door_count;

	struct font* name_font;

	u32 forground_index;

	char* path;
	char* name;

	double name_timer;

	struct world* world;
};

static char* read_name(struct file* file) {
	u32 name_len;
	file_read(&name_len, sizeof(name_len), 1, file);
	char* name = core_alloc(name_len + 1);
	name[name_len] = '\0';
	file_read(name, 1, name_len, file);

	return name;
}

void skip_name(struct file* file) {
	u32 obj_name_len;

	file_read(&obj_name_len, sizeof(obj_name_len), 1, file);
	file_seek(file, file->cursor + obj_name_len);
}

struct room* load_room(struct world* world, const char* path) {
	struct room* room = core_calloc(1, sizeof(struct room));
	room->world = world;

	struct file file = file_open(path);
	if (!file_good(&file)) {
		fprintf(stderr, "Failed to open file `%s'.", path);
		core_free(room);
		return null;
	}

	room->name_font = load_font("res/DejaVuSansMono.ttf", 25.0f);
	room->name_timer = 3.0;

	room->name = read_name(&file);

	room->path = copy_string(path);

	room->entrances = new_table(sizeof(v2i));
	room->paths = new_table(sizeof(struct path));

	/* Load the tilesets. */
	file_read(&room->tileset_count, sizeof(room->tileset_count), 1, &file);
	room->tilesets = core_calloc(room->tileset_count, sizeof(struct tileset));

	for (u32 i = 0; i < room->tileset_count; i++) {
		struct tileset* current = room->tilesets + i;

		/* Read the name. */
		current->name = read_name(&file);

		/* Read the tileset image. */
		u32 path_len;
		file_read(&path_len, sizeof(path_len), 1, &file);
		char* path = core_alloc(path_len + 1);
		path[path_len] = '\0';
		file_read(path, 1, path_len, &file);
		current->image = load_texture(path);
		core_free(path);

		/* Read the tile count. */
		file_read(&current->tile_count, sizeof(current->tile_count), 1, &file);

		/* Read the tile width and height */
		file_read(&current->tile_w, sizeof(current->tile_w), 1, &file);
		file_read(&current->tile_h, sizeof(current->tile_h), 1, &file);
	}

	/* Load the tile layers */	
	file_read(&room->layer_count, sizeof(room->layer_count), 1, &file);
	room->layers = core_calloc(room->layer_count, sizeof(struct layer));

	for (u32 i = 0; i < room->layer_count; i++) {
		struct layer* layer = room->layers + i;

		/* Read the name. */
		layer->name = read_name(&file);

		if (strcmp(layer->name, "forground") == 0) {
			room->forground_index = i;
		}

		file_read(&layer->type, sizeof(layer->type), 1, &file);

		switch (layer->type) {
			case layer_tiles: {
				file_read(&layer->as.tile_layer.w, sizeof(layer->as.tile_layer.w), 1, &file);
				file_read(&layer->as.tile_layer.h, sizeof(layer->as.tile_layer.h), 1, &file);

				layer->as.tile_layer.tiles = core_alloc(sizeof(struct tile) * layer->as.tile_layer.w * layer->as.tile_layer.h);

				u32 w = layer->as.tile_layer.w;
				u32 h = layer->as.tile_layer.h;
				for (u32 y = 0; y < h; y++) {
					for (u32 x = 0; x < w; x++) {
						i16 id, tileset_idx = 0;

						file_read(&id, sizeof(id), 1, &file);
						file_read(&tileset_idx, sizeof(tileset_idx), 1, &file);

						layer->as.tile_layer.tiles[x + y * w] = (struct tile) {
							.id = id,
							.tileset_id = tileset_idx
						};
					}
				}
			} break;
			case layer_objects: {
				u32 object_count;
				file_read(&object_count, sizeof(object_count), 1, &file);

				if (strcmp(layer->name, "collisions") == 0) {
					room->box_collider_count = object_count;
					room->box_colliders = core_alloc(sizeof(*room->box_colliders) * object_count);
					for (u32 ii = 0; ii < object_count; ii++) {
						skip_name(&file);

						file_read(room->box_colliders + ii, sizeof(*room->box_colliders), 1, &file);

						room->box_colliders[ii].x *= sprite_scale;
						room->box_colliders[ii].y *= sprite_scale;
						room->box_colliders[ii].w *= sprite_scale;
						room->box_colliders[ii].h *= sprite_scale;
					}
				} else if (strcmp(layer->name, "slopes") == 0) {
					room->slope_collider_count = object_count;
					room->slope_colliders = core_alloc(sizeof(*room->slope_colliders) * object_count);
					for (u32 ii = 0; ii < object_count; ii++) {
						skip_name(&file);

						file_read(room->slope_colliders + ii, sizeof(*room->slope_colliders), 1, &file);

						/* Ensure that the start of the slope is alway smaller than the end on the `x' axis. */
						v2i start = make_v2i(room->slope_colliders[ii].x * sprite_scale, room->slope_colliders[ii].y * sprite_scale);
						v2i end = make_v2i(room->slope_colliders[ii].z * sprite_scale, room->slope_colliders[ii].w * sprite_scale);
						if (start.y > end.y) {
							i32 c = end.y;
							end.y = start.y;
							start.y = c;

							c = end.x;
							end.x = start.x;
							start.x = c;
						}

						room->slope_colliders[ii] = make_v4i(start.x, start.y, end.x, end.y);
					}
				} else if (strcmp(layer->name, "entrances") == 0) {
					struct rect r;
					for (u32 ii = 0; ii < object_count; ii++) {
						char* obj_name = read_name(&file);

						file_read(&r, sizeof(r), 1, &file);
						r.x *= sprite_scale;
						r.y *= sprite_scale;
						r.w *= sprite_scale;
						r.h *= sprite_scale;
						table_set(room->entrances, obj_name, &r);

						core_free(obj_name);
					}
				} else if (strcmp(layer->name, "enemy_paths") == 0) {
					struct rect r;
					for (u32 ii = 0; ii < object_count; ii++) {
						char* obj_name = read_name(&file);

						file_read(&r, sizeof(r), 1, &file);

						struct path p = { 0 };
						file_read(&p.count, sizeof(p.count), 1, &file);
						p.points = core_alloc(sizeof(v2f) * p.count);

						for (u32 iii = 0; iii < p.count; iii++) {
							file_read(&p.points[iii].x, sizeof(float), 1, &file);
							file_read(&p.points[iii].y, sizeof(float), 1, &file);

							p.points[iii] = v2f_mul(p.points[iii], make_v2f(sprite_scale, sprite_scale));
						}

						table_set(room->paths, obj_name, &p);

						core_free(obj_name);
					}
				} else if (strcmp(layer->name, "enemies") == 0) {
					struct rect r;
					for (u32 ii = 0; ii < object_count; ii++) {
						char* obj_name = read_name(&file);

						file_read(&r, sizeof(r), 1, &file);

						char* path_name = read_name(&file);
						bool has_path = false;
						if (strlen(path_name) > 0) {
							has_path = true;
						}

						if (strcmp(obj_name, "bat") == 0) {
							new_bat(world, room, make_v2f(r.x * sprite_scale, r.y * sprite_scale), has_path ? path_name : null);
						}

						core_free(obj_name);
					}
				} else if (strcmp(layer->name, "transition_triggers") == 0) {
					room->transition_triggers = core_alloc(sizeof(struct transition_trigger) * object_count);
					room->transition_trigger_count = object_count;

					struct rect r;
					for (u32 ii = 0; ii < object_count; ii++) {
						skip_name(&file);

						file_read(&r, sizeof(r), 1, &file);

						char* change_to = read_name(&file);
						char* entrance = read_name(&file);

						room->transition_triggers[ii] = (struct transition_trigger) {
							.rect = { r.x * sprite_scale, r.y * sprite_scale, r.w * sprite_scale, r.h * sprite_scale },
							.change_to = change_to,
							.entrance = entrance
						};
					}
				} else if (strcmp(layer->name, "doors") == 0) {
					room->doors = core_alloc(sizeof(struct door) * object_count);
					room->door_count = object_count;

					struct rect r;
					for (u32 ii = 0; ii < object_count; ii++) {
						skip_name(&file);

						file_read(&r, sizeof(r), 1, &file);

						char* change_to = read_name(&file);
						char* entrance = read_name(&file);

						room->doors[ii] = (struct door) {
							.rect = { r.x * sprite_scale, r.y * sprite_scale, r.w * sprite_scale, r.h * sprite_scale },
							.change_to = change_to,
							.entrance = entrance
						};
					}
				} else if (strcmp(layer->name, "save_points") == 0) {
					struct rect r;
					for (u32 ii = 0; ii < object_count; ii++) {
						skip_name(&file);

						file_read(&r, sizeof(r), 1, &file);

						new_save_point(world, room, (struct rect) {
							r.x * sprite_scale, r.y * sprite_scale,
							r.w * sprite_scale, r.h * sprite_scale });
					}
				} else if (strcmp(layer->name, "upgrade_pickups") == 0) {
					struct rect r;
					for (u32 ii = 0; ii < object_count; ii++) {
						char* obj_name = read_name(&file);

						file_read(&r, sizeof(r), 1, &file);
						char* item_prefix = read_name(&file);
						char* item_name = read_name(&file);

						bool hp = false;
						bool booster = false;
						i32 sprite_id = -1;
						i32 upgrade_id = -1;

						if (strcmp(obj_name, "jetpack") == 0) {
							sprite_id = sprid_upgrade_jetpack;
							upgrade_id = upgrade_jetpack;
						} else if (strcmp(obj_name, "health_pack") == 0) {
							sprite_id = sprid_upgrade_health_pack;
							hp = true;
						} else if (strcmp(obj_name, "health_booster") == 0) {
							sprite_id = sprid_upgrade_health_booster;
							hp = true;
							booster = true;
						}

						if (hp) {
							file_read(&upgrade_id, sizeof(upgrade_id), 1, &file);
						}

						bool has_upgrade = false;
						for (single_view(world, view, struct player)) {
							struct player* player = single_view_get(&view);
							
							if (hp && player->hp_ups & upgrade_id) {
								has_upgrade = true;
								break;
							} else if (player->items & upgrade_id) {
								has_upgrade = true;
								break;
							}
						}

						if (!has_upgrade) {
							if (hp && sprite_id != -1) {
								struct sprite sprite = get_sprite(sprite_id);

								entity pickup = new_entity(world);
								add_componentv(world, pickup, struct room_child, .parent = room);
								add_componentv(world, pickup, struct transform,
									.position = { r.x * sprite_scale, r.y * sprite_scale },
									.dimentions = { sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale });
								add_component(world, pickup, struct sprite, sprite);
								add_componentv(world, pickup, struct health_upgrade, .id = upgrade_id,
									.collider = { r.x * sprite_scale, r.y * sprite_scale, r.w * sprite_scale, r.h * sprite_scale },
									.booster = booster);
							} else if (sprite_id != -1 && upgrade_id != -1) { 
								struct sprite sprite = get_sprite(sprite_id);

								entity pickup = new_entity(world);
								add_componentv(world, pickup, struct room_child, .parent = room);
								add_componentv(world, pickup, struct transform,
									.position = { r.x * sprite_scale, r.y * sprite_scale },
									.dimentions = { sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale });
								add_component(world, pickup, struct sprite, sprite);
								add_componentv(world, pickup, struct upgrade, .id = upgrade_id, .collider = r,
									.prefix = copy_string(item_prefix), .name = copy_string(item_name));
							}
						}

						core_free(obj_name);
						core_free(item_prefix);
						core_free(item_name);
					}
				} else if (strcmp(layer->name, "meta") == 0) {
					struct rect r;
					for (u32 ii = 0; ii < object_count; ii++) {
						char* obj_name = read_name(&file);

						file_read(&r, sizeof(r), 1, &file);

						if (strcmp(obj_name, "camera_bounds") == 0) {
							room->camera_bounds = (struct rect) {
								r.x * sprite_scale, r.y * sprite_scale,
								r.w * sprite_scale, r.h * sprite_scale
							};
						}

						core_free(obj_name);
					}
				}
			} break;
			default: break;
		}
	}

	file_close(&file);

	return room;
}

void free_room(struct room* room) {
	core_free(room->path);
	core_free(room->name);

	for (single_view(room->world, view, struct room_child)) {
		struct room_child* rc = single_view_get(&view);
		
		if (rc->parent == room) {
			destroy_entity(room->world, view.e);
		}
	}

	if (room->tilesets) {
		for (u32 i = 0; i < room->tileset_count; i++) {
			core_free(room->tilesets[i].name);
		}

		core_free(room->tilesets);
	}

	if (room->layers) {
		for (u32 i = 0; i < room->layer_count; i++) {
			core_free(room->layers[i].name);

			switch (room->layers[i].type) {
				case layer_tiles:
					core_free(room->layers[i].as.tile_layer.tiles);
					break;
				default: break;
			}
		}

		core_free(room->layers);
	}

	if (room->box_colliders) {
		core_free(room->box_colliders);
	}

	if (room->transition_triggers) {
		for (u32 i = 0; i < room->transition_trigger_count; i++) {
			core_free(room->transition_triggers[i].change_to);
			core_free(room->transition_triggers[i].entrance);
		}

		core_free(room->transition_triggers);
	}

	if (room->doors) {
		for (u32 i = 0; i < room->door_count; i++) {
			core_free(room->doors[i].change_to);
			core_free(room->doors[i].entrance);
		}

		core_free(room->doors);
	}

	free_table(room->entrances);

	for (struct table_iter i = new_table_iter(room->paths); table_iter_next(&i);) {
		struct path* p = i.value;
		core_free(p->points);
	}
	free_table(room->paths);

	core_free(room);
}

void draw_room(struct room* room, struct renderer* renderer, double ts) {
	if (room->name_timer >= 0.0) {
		room->name_timer -= ts;
	
		i32 win_w, win_h;
		query_window(main_window, &win_w, &win_h);

		i32 text_w = text_width(room->name_font, room->name);
		i32 text_h = text_height(room->name_font);

		render_text(logic_store->ui_renderer, room->name_font, room->name,
			(win_w / 2) - (text_w / 2), (win_h / 2) - (text_h / 2) - 40, make_color(0xffffff, 255));
	}

	for (u32 i = 0; i < room->layer_count; i++) {
		struct layer* layer = room->layers + i;

		if (layer->type == layer_tiles && room->forground_index != i) {
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
								.x = ((tile.id % (set->image->width  / set->tile_w)) * set->tile_w),
								.y = ((tile.id / (set->image->width / set->tile_h)) * set->tile_h),
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

	for (u32 i = 0; i < room->door_count; i++) {
		struct sprite sprite = get_sprite(sprid_door);

		struct textured_quad quad = {
			.texture = sprite.texture,
			.position = { room->doors[i].rect.x, room->doors[i].rect.y },
			.dimentions = { sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale },
			.rect = sprite.rect,
			.color = sprite.color
		};

		renderer_push(renderer, &quad);
	}
}

void draw_room_forground(struct room* room, struct renderer* renderer) {
	struct layer* layer = room->layers + room->forground_index;

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
							.x = ((tile.id % (set->image->width  / set->tile_w)) * set->tile_w),
							.y = ((tile.id / (set->image->width / set->tile_h)) * set->tile_h),
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

void handle_body_collisions(struct room** room_ptr, struct rect collider, v2f* position, v2f* velocity) {
	struct room* room = *room_ptr;

	struct rect body_rect = {
		.x = collider.x + position->x,
		.y = collider.y + position->y,
		.w = collider.w, .h = collider.h
	};

	/* Resolve rectangle collisions, using a basic AABB vs AABB method. */
	for (u32 i = 0; i < room->box_collider_count; i++) {
		struct rect rect = room->box_colliders[i];

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

	/* Resolve slope collisions.
	 *
	 * The collisons are checked for by checking the bottom-centre point of the collider
	 * against the right-angle triangle created by the two points that define the slope.
	 *
	 * Slope-intersection is then used to position the player on the `y' access accordingly. */
 	v2i check_point = make_v2i(body_rect.x + (body_rect.w / 2), body_rect.y + body_rect.h);
	for (u32 i = 0; i < room->slope_collider_count; i++) {
		v2i start = make_v2i(room->slope_colliders[i].x, room->slope_colliders[i].y);
		v2i end   = make_v2i(room->slope_colliders[i].z, room->slope_colliders[i].w);

		if (point_vs_rtri(check_point, start, end)) {
		 	float col_centre = body_rect.x + (body_rect.w / 2);

			float slope = (float)(end.y - start.y) / (float)(end.x - start.x); /* Rise/Run. */
		 	float b = (start.y - (slope * start.x));

		 	/* y = mx + b */
			position->y = (((slope * col_centre) + b) - body_rect.h) - collider.y;
			velocity->y = 0.0f;
		}
	}
}

char* get_room_path(struct room* room) {
	return room->path;
}

void handle_body_interactions(struct room** room_ptr, struct rect collider, entity body, bool body_on_ground) {
	struct room* room = *room_ptr;

	assert(has_component(room->world, body, struct transform));

	v2f* position = &get_component(room->world, body, struct transform)->position;

	struct rect body_rect = {
		.x = collider.x + position->x,
		.y = collider.y + position->y,
		.w = collider.w, .h = collider.h
	};

	struct transition_trigger* transition = null;
	for (u32 i = 0; i < room->transition_trigger_count; i++) {
		struct rect rect = room->transition_triggers[i].rect;

		v2i normal;
		if (rect_overlap(body_rect, rect, &normal)) {
			transition = room->transition_triggers + i;
			break;
		}
	}

	struct door* door = null;
	if (body_on_ground && key_just_pressed(main_window, mapped_key("interact"))) {
		for (u32 i = 0; i < room->door_count; i++) {
			struct rect rect = room->doors[i].rect;

			if (rect_overlap(body_rect, rect, null)) {
				door = room->doors + i;
				break;
			}
		}

		for (single_view(room->world, view, struct save_point)) {
			struct save_point* sp = single_view_get(&view);

			if (rect_overlap(body_rect, sp->rect, null)) {
				ask_savegame();
			}
		}
	}

	char* change_to = null;
	char* entrance = null;

	if (transition) {
		change_to = copy_string(transition->change_to);
		entrance = copy_string(transition->entrance);
	} else if (door) {
		change_to = copy_string(door->change_to);
		entrance = copy_string(door->entrance);
	}

	if (change_to && entrance) {
		struct world* world = room->world;

		free_room(room);
		*room_ptr = load_room(world, change_to);
		room = *room_ptr;

		v2i* entrance_pos = (v2i*)table_get(room->entrances, entrance);
		if (entrance_pos) {
			position = &get_component(room->world, body, struct transform)->position;

			position->x = entrance_pos->x - (collider.w / 2);
			position->y = entrance_pos->y - collider.h;

			logic_store->camera_position = *position;
		} else {
			fprintf(stderr, "Failed to locate entrance with name `%s'\n", entrance);
		}

		core_free(change_to);
		core_free(entrance);
	}
}

bool rect_room_overlap(struct room* room, struct rect rect, v2i* normal) {
	for (u32 i = 0; i < room->box_collider_count; i++) {
		struct rect r = room->box_colliders[i];

		if (rect_overlap(rect, r, normal)) {
			return true;
		}
	}

 	v2i check_point = make_v2i(rect.x + (rect.w / 2), rect.y + rect.h);

	for (u32 i = 0; i < room->slope_collider_count; i++) {
		v2i start = make_v2i(room->slope_colliders[i].x, room->slope_colliders[i].y);
		v2i end   = make_v2i(room->slope_colliders[i].z, room->slope_colliders[i].w);

		if (point_vs_rtri(check_point, start, end)) {
			return true;
		}
	}

	return false;
}

v2i get_spawn(struct room* room) {
	v2i* entrance_pos = (v2i*)table_get(room->entrances, "spawn");
	if (entrance_pos) {
		return make_v2i(entrance_pos->x, entrance_pos->y);
	}

	return make_v2i(0, 0);
}

struct rect room_get_camera_bounds(struct room* room) {
	return room->camera_bounds;
}

struct path* get_path(struct room* room, const char* name) {
	return table_get(room->paths, name);
}

entity new_save_point(struct world* world, struct room* room, struct rect rect) {
	struct sprite sprite = get_sprite(sprid_terminal);

	entity e = new_entity(world);
	add_componentv(world, e, struct room_child, .parent = room);
	add_componentv(world, e, struct transform, .position = { rect.x, rect.y - 5 * sprite_scale },
		.dimentions = { sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale });
	add_component(world, e, struct sprite, sprite);
	add_componentv(world, e, struct room_child, .parent = room);
	add_componentv(world, e, struct save_point, .rect = rect);

	return e;
}
