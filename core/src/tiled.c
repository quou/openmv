#include <stdio.h>

#include "core.h"
#include "res.h"
#include "tiled.h"

static char* read_string(struct file* file) {
	u32 len;
	file_read(&len, sizeof(len), 1, file);
	char* str = core_alloc(len + 1);
	str[len] = '\0';
	file_read(str, 1, len, file);
	return str;
}

static u32 read_u32(struct file* file) {
	u32 u;
	file_read(&u, sizeof(u), 1, file);
	return u;
}

static i32 read_i32(struct file* file) {
	i32 i;
	file_read(&i, sizeof(i), 1, file);
	return i;
}

static i16 read_i16(struct file* file) {
	i16 i;
	file_read(&i, sizeof(i), 1, file);
	return i;
}

static float read_f32(struct file* file) {	
	float f;
	file_read(&f, sizeof(f), 1, file);
	return f;
}

static double read_f64(struct file* file) {	
	double f;
	file_read(&f, sizeof(f), 1, file);
	return f;
}

static bool read_bool(struct file* file) {
	bool b;
	file_read(&b, sizeof(b), 1, file);
	return b;
}

static struct table* read_properties(struct file* file) {
	struct table* t = new_table(sizeof(struct property));

	u32 count = read_u32(file);

	for (u32 i = 0; i < count; i++) {
		struct property prop = { 0 };

		char* name = read_string(file);

		prop.type = read_i32(file);

		switch (prop.type) {
			case prop_bool:
				prop.as.boolean = read_bool(file);
				break;
			case prop_number:
				prop.as.number = read_f64(file);
				break;
			case prop_string:
				prop.as.string = read_string(file);
				break;
			default:
				break;
		}

		table_set(t, name, &prop);

		core_free(name);
	}

	return t;
}

struct tiled_map* load_map(const char* filename) {
	struct file file = file_open(filename);
	if (!file_good(&file)) {
		fprintf(stderr, "Failed to open file `%s'.", filename);
		return null;
	}

	struct tiled_map* map = core_alloc(sizeof(struct tiled_map));

	map->properties = read_properties(&file);

	/* Load the tilesets. */
	map->tileset_count = read_u32(&file);
	map->tilesets = core_calloc(map->tileset_count, sizeof(struct tileset));

	for (u32 i = 0; i < map->tileset_count; i++) {
		struct tileset* current = map->tilesets + i;

		current->name = read_string(&file);

		u32 path_len;
		char* path = read_string(&file);
		current->image = load_texture(path);
		core_free(path);

		current->tile_count = read_u32(&file);

		current->tile_w = read_u32(&file);
		current->tile_h = read_u32(&file);

		current->animations = core_calloc(current->tile_count, sizeof(struct animated_tile));

		/* Load animations. */
		u32 animation_count = read_u32(&file);
		
		for (u32 ii = 0; ii < animation_count; ii++) {
			u32 frame_count = read_u32(&file);
			u32 id = read_u32(&file);

			struct animated_tile* tile = current->animations + id;
			tile->frame_count = frame_count;
			tile->timer = 0.0;
			tile->exists = true;
			tile->current_frame = 0;

			for (u32 iii = 0; iii < frame_count; iii++) {
				i32 tile_id = read_i32(&file);
				i32 duration = read_i32(&file);
				
				tile->frames[iii] = tile_id;
				tile->durations[iii] = (double)duration * 0.001;
			}
		}
	}

	map->layer_count = read_u32(&file);
	map->layers = core_calloc(map->layer_count, sizeof(struct layer));

	for (u32 i = 0; i < map->layer_count; i++) {
		struct layer* layer = map->layers + i;

		layer->name = read_string(&file);

		layer->properties = read_properties(&file);

		layer->type = read_i32(&file);

		switch (layer->type) {
			case layer_tiles: {
				layer->as.tile_layer.w = read_u32(&file);
				layer->as.tile_layer.h = read_u32(&file);

				layer->as.tile_layer.tiles = core_calloc(layer->as.tile_layer.w * layer->as.tile_layer.h, sizeof(struct tile));

				for (u64 y = 0; y < layer->as.tile_layer.h; y++) {
					for (u64 x = 0; x < layer->as.tile_layer.w; x++) {
						layer->as.tile_layer.tiles[x + y * layer->as.tile_layer.w] = (struct tile) {
							.id = read_i16(&file),
							.tileset_id = read_i16(&file)
						};
					}
				}
			} break;
			case layer_objects: {
				layer->as.object_layer.object_count = read_u32(&file);
				layer->as.object_layer.objects = core_calloc(layer->as.object_layer.object_count, sizeof(struct object));

				for (u32 ii = 0; ii < layer->as.object_layer.object_count; ii++) {
					struct object* object = layer->as.object_layer.objects + ii;

					object->properties = read_properties(&file);

					object->name = read_string(&file);
					object->type = read_string(&file);

					object->shape = read_i32(&file);

					switch (object->shape) {
						case object_shape_point:
							object->as.point.x = read_f32(&file);
							object->as.point.y = read_f32(&file);
							break;
						case object_shape_polygon: {
							object->as.polygon.count = read_u32(&file);
							object->as.polygon.points = core_calloc(object->as.polygon.count, sizeof(v2f));

							for (u32 iii = 0; iii < object->as.polygon.count; iii++) {
								object->as.polygon.points[iii].x = read_f32(&file);
								object->as.polygon.points[iii].y = read_f32(&file);
							}
						} break;
						case object_shape_rect:
							object->as.rect.x = read_f32(&file);
							object->as.rect.y = read_f32(&file);
							object->as.rect.w = read_f32(&file);
							object->as.rect.h = read_f32(&file);
							break;
						default: break;
					}
				}
			} break;
			default:
				fprintf(stderr, "Loading map `%s'; Unkown layer type ID %d\n", filename, layer->type);
				break;
		};
	}

	return map;
}

void free_map(struct tiled_map* map) {
	for (table_iter(map->properties, i)) {
		struct property* prop = i.value;

		if (prop->type == prop_string) {
			core_free(prop->as.string);
		}
	}
	free_table(map->properties);

	if (map->tilesets) {
		for (u32 i = 0; i < map->tileset_count; i++) {
			core_free(map->tilesets[i].name);
			core_free(map->tilesets[i].animations);
		}

		core_free(map->tilesets);
	}

	if (map->layers) {
		for (u32 i = 0; i < map->layer_count; i++) {
			struct layer* layer = map->layers + i;

			core_free(layer->name);

			for (table_iter(layer->properties, it)) {
				struct property* prop = it.value;

				if (prop->type == prop_string) {
					core_free(prop->as.string);
				}
			}
			free_table(layer->properties);

			switch (layer->type) {
				case layer_tiles:
					core_free(layer->as.tile_layer.tiles);
					break;
				case layer_objects: {
					for (u32 ii = 0; ii < layer->as.object_layer.object_count; ii++) {
						struct object* object = layer->as.object_layer.objects + ii;

						core_free(object->name);
						core_free(object->type);

						if (object->shape == object_shape_polygon) {
							core_free(object->as.polygon.points);
						}
					
						for (table_iter(object->properties, it)) {
							struct property* prop = it.value;

							if (prop->type == prop_string) {
								core_free(prop->as.string);
							}
						}
						free_table(object->properties);
					}
					core_free(layer->as.object_layer.objects);
				} break;
				default: break;
			}
		}

		core_free(map->layers);
	}

	core_free(map);
}
