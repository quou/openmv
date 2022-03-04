#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "consts.h"
#include "core.h"
#include "coresys.h"
#include "dialogue.h"
#include "dynlib.h"
#include "enemy.h"
#include "keymap.h"
#include "logic_store.h"
#include "physics.h"
#include "player.h"
#include "res.h"
#include "room.h"
#include "savegame.h"
#include "shop.h"
#include "sprites.h"
#include "table.h"
#include "tiled.h"

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

struct tile_layer {
	struct tile* tiles;
	u32 w, h;
};

struct room {
	bool dark;

	struct tiled_map* map;

	struct tile_layer* layers;
	u32 layer_count;

	struct tileset* tilesets;
	u32 tileset_count;

	struct rect* box_colliders;
	u32 box_collider_count;

	struct rect* killzones;
	u32 killzone_count;

	struct rect* shops;
	u32 shop_count;

	v4i* slope_colliders;
	u32 slope_collider_count;

	struct table* entrances;
	struct table* paths;

	struct rect camera_bounds;

	struct transition_trigger* transition_triggers;
	u32 transition_trigger_count;

	struct door* doors;
	u32 door_count;

	struct dialogue* dialogue;
	u32 dialogue_count;

	struct font* name_font;

	u32 forground_index;

	char* path;
	char* name;

	f64 name_timer;

	struct world* world;

	bool transitioning_out;
	bool transitioning_in;

	f64 transition_timer;
	f64 transition_speed;

	char* transition_to;
	char* entrance;

	entity body;
	struct rect collider;

	struct room** ptr;
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

#define read_rects(d_, d_c_) \
	do { \
		d_ = core_alloc(sizeof(*(d_)) * object_count); \
		d_c_ = 0; \
		for (u32 ii = 0; ii < object_count; ii++) { \
			struct object* object = layer->as.object_layer.objects + ii; \
			\
			if (object->shape == object_shape_rect) { \
				(d_)[(d_c_)].x = (i32)object->as.rect.x * sprite_scale; \
				(d_)[(d_c_)].y = (i32)object->as.rect.y * sprite_scale; \
				(d_)[(d_c_)].w = (i32)object->as.rect.w * sprite_scale; \
				(d_)[(d_c_)].h = (i32)object->as.rect.h * sprite_scale; \
				\
				(d_c_)++; \
			} \
		} \
	} while (0)

struct room* load_room(struct world* world, const char* path) {
	struct room* room = core_calloc(1, sizeof(struct room));
	room->world = world;

	room->map = load_map(path);
	struct tiled_map* map = room->map;

	if (!room->map) {
		core_free(room);
		return null;
	}

	room->transitioning_in = true;
	room->transition_timer = 1.0;
	room->transition_speed = 5.0;

	room->name_font = load_font("res/CourierPrime.ttf", 25.0f);
	room->name_timer = 3.0;

	struct property* name_prop = table_get(map->properties, "name");
	if (name_prop && name_prop->type == prop_string) {
		room->name = name_prop->as.string;
	}

	struct property* dark_prop = table_get(map->properties, "dark");
	if (dark_prop && dark_prop->type == prop_bool) {
		room->dark = dark_prop->as.boolean;
	}

	room->path = copy_string(path);

	room->entrances = new_table(sizeof(v2i));
	room->paths = new_table(sizeof(struct path));

	room->tilesets = map->tilesets;
	room->tileset_count = map->tileset_count;

