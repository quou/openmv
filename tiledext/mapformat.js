var dat_format = {
	name: "OpenMV",
	extension: "dat",

	write:function(map, filename) {
		var file = new BinaryFile(filename, BinaryFile.WriteOnly);

		var tilesets = map.usedTilesets();

		/* Write the amount of tilesets. */
		var tileset_count_buf = new ArrayBuffer(4);
		var tileset_count_view = new Uint32Array(tileset_count_buf);
		tileset_count_view[0] = tilesets.length;
		file.write(tileset_count_buf);

		/* Write the tilesets */
		for (var i = 0; i < tilesets.length; i++) {
			var tileset = tilesets[i];

			/* Write the name length, followed by the name string. */
			var name_len_buf = new ArrayBuffer(4);
			var name_len_view = new Uint32Array(name_len_buf);
			name_len_view[0] = tileset.name.length;
			file.write(name_len_buf);

			var name_buf = new ArrayBuffer(tileset.name.length);
			var name_view = new Uint8Array(name_buf);
			for (var ii = 0; ii < tileset.name.length; ii++) {
				name_view[ii] = tileset.name.charCodeAt(ii);
			}
			file.write(name_buf);

			var image_path = tileset.image.substring(
					tileset.image.lastIndexOf("res"),
					tileset.image.length);

			/* Write the image path length, followed by the image path string. */
			var image_path_len_buf = new ArrayBuffer(4);
			var image_path_len_view = new Uint32Array(image_path_len_buf);
			image_path_len_view[0] = image_path.length;
			file.write(image_path_len_buf);

			var image_path_buf = new ArrayBuffer(image_path.length);
			var image_path_view = new Uint8Array(image_path_buf);
			for (var ii = 0; ii < image_path.length; ii++) {
				image_path_view[ii] = image_path.charCodeAt(ii);
			}
			file.write(image_path_buf);

			/* Write tile size */
			var size_buf = new ArrayBuffer(8);
			var size_view = new Uint32Array(size_buf);
			size_view[0] = tileset.tileSize.width;
			size_view[1] = tileset.tileSize.height;
			file.write(size_buf);
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

					/* Write the object name */
					var obj_name_len_buf = new ArrayBuffer(4);
					var obj_name_len_view = new Uint32Array(obj_name_len_buf);
					obj_name_len_view[0] = obj.name.length;
					file.write(obj_name_len_buf);

					var obj_name_buf = new ArrayBuffer(obj.name.length);
					var obj_name_view = new Uint8Array(obj_name_buf);
					for (var iii = 0; iii < obj.name.length; iii++) {
						obj_name_view[iii] = obj.name.charCodeAt(iii);
					}
					file.write(obj_name_buf);

					/* Write the rectangle. */
					var rect_buf = new ArrayBuffer(4*4);
					var rect_view = new Int32Array(rect_buf);
					rect_view[0] = obj.x;
					rect_view[1] = obj.y;
					rect_view[2] = obj.size.width;
					rect_view[3] = obj.size.height;
					file.write(rect_buf);

					if (layer.name === "transition_triggers") {
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
