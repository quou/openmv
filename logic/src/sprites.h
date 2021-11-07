#pragma once

#include "common.h"
#include "coresys.h"

/* Texture IDs */
enum {
	texid_player = 0
};

/* Sprite IDs  */
enum {
	animsprid_player_run_right = 0,
	animsprid_player_run_left,
};

void preload_sprites();
struct sprite get_sprite(u32 id);
struct animated_sprite get_animated_sprite(u32 id);