	/* Load the tile layers */
	for (u32 i = 0; i < map->layer_count; i++) {
		struct layer* layer = map->layers + i;

		switch (layer->type) {
			case layer_tiles: {
				u32 idx = room->layer_count++;
				room->layers = core_realloc(room->layers, room->layer_count * sizeof(struct tile_layer));

				room->layers[idx].tiles = layer->as.tile_layer.tiles;
				room->layers[idx].w = layer->as.tile_layer.w;
				room->layers[idx].h = layer->as.tile_layer.h;

				if (strcmp(layer->name, "forground") == 0) {
					room->forground_index = idx;
				}
			} break;
			case layer_objects: {
				u32 object_count = layer->as.object_layer.object_count;

				if (strcmp(layer->name, "collisions") == 0) {
					read_rects(room->box_colliders, room->box_collider_count);
				} else if (strcmp(layer->name, "killzones") == 0) {
					read_rects(room->killzones, room->killzone_count);
				} else if (strcmp(layer->name, "slopes") == 0) {
					room->slope_collider_count = 0;
					room->slope_colliders = null;
					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape == object_shape_polygon) {
							room->slope_colliders = core_realloc(room->slope_colliders,
								sizeof(*room->slope_colliders) * (room->slope_collider_count + object->as.polygon.count));
							for (u32 iii = 1; iii < object->as.polygon.count; iii += 1) {
								v2f start = object->as.polygon.points[iii - 1];
								v2f end   = object->as.polygon.points[iii];

								room->slope_colliders[room->slope_collider_count++] = make_v4i(start.x, start.y, end.x, end.y);
							}
						}
					}

					for (u32 ii = 0; ii < room->slope_collider_count; ii++) {
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
						} else if (start.y == end.y && start.x > end.x) {
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
					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;
						
						if (object->shape == object_shape_point) {
							v2i point;
							point.x = (i32)object->as.point.x * sprite_scale;
							point.y = (i32)object->as.point.y * sprite_scale;

							table_set(room->entrances, object->name, &point);
						}
					}
				} else if (strcmp(layer->name, "enemy_paths") == 0) {
					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape == object_shape_polygon) {
							struct path p = { 0 };
							p.count = object->as.polygon.count;
							p.points = core_calloc(p.count, sizeof(v2i));

							for (u32 iii = 0; iii < p.count; iii++) {
								p.points[iii].x = (i32)object->as.polygon.points[iii].x * sprite_scale;
								p.points[iii].y = (i32)object->as.polygon.points[iii].y * sprite_scale;
							}

							table_set(room->paths, object->name, &p);
						}
					}
				} else if (strcmp(layer->name, "enemies") == 0) {	
					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape == object_shape_point) {
							char* path_name = null;
							struct property* path_name_prop = table_get(object->properties, "path");
							if (path_name_prop && path_name_prop->type == prop_string) {
								path_name = path_name_prop->as.string;
							}

							v2f pos = v2f_mul(object->as.point, make_v2f(sprite_scale, sprite_scale));

							if (strcmp(object->name, "bat") == 0) {
								new_bat(world, room, pos, path_name);
							} else if (strcmp(object->name, "spider") == 0) {
								new_spider(world, room, pos);
							} else if (strcmp(object->name, "drill") == 0) {
								new_drill(world, room, pos);
							} else if (strcmp(object->name, "scav") == 0) {
								new_scav(world, room, pos);
							}
						}
					}
				} else if (strcmp(layer->name, "transition_triggers") == 0) {
					room->transition_triggers = core_calloc(object_count, sizeof(struct transition_trigger));

					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape == object_shape_rect) {
							struct rect r = {
								(i32)object->as.rect.x * sprite_scale,
								(i32)object->as.rect.y * sprite_scale,
								(i32)object->as.rect.w * sprite_scale,
								(i32)object->as.rect.h * sprite_scale
							};

							char* change_to = null;
							char* entrance = null;

							struct property* change_to_prop = table_get(object->properties, "change_to");
							if (change_to_prop && change_to_prop->type == prop_string) {
								change_to = change_to_prop->as.string;
							}

							struct property* entrance_prop = table_get(object->properties, "entrance");
							if (entrance_prop && entrance_prop->type == prop_string) {
								entrance = entrance_prop->as.string;
							}

							room->transition_triggers[room->transition_trigger_count++] = (struct transition_trigger) {
								.rect = r,
								.change_to = change_to,
								.entrance = entrance
							};
						}
					}
				} else if (strcmp(layer->name, "doors") == 0) {
					room->doors = core_calloc(object_count, sizeof(struct door));

					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape == object_shape_rect) {
							struct rect r = {
								(i32)object->as.rect.x * sprite_scale,
								(i32)object->as.rect.y * sprite_scale,
								(i32)object->as.rect.w * sprite_scale,
								(i32)object->as.rect.h * sprite_scale
							};

							char* change_to = null;
							char* entrance = null;

							struct property* change_to_prop = table_get(object->properties, "change_to");
							if (change_to_prop && change_to_prop->type == prop_string) {
								change_to = change_to_prop->as.string;
							}

							struct property* entrance_prop = table_get(object->properties, "entrance");
							if (entrance_prop && entrance_prop->type == prop_string) {
								entrance = entrance_prop->as.string;
							}

							room->doors[room->door_count++] = (struct door) {
								.rect = r,
								.change_to = change_to,
								.entrance = entrance
							};
						}
					}
				} else if (strcmp(layer->name, "save_points") == 0) {
					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape == object_shape_rect) {
							new_save_point(world, room, (struct rect) {
								object->as.rect.x * sprite_scale, object->as.rect.y * sprite_scale,
								object->as.rect.w * sprite_scale, object->as.rect.h * sprite_scale });
						}
					}
				} else if (strcmp(layer->name, "meta") == 0) {
					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (strcmp(object->name, "camera_bounds") == 0 && object->shape == object_shape_rect) {
							room->camera_bounds = (struct rect) {
								object->as.rect.x * sprite_scale, object->as.rect.y * sprite_scale,
								object->as.rect.w * sprite_scale, object->as.rect.h * sprite_scale
							};
						}
					}
				} else if (strcmp(layer->name, "upgrade_pickups") == 0) {
					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape != object_shape_rect) {
							continue;
						}

						char* obj_name = object->name;

						struct rect r = {
							object->as.rect.x,
							object->as.rect.y,
							object->as.rect.w,
							object->as.rect.h,
						};

						char* item_prefix = null;
						char* item_name = null;

						struct property* item_prefix_prop = table_get(object->properties, "prefix");
						if (item_prefix_prop && item_prefix_prop->type == prop_string) {
							item_prefix = item_prefix_prop->as.string;
						}

						struct property* item_name_prop = table_get(object->properties, "name");
						if (item_name_prop && item_name_prop->type == prop_string) {
							item_name = item_name_prop->as.string;
						}

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
							struct property* id_prop = table_get(object->properties, "id");
							if (id_prop && id_prop->type == prop_number) {
								upgrade_id = (i32)id_prop->as.number;
							}
						}

						struct player* player = get_component(world, logic_store->player, struct player);
						bool has_upgrade = (hp && player->hp_ups & upgrade_id) || (player->items & upgrade_id);

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
									.booster = booster);
								add_componentv(world, pickup, struct collider,
									.rect = { 0, 0, r.w * sprite_scale, r.h * sprite_scale });
							} else if (sprite_id != -1 && upgrade_id != -1) { 
								struct sprite sprite = get_sprite(sprite_id);

								entity pickup = new_entity(world);
								add_componentv(world, pickup, struct room_child, .parent = room);
								add_componentv(world, pickup, struct transform,
									.position = { r.x * sprite_scale, r.y * sprite_scale },
									.dimentions = { sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale });
								add_component(world, pickup, struct sprite, sprite);
								add_componentv(world, pickup, struct collider, .rect = { 0, 0, r.x * sprite_scale, r.y * sprite_scale });
								add_componentv(world, pickup, struct upgrade, .id = upgrade_id,
									.prefix = copy_string(item_prefix), .name = copy_string(item_name));
							}
						}
					}
				} else if (strcmp(layer->name, "dialogue_triggers") == 0) {
					room->dialogue = core_alloc(object_count * sizeof(struct dialogue));

					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape == object_shape_rect) {
							char* on_play_name = null;
							char* on_next_name = null;

							struct property* on_play_prop = table_get(object->properties, "on_play");
							if (on_play_prop && on_play_prop->type == prop_string) {
								on_play_name = on_play_prop->as.string;
							}
						
							struct property* on_next_prop = table_get(object->properties, "on_next");
							if (on_next_prop && on_next_prop->type == prop_string) {
								on_next_name = on_next_prop->as.string;
							}

							struct f32_rect r = object->as.rect;

							room->dialogue[room->dialogue_count++] = (struct dialogue) {
								.rect = { r.x * sprite_scale, r.y * sprite_scale, r.w * sprite_scale, r.h * sprite_scale },
								.on_play = on_play_name ? get_dialogue_fun(on_play_name) : null,
								.on_next = on_next_name ? get_dialogue_fun(on_next_name) : null,
							};
						}
					}
				} else if (strcmp(layer->name, "entity_spawners") == 0) {
					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape == object_shape_point) {
							f64 min = 0.0;
							f64 max = 0.0;

							struct property* min_prop = table_get(object->properties, "min_increment");
							if (min_prop && min_prop->type == prop_number) {
								min = min_prop->as.number;
							}

							struct property* max_prop = table_get(object->properties, "max_increment");
							if (max_prop && max_prop->type == prop_number) {
								max = max_prop->as.number;
							}

							struct property* entity_type_prop = table_get(object->properties, "entity_type");
							char* entity_type = null;
							if (entity_type_prop && entity_type_prop->type == prop_string) {
								entity_type = entity_type_prop->as.string;
							}

							u32 spawn_type = 0;
							if (strcmp(entity_type, "broken_robot") == 0) {
								spawn_type = spawn_type_broken_robot;
							}

							v2f r = object->as.point;

							entity e = new_entity(world);
							add_componentv(world, e, struct transform, .position = { r.x * sprite_scale, r.y * sprite_scale });
							add_componentv(world, e, struct entity_spawner, .spawn_type = spawn_type,
								.next_spawn = random_f64(min, max), .max_increment = (f64)max, .min_increment = (f64)min);
							add_componentv(world, e, struct room_child, .parent = room);
						}
					}
				} else if (strcmp(layer->name, "lava") == 0) {
					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape == object_shape_rect) {
							struct f32_rect r = object->as.rect;

							entity e = new_entity(world);
							add_componentv(world, e, struct lava, .collider = {
								r.x * sprite_scale, r.y * sprite_scale,
								r.w * sprite_scale, r.h * sprite_scale});
							add_componentv(world, e, struct room_child, .parent = room);
						}
					}
				} else if (strcmp(layer->name, "shops") == 0) {	
					read_rects(room->shops, room->shop_count);
				} else if (strcmp(layer->name, "lights") == 0) {
					for (u32 ii = 0; ii < object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						if (object->shape == object_shape_point) {
							f32 intensity = 1.0f;
							f32 range = 1000.0f;
							
							v2f r = object->as.point;

							entity e = new_entity(world);
							add_componentv(world, e, struct transform, .position = { r.x * sprite_scale, r.y * sprite_scale });
							add_componentv(world, e, struct light, .intensity = intensity, .range = range);
							add_componentv(world, e, struct room_child, .parent = room);
						}
					}
				} else {
					fprintf(stderr, "Warning: Unknown object layer type `%s'\n", layer->name);
				}
			} break;
			default: break;
		}
	}

	return room;
}

