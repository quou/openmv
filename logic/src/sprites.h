#pragma once

#include "common.h"
#include "coresys.h"
#include "room.h"

/* Texture IDs */
enum {
	texid_player = 0,
	texid_tsblue
};

/* Sprite IDs  */
enum {
	animsprid_player_run_right = 0,
	animsprid_player_run_left,
};

/* Tile IDs */
enum {
	tile_empty = 0,
	tsbluetile_centre,
	tsbluetile_corner_top_left,
	tsbluetile_corner_top_right,
	tsbluetile_corner_bot_left,
	tsbluetile_corner_bot_right,
	tsbluetile_wall_left,
	tsbluetile_wall_right,
	tsbluetile_top,
	tsbluetile_bot,
};

/* Tile Set IDs */
enum {
	tilesetid_blue = 0
};

void preload_sprites();
struct sprite get_sprite(u32 id);
struct animated_sprite get_animated_sprite(u32 id);
struct tile_set get_tile_set(u32 id);
