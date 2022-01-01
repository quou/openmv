function write_i16(val, file) {
	var buf = new ArrayBuffer(2);
	var view = new Int16Array(buf);
	view[0] = val;
	file.write(buf);
}

function write_i32(val, file) {
	var buf = new ArrayBuffer(4);
	var view = new Int32Array(buf);
	view[0] = val;
	file.write(buf);
}

function write_u32(val, file) {
	var buf = new ArrayBuffer(4);
	var view = new Uint32Array(buf);
	view[0] = val;
	file.write(buf);
}

function write_f32(val, file) {
	var buf = new ArrayBuffer(4);
	var view = new Float32Array(buf);
	view[0] = val;
	file.write(buf);
}

function write_f64(val, file) {
	var buf = new ArrayBuffer(8);
	var view = new Float64Array(buf);
	view[0] = val;
	file.write(buf);
}

function write_bool(val, file) {
	var buf = new ArrayBuffer(1);
	var view = new Int8Array(buf);
	view[0] = val;
	file.write(buf);
}

function write_str(string, file) {
	write_u32(string.length, file);

	var string_buf = new ArrayBuffer(string.length);
	var string_view = new Uint8Array(string_buf);
	for (var i = 0; i < string.length; i++) {
		string_view[i] = string.charCodeAt(i);
	}
	file.write(string_buf);
}

function get_local_fp(fp) {
	return fp.substring(
		fp.lastIndexOf("res"),
		fp.length);
}

/* Recurses the layers inside groups. */
function count_layers(map) {
	var count = 0;

	for (var i = 0; i < map.layerCount; i++) {
		var layer = map.layerAt(i);

		if (layer.isGroupLayer) {
			count += count_layers(layer);
		}
	}

	count += map.layerCount;

	return count;
}

function write_properties(obj, file) {
	var prop_count = 0;
	for (const prop of Object.entries(obj.resolvedProperties())) {
		prop_count++;
	}

	write_u32(prop_count, file);

	for (const prop of Object.entries(obj.resolvedProperties())) {
		write_str(prop[0], file);

		switch (typeof prop[1]) {
			case "number":
				write_i32(1, file);
				write_f64(prop[1], file);
				break;
			case "string":
				write_i32(2, file);
				write_str(prop[1], file);
				break;
			case "boolean":
				write_i32(0, file);
				write_bool(prop[1], file);
				break;
			default:
				write_i32(-1, file);
				console.error("Property type not supported.");
				break;
		}
	}
}

function write_layer(tilesets, layer, file) {
	if (layer.isGroupLayer) {
		for (var i = 0; i < layer.layerCount; i++) {
			write_layer(tilesets, layer.layerAt(i), file);
		}

		return;
	}

	write_str(layer.name, file);

	write_properties(layer, file);

	if (layer.isTileLayer) {
		write_i32(0, file);

		write_u32(layer.width, file);
		write_u32(layer.height, file);

		for (var y = 0; y < layer.height; y++) {
			for (var x = 0; x < layer.width; x++) {
				var tile_id = -1;
				var tile = layer.tileAt(x, y);
				if (tile) {
					tile_id = tile.id;
				}

				var tileset_id = 0;
				if (tile_id >= 0) {
					for (var i = 0; i < tilesets.length; i++) {
						if (tile.tileset == tilesets[i]) {
							tileset_id = i;
							break;
						}
					}
				}

				write_i16(tile_id, file);
				write_i16(tileset_id, file);
			}
		}
	} else if (layer.isObjectLayer) {
		write_i32(1, file);

		write_u32(layer.objectCount, file);

		for (var obj of layer.objects) {
			write_properties(obj, file);

			write_str(obj.name, file);
			write_str(obj.type, file);

			if (obj.polygon.length > 0) { /* Polygon */
				write_i32(2, file);
				write_u32(obj.polygon.length, file);
				for (var point of obj.polygon) {
					write_f32(obj.x + point.x, file);
					write_f32(obj.y + point.y, file);
				}
			} else if (obj.width == 0 && obj.height == 0) { /* Point */
				write_i32(1, file);
				write_f32(obj.x, file);
				write_f32(obj.y, file);
			} else { /* Rectangle */
				write_i32(0, file);
				write_f32(obj.x, file);
				write_f32(obj.y, file);
				write_f32(obj.width, file);
				write_f32(obj.height, file);
			}
		}
	} else {
		write_i32(-1, file);
	}
}

var dat_format = {
	name: "OpenMV",
	extension: "dat",

	write:function(map, filename) {
		var file = new BinaryFile(filename, BinaryFile.WriteOnly);

		write_properties(map, file);

		var tilesets = map.usedTilesets();

		/* Write the amount of tilesets. */
		write_u32(tilesets.length, file);

		/* Write the tilesets */
		for (var i = 0; i < tilesets.length; i++) {
			var tileset = tilesets[i];

			write_str(tileset.name, file);

			var image_path = get_local_fp(tileset.image);
			write_str(image_path, file);

			write_u32(tileset.tileCount, file);
			write_u32(tileset.tileWidth, file);
			write_u32(tileset.tileHeight, file);

			var animation_count = 0;
			for (var ii = 0; ii < tileset.tiles.length; ii++) {
				var tile = tileset.tiles[ii];

				if (tile.animated) {
					animation_count++;
				}
			}

			write_u32(animation_count, file);

			for (var ii = 0; ii < tileset.tiles.length; ii++) {
				var tile = tileset.tiles[ii];
				if (tile == null) { continue; }

				if (tile.animated) {
					write_u32(tile.frames.length, file);
					write_u32(tile.id, file);
				}

				for (var iii = 0; iii < tile.frames.length; iii++) {
					write_u32(tile.frames[iii].tileId, file);
					write_u32(tile.frames[iii].duration, file);
				}
			}
		}

		/* Write the layer count */
		write_u32(count_layers(map), file);

		/* Write the layers */
		for (var i = 0; i < map.layerCount; i++) {
			write_layer(tilesets, map.layerAt(i), file);
		}

		file.commit();
	}
}

tiled.registerMapFormat("dat", dat_format);