void free_room(struct room* room) {
	free_map(room->map);

	core_free(room->path);

	for (view(room->world, view, type_info(struct room_child))) {
		struct room_child* rc = view_get(&view, struct room_child);
		
		if (rc->parent == room) {
			destroy_entity(room->world, view.e);
		}
	}

	if (room->box_colliders) {
		core_free(room->box_colliders);
	}

	if (room->slope_colliders) {
		core_free(room->slope_colliders);
	}

	if (room->killzones) {
		core_free(room->killzones);
	}

	if (room->shops) {
		core_free(room->shops);
	}

	if (room->layers) {
		core_free(room->layers);
	}

	if (room->transition_triggers) {
		core_free(room->transition_triggers);
	}

	if (room->doors) {
		core_free(room->doors);
	}

	if (room->dialogue) {	
		core_free(room->dialogue);
	}

	free_table(room->entrances);

	for (struct table_iter i = new_table_iter(room->paths); table_iter_next(&i);) {
		struct path* p = i.value;
		core_free(p->points);
	}
	free_table(room->paths);

	core_free(room);
}

static void draw_tile_layer(struct room* room, struct renderer* renderer, struct tile_layer* layer, i32 idx) {
	if (idx == -1 || room->forground_index != idx) {
		v2i cam_pos = renderer->camera_pos;
		v2i cam_top_corner = v2i_sub(cam_pos, v2i_div(renderer->dimentions, make_v2i(2, 2)));
		v2i cam_bot_corner = v2i_add(cam_pos, v2i_div(renderer->dimentions, make_v2i(2, 2)));

		i32 start_x = cam_top_corner.x / (room->tilesets[0].tile_w * sprite_scale);
		i32 start_y = cam_top_corner.y / (room->tilesets[0].tile_h * sprite_scale);
		i32 end_x =   (cam_bot_corner.x / (room->tilesets[0].tile_w * sprite_scale)) + 1;
		i32 end_y =   (cam_bot_corner.y / (room->tilesets[0].tile_h * sprite_scale)) + 1;

		start_x = start_x < 0 ? 0 : start_x;
		start_y = start_y < 0 ? 0 : start_y;
		end_x = end_x > layer->w ? layer->w : end_x;
		end_y = end_y > layer->h ? layer->h : end_y;

		for (u32 y = start_y; y < end_y; y++) {
			for (u32 x = start_x; x < end_x; x++) {
				struct tile tile = layer->tiles[x + y * layer->w];
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

					if (set->animations[tile.id].exists) {
						struct animated_tile* at = set->animations + tile.id;

						quad.rect = (struct rect) {	
							.x = ((at->frames[at->current_frame] % (set->image->width  / set->tile_w)) * set->tile_w),
							.y = ((at->frames[at->current_frame] / (set->image->width / set->tile_h)) * set->tile_h),
							.w = set->tile_w,
							.h = set->tile_h
						};
					}

					renderer_push(renderer, &quad);
				}
			}
		}
	}
}

