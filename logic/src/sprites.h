#pragma once

#include "common.h"
#include "coresys.h"

/* Texture IDs */
enum {
	texid_player = 0
};

/* Sprite IDs  */
enum {
	animsprid_player_run = 0
};

void preload_sprites();
struct sprite get_sprite(u32 id);
struct animated_sprite get_animated_sprite(u32 id);
