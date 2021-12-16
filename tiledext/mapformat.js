function fwrite_uint(val, file) {
	var buf = new ArrayBuffer(4);
	var view = new Uint32Array(buf);
	view[0] = val;
	file.write(buf);
}

function fwrite_int(val, file) {
	var buf = new ArrayBuffer(4);
	var view = new Int32Array(buf);
	view[0] = val;
	file.write(buf);
}

function fwrite_float(val, file) {
	var buf = new ArrayBuffer(4);
	var view = new Float32Array(buf);
	view[0] = val;
	file.write(buf);
}

function fwrite_string(string, file) {
	fwrite_uint(string.length, file);

	var string_buf = new ArrayBuffer(string.length);
	var string_view = new Uint8Array(string_buf);
	for (var i = 0; i < string.length; i++) {
		string_view[i] = string.charCodeAt(i);
	}
	file.write(string_buf);
}

var dat_format = {
	name: "OpenMV",
	extension: "dat",

	write:function(map, filename) {
		var file = new BinaryFile(filename, BinaryFile.WriteOnly);

		/* Write the name of the map. */
		var map_name = map.property("name")
		if (map_name == undefined || map_name == null) {
			var map_name_len_buf = new ArrayBuffer(4);
			var map_name_len_view = new Uint32Array(map_name_len_buf);
			map_name_len_view[0] = 0;
			file.write(map_name_len_buf);
		} else {
			var map_name_len_buf = new ArrayBuffer(4);
			var map_name_len_view = new Uint32Array(map_name_len_buf);
			map_name_len_view[0] = map_name.length;
			file.write(map_name_len_buf);

			var map_name_buf = new ArrayBuffer(map_name.length);
			var map_name_view = new Uint8Array(map_name_buf);
			for (var ii = 0; ii < map_name.length; ii++) {
				map_name_view[ii] = map_name.charCodeAt(ii);
			}
			file.write(map_name_buf);
		}

		var tilesets = map.usedTilesets();

		/* Write the amount of tilesets. */
		fwrite_uint(tilesets.length, file);

		/* Write the tilesets */
		for (var i = 0; i < tilesets.length; i++) {
			var tileset = tilesets[i];

			fwrite_string(tileset.name, file);

			var image_path = tileset.image.substring(
					tileset.image.lastIndexOf("res"),
					tileset.image.length);

			fwrite_string(image_path, file);

			fwrite_uint(tileset.tileCount, file);
			fwrite_uint(tileset.tileWidth, file);
			fwrite_uint(tileset.tileHeight, file);

			var animation_count = 0;
			for (var ii = 0; ii < tileset.tiles.length; ii++) {
				var tile = tileset.tiles[ii];

				if (tile.animated) {
					animation_count++;
				}
			}

			fwrite_uint(animation_count, file);

			for (var ii = 0; ii < tileset.tiles.length; ii++) {
				var tile = tileset.tiles[ii];

				if (tile.animated) {
					fwrite_uint(tile.frames.length, file);
					fwrite_uint(tile.id, file);
				}

				for (var iii = 0; iii < tile.frames.length; iii++) {
					fwrite_uint(tile.frames[iii].tileId, file);
					fwrite_uint(tile.frames[iii].duration, file);
				}
			}
		}

		/* Write the layer count */
		var layer_count_buf = new ArrayBuffer(4);
		var layer_count_view = new Uint32Array(layer_count_buf);
		layer_count_view[0] = map.layerCount;
		file.write(layer_count_buf);

		/* Write the layers */
		for (var i = 0; i < map.layerCount; i++) {
			var layer = map.layerAt(i);

			/* Write the layer name length, followed by the bytes of the layer name string. */
			var name_len_buf = new ArrayBuffer(4);
			var name_len_view = new Uint32Array(name_len_buf);
			name_len_view[0] = layer.name.length;
			file.write(name_len_buf);

			var name_buf = new ArrayBuffer(layer.name.length);
			var name_view = new Uint8Array(name_buf);
			for (var ii = 0; ii < layer.name.length; ii++) {
				name_view[ii] = layer.name.charCodeAt(ii);
			}
			file.write(name_buf);

			if (layer.isTileLayer) {
				/* Write the layer type ID */
				var type_buf = new ArrayBuffer(4);
				var type_view = new Int32Array(type_buf);
				type_view[0] = 0;
				file.write(type_buf);

				/* Write the width and height */
				var size_buf = new ArrayBuffer(8);
				var size_view = new Uint32Array(size_buf);
				size_view[0] = layer.width;
				size_view[1] = layer.height;
				file.write(size_buf);

				for (var y = 0; y < layer.height; y++) {
					for (var x = 0; x < layer.width; x++) {
						var tile_id = -1;
						var tile = layer.tileAt(x, y);
						if (tile) {
							tile_id = tile.id;
						}
						var tileset_idx = 0;
						if (tile_id >= 0) {
							for (var ii = 0; ii < tilesets.length; ii++) {
								if (tile.tileset == tilesets[ii]) {
									tileset_idx = ii;
									break;
								}
							}
						}

						/* Write the ID */
						var id_buf = new ArrayBuffer(2);
						var id_view = new Int16Array(id_buf);
						id_view[0] = tile_id;

						var set_id_buf = new ArrayBuffer(2);
						var set_id_view = new Int16Array(set_id_buf);
						set_id_view[0] = tileset_idx;

						file.write(id_buf);
						file.write(set_id_buf);
					}
				}
			} else if (layer.isObjectLayer) {
				/* Write the layer type ID */
				var type_buf = new ArrayBuffer(4);
				var type_view = new Int32Array(type_buf);
				type_view[0] = 1;
				file.write(type_buf);

				/* Write the object count */
				var obj_count_buf = new ArrayBuffer(4);
				var obj_count_view = new Uint32Array(obj_count_buf);
				obj_count_view[0] = layer.objectCount;
				file.write(obj_count_buf);

				for (var ii = 0; ii < layer.objectCount; ii++) {
					var obj = layer.objects[ii];

					fwrite_string(obj.name, file);

					/* Write the rectangle. */
					if (layer.name == "slopes") {
						for (var iii = 0; iii < 2; iii += 2) {
							var rect_buf = new ArrayBuffer(4*4);
							var rect_view = new Int32Array(rect_buf);
							rect_view[0] = obj.x + obj.polygon[iii].x;
							rect_view[1] = obj.y + obj.polygon[iii].y;
							rect_view[2] = obj.x + obj.polygon[iii + 1].x;
							rect_view[3] = obj.y + obj.polygon[iii + 1].y;
							file.write(rect_buf);
						}
					} else {
						var rect_buf = new ArrayBuffer(4*4);
						var rect_view = new Int32Array(rect_buf);
						rect_view[0] = obj.x;
						rect_view[1] = obj.y;
						rect_view[2] = obj.size.width;
						rect_view[3] = obj.size.height;
						file.write(rect_buf);
					}

					if (layer.name === "transition_triggers" || layer.name == "doors") {
						var change_to_prop = obj.property("change_to");
						var entrance_prop = obj.property("entrance");

						if (!change_to_prop || !entrance_prop) {
							print("Object `" + obj.name + "' doesn't have the correct properties to be a transition trigger.");
						} else {
							var change_to_len_buf = new ArrayBuffer(4);
							var change_to_len_view = new Uint32Array(change_to_len_buf);
							change_to_len_view[0] = change_to_prop.length;
							file.write(change_to_len_buf);

							var change_to_buf = new ArrayBuffer(change_to_prop.length);
							var change_to_view = new Uint8Array(change_to_buf);
							for (var iii = 0; iii < change_to_prop.length; iii++) {
								change_to_view[iii] = change_to_prop.charCodeAt(iii);
							}
							file.write(change_to_buf);

							var entrance_len_buf = new ArrayBuffer(4);
							var entrance_len_view = new Uint32Array(entrance_len_buf);
							entrance_len_view[0] = entrance_prop.length;
							file.write(entrance_len_buf);

							var entrance_buf = new ArrayBuffer(entrance_prop.length);
							var entrance_view = new Uint8Array(entrance_buf);
							for (var iii = 0; iii < entrance_prop.length; iii++) {
								entrance_view[iii] = entrance_prop.charCodeAt(iii);
							}
							file.write(entrance_buf);
						}
					} else if (layer.name == "enemies") {
						if (obj.property("path") != undefined && obj.property("path") != null) {
							fwrite_string(obj.property("path"), file);
						} else {
							fwrite_uint(0, file);
						}
					} else if (layer.name == "enemy_paths") {
						fwrite_uint(obj.polygon.length, file);

						for (var iii = 0; iii < obj.polygon.length; iii++) {
							fwrite_float(obj.x + obj.polygon[iii].x, file);
							fwrite_float(obj.y + obj.polygon[iii].y, file);
						}
					} else if (layer.name == "upgrade_pickups") {
						var prefix = obj.property("prefix");
						var item_name = obj.property("name");
						if (prefix == undefined || prefix == null) { prefix = "undefined" }
						if (item_name == undefined || item_name == null) { item_name = "undefined" }
						fwrite_string(prefix, file);
						fwrite_string(item_name, file);

						if (obj.name == "health_pack" || obj.name == "health_booster") {
							fwrite_uint(obj.property("id"), file);
						}
					} else if (layer.name == "dialogue_triggers") {
						fwrite_string(obj.property("script"), file);
					}
				}
			} else {
				/* Layer type ID is -1 because it's an unknown type */
				var type_buf = new ArrayBuffer(4);
				var type_view = new Int32Array(type_buf);
				type_view[0] = -1;
				file.write(type_buf);
			}
		}

		file.commit();
	}
}

tiled.registerMapFormat("dat", dat_format);