void draw_room(struct room* room, struct renderer* renderer, f64 ts) {
	if (room->name && room->name_timer >= 0.0) {
		room->name_timer -= ts;
	
		i32 win_w, win_h;
		query_window(main_window, &win_w, &win_h);

		i32 text_w = text_width(room->name_font, room->name);
		i32 text_h = text_height(room->name_font, room->name);

		render_text(logic_store->hud_renderer, room->name_font, room->name,
			(win_w / 2) - (text_w / 2), (win_h / 2) - (text_h / 2) - 40, make_color(0xffffff, 255));
	}

	for (u32 i = 0; i < room->layer_count; i++) {
		struct tile_layer* layer = room->layers + i;

		draw_tile_layer(room, renderer, layer, i);
	}
}

void update_room_light(struct room* room, struct renderer* renderer) {
	if (room->dark) {
		renderer->ambient_light = 0.0f;
	} else {
		renderer->ambient_light = 1.0f;
	}
}

void update_room(struct room* room, f64 ts, f64 actual_ts) {
	/* Update tile animations */
	for (u32 i = 0; i < room->tileset_count; i++) {
		for (u32 ii = 0; ii < room->tilesets[i].tile_count; ii++) {
			struct animated_tile* at = room->tilesets[i].animations + ii;
			if (at->exists) {
				at->timer += ts;
				if (at->timer > at->durations[at->current_frame]) {
					at->timer = 0.0;
					at->current_frame++;
					if (at->current_frame >= at->frame_count) {
						at->current_frame = 0;
					}
				}
			}
		}
	}

	/* Update spawners */
	for (view(room->world, view, type_info(struct transform), type_info(struct entity_spawner))) {
		struct transform* transform = view_get(&view, struct transform);
		struct entity_spawner* spawner = view_get(&view, struct entity_spawner);

		spawner->next_spawn -= ts;
		if (spawner->next_spawn <= 0.0) {
			spawner->next_spawn = random_f64(spawner->min_increment, spawner->max_increment);

			switch (spawner->spawn_type) {
				case spawn_type_broken_robot: {
					struct sprite sprite = get_sprite(sprid_broken_robot);

					entity e = new_entity(room->world);
					add_componentv(room->world, e, struct transform, .position = transform->position,
						.dimentions = { sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale });
					add_component(room->world, e, struct sprite, sprite);
					add_componentv(room->world, e, struct lava_interact, 0);
					add_componentv(room->world, e, struct collider,
						.rect = {
							-(sprite.rect.w * sprite_scale) / 2,
							-(sprite.rect.h * sprite_scale) / 2,
							sprite.rect.w * sprite_scale,
							sprite.rect.h * sprite_scale });
					add_componentv(room->world, e, struct room_child, .parent = room);
					add_componentv(room->world, e, struct fall, .mul = 1.0);
				} break;
				default: break;
			}
		}
	}

	/* TODO: Make a separate system function for this. */
	for (view(room->world, view, type_info(struct transform), type_info(struct fall))) {
		struct transform* transform = view_get(&view, struct transform);
		struct fall* fall = view_get(&view, struct fall);

		fall->velocity.y += g_gravity * ts * fall->mul;

		if (fall->velocity.y > g_max_gravity) {
			fall->velocity.y = g_max_gravity;
		}

		transform->position = v2f_add(transform->position, v2f_mul(fall->velocity, make_v2f(ts, ts)));
	}

	/* TODO: As above, same here. */
	for (view(room->world, view, type_info(struct lava))) {
		struct lava* lava = view_get(&view, struct lava);

		for (view(room->world, view, type_info(struct transform), type_info(struct lava_interact), type_info(struct collider))) {
			struct transform* transform = view_get(&view, struct transform);
			struct lava_interact* inter = view_get(&view, struct lava_interact);
			struct collider* collider = view_get(&view, struct collider);

			struct rect rect = {
				.x = transform->position.x + collider->rect.x,
				.y = transform->position.y + collider->rect.y,
				.w = collider->rect.w,
				.h = collider->rect.h,
			};
	
			if (rect_overlap(lava->collider, rect, null)) {
				play_audio_clip(logic_store->explosion_sound);

				struct sprite sprite = get_sprite(sprid_lava_particle);

				for (u32 i = 0; i < random_int(10, 20); i++) {
					entity e = new_entity(room->world);
					add_componentv(room->world, e, struct transform, .position = transform->position,
						.dimentions = { sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale });
					add_component(room->world, e, struct sprite, sprite);
					add_componentv(room->world, e, struct lava_particle,
						.velocity = { random_f64(-100, 100), random_f64(-600, -300) },
						.lifetime = 1.0,
						.rotation_inc = random_f64(-100, 100));
				}

				destroy_entity(room->world, view.e);
			}
		}
	}

	for (view(room->world, view, type_info(struct transform), type_info(struct lava_particle), type_info(struct sprite))) {
		struct transform* transform = view_get(&view, struct transform);
		struct lava_particle* particle = view_get(&view, struct lava_particle);
		struct sprite* sprite = view_get(&view, struct sprite);

		particle->velocity.y += g_gravity * ts;

		transform->position = v2f_add(transform->position, v2f_mul(particle->velocity, make_v2f(ts, ts)));

		transform->rotation += ts * particle->rotation_inc;

		particle->lifetime -= ts;
		if (particle->lifetime <= 0.0) {
			destroy_entity(room->world, view.e);
		}
	}

	for (u32 i = 0; i < room->dialogue_count; i++) {
		struct dialogue* d = room->dialogue + i;
		if (d->want_next && d->on_next) {
			d->on_next();
			d->want_next = false;
		}
	}

	if (room->transitioning_in) {
		logic_store->frozen = true;

		room->transition_timer -= actual_ts * room->transition_speed;
		if (room->transition_timer <= 0.0) {
			room->transitioning_in = false;
			logic_store->frozen = false;
		}
	} else if (room->transitioning_out) {
		logic_store->frozen = true;

		room->transition_timer += actual_ts * room->transition_speed;
		if (room->transition_timer >= 1.0) {
			struct room** ptr = room->ptr;

			struct world* world = room->world;

			entity body = room->body;
			struct rect collider = room->collider;
			char* change_to = room->transition_to;
			char* entrance = room->entrance;

			free_room(room);
			*ptr = load_room(world, change_to);
			room = *ptr;

			v2i* entrance_pos = (v2i*)table_get(room->entrances, entrance);
			if (entrance_pos) {
				v2f* position = &get_component(room->world, body, struct transform)->position;

				position->x = entrance_pos->x - (collider.w / 2);
				position->y = entrance_pos->y - collider.h;

				logic_store->camera_position = *position;
			} else {
				fprintf(stderr, "Failed to locate entrance with name `%s'\n", entrance);
			}

			core_free(change_to);
			core_free(entrance);

			room->transitioning_out = false;
			logic_store->frozen = false;	
		}
	}
}

void draw_room_forground(struct room* room, struct renderer* renderer, struct renderer* transition_renderer) {
	struct tile_layer* layer = room->layers + room->forground_index;

	draw_tile_layer(room, renderer, layer, -1);

	if (room->transitioning_in || room->transitioning_out) {
		i32 win_w, win_h;
		query_window(main_window, &win_w, &win_h);

		struct textured_quad quad = {
			.position = { 0, 0 },
			.dimentions = { win_w, win_h },
			.color = { 0, 0, 0, room->transition_timer > 1.0 ? 255 : (u8)(room->transition_timer * 255.0) },
		};

		renderer_push(transition_renderer, &quad);
	}
}

bool handle_body_collisions(struct room* room, struct rect collider, v2f* position, v2f* velocity) {
	struct rect body_rect = {
		.x = collider.x + position->x,
		.y = collider.y + position->y,
		.w = collider.w, .h = collider.h
	};

	bool collided = false;

	/* Resolve rectangle collisions, using a basic AABB vs AABB method. */
	for (u32 i = 0; i < room->box_collider_count; i++) {
		struct rect rect = room->box_colliders[i];

		v2i normal;
		if (rect_overlap(body_rect, rect, &normal)) {
			collided = true;
			if (normal.x == 1) {
				position->x = ((f32)rect.x - body_rect.w) - collider.x;
				velocity->x = 0.0f;
			} else if (normal.x == -1) {
				position->x = ((f32)rect.x + rect.w) - collider.x;
				velocity->x = 0.0f;
			} else if (normal.y == 1) {
				position->y = ((f32)rect.y - body_rect.h) - collider.y;
				velocity->y = 0.0f;
			} else if (normal.y == -1) {
				position->y = ((f32)rect.y + rect.h) - collider.y;
				velocity->y = 0.0f;
			}
		}
	}

	/* Resolve slope collisions.
	 *
	 * The collisons are checked for by checking the bottom-centre point of the collider
	 * against the right-angle triangle created by the two points that define the slope.
	 *
	 * Slope-intersection is then used to position the player on the `y' axis accordingly.
	 *
	 * NOTE: Slopes are buggy at low framerate, for unknown reasons. */
 	v2i check_point = make_v2i(body_rect.x + (body_rect.w / 2), body_rect.y + body_rect.h);
	for (u32 i = 0; i < room->slope_collider_count; i++) {
		v2i start = make_v2i(room->slope_colliders[i].x, room->slope_colliders[i].y);
		v2i end   = make_v2i(room->slope_colliders[i].z, room->slope_colliders[i].w);

		if (start.y == end.y) { /* Straight lines. */
			if (check_point.x > start.x && check_point.x < end.x &&
				check_point.y > start.y && check_point.y < start.y + 32) {
				position->y = ((start.y) - body_rect.h) - collider.y;
				velocity->y = 0.0f;
			}
		} else {
			if (point_vs_rtri(check_point, start, end)) {
		 		f32 col_centre = body_rect.x + (body_rect.w / 2);

				f32 slope = (f32)(end.y - start.y) / (f32)(end.x - start.x); /* Rise/Run. */
		 		f32 b = (start.y - (slope * start.x));

		 		/* y = mx + b */
				position->y = (((slope * col_centre) + b) - body_rect.h) - collider.y;
				velocity->y = 0.0f;

				collided = true;
			}
		}
	}

	return collided;
}

char* get_room_path(struct room* room) {
	return room->path;
}

void room_transition_to(struct room** room, entity body, struct rect collider, const char* path, const char* entrance) {
	(*room)->transition_to = copy_string(path);
	(*room)->entrance = copy_string(entrance);
	(*room)->ptr = room;

	(*room)->transitioning_out = true;
	(*room)->transition_timer = 0.0;

	(*room)->body = body;
	(*room)->collider = collider;
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

		if (rect_overlap(body_rect, rect, null)) {
			transition = room->transition_triggers + i;
			break;
		}
	}

	for (u32 i = 0; i < room->killzone_count; i++) {
		struct rect rect = room->killzones[i];

		if (rect_overlap(body_rect, rect, null)) {
			kill_player(room->world, body);
			return;
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

		for (view(room->world, view, type_info(struct save_point))) {
			struct save_point* sp = view_get(&view, struct save_point);

			if (rect_overlap(body_rect, sp->rect, null)) {
				ask_savegame();
			}
		}

		for (u32 i = 0; i < room->dialogue_count; i++) {
			struct dialogue* d = room->dialogue + i;

			if (rect_overlap(body_rect, room->dialogue[i].rect, null) && d->on_play) {
				d->want_next = true;
				d->on_play(&d->want_next);
			}
		}

		for (u32 i = 0; i < room->shop_count; i++) {
			if (rect_overlap(body_rect, room->shops[i], null)) {
				shopping();
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

		room_transition_to(room_ptr, body, collider, change_to, entrance);

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

 	v2i check_points[] = {
 		{ rect.x + (rect.w / 2), rect.y + rect.h },
 		{ rect.x,rect.y + rect.h },
 		{ rect.x + rect.w, rect.y + rect.h },
 	};

	for (u32 j = 0; j < sizeof(check_points) / sizeof(*check_points); j++) {
		v2i check_point = check_points[j];

		for (u32 i = 0; i < room->slope_collider_count; i++) {
			v2i start = make_v2i(room->slope_colliders[i].x, room->slope_colliders[i].y);
			v2i end   = make_v2i(room->slope_colliders[i].z, room->slope_colliders[i].w);

			if (start.y == end.y) { /* Straight lines. */
				if (check_point.x > start.x && check_point.x < end.x &&
					check_point.y > start.y && check_point.y < start.y + 32) {
					if (normal) { *normal = make_v2i(0, 1); }
					return true;
				}
			} else {
				if (point_vs_rtri(check_point, start, end)) {
					if (normal) { *normal = make_v2i(0, 1); }
					return true;
				}
			}
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
	entity e = new_entity(world);
	add_componentv(world, e, struct room_child, .parent = room);
	add_componentv(world, e, struct save_point, .rect = rect);

	return e;
}

static void on_dialogue_message_finish(void* ctx) {
	*(bool*)ctx = true;
}

struct dialogue_ask_udata {
	void* ctx;
	dialogue_ask_submit_func on_submit;
};

void on_dialogue_ask_finish(bool yes, void* uptr) {
	struct dialogue_ask_udata* udata = uptr;

	*(bool*)udata->ctx = true;

	udata->on_submit(yes, udata->ctx);

	core_free(udata);
}

void dialogue_ask(const char* text, dialogue_ask_submit_func on_submit, void* ctx) {
	struct dialogue_ask_udata* udata = core_alloc(sizeof(struct dialogue_ask_udata));

	udata->ctx = ctx;
	udata->on_submit = on_submit;

	prompt_ask(text, on_dialogue_ask_finish, udata);
}

void dialogue_message(const char* text, void* ctx) {
	message_prompt_ex(text, on_dialogue_message_finish, ctx);
}

struct player* get_player() {
	return get_component(logic_store->world, logic_store->player, struct player);
}
