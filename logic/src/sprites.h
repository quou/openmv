#pragma once

#include "common.h"
#include "coresys.h"
#include "room.h"

/* Texture IDs */
enum {
	texid_player = 0,
	texid_upgrades,
	texid_tsblue
};

/* Sprite IDs  */
enum {
	animsprid_player_run_right = 0,
	animsprid_player_run_left,
	animsprid_player_jump_right,
	animsprid_player_jump_left,
	animsprid_player_fall_right,
	animsprid_player_fall_left,
	animsprid_player_idle_right,
	animsprid_player_idle_left,
	sprid_upgrade_jetpack
};

void preload_sprites();
struct sprite get_sprite(u32 id);
struct animated_sprite get_animated_sprite(u32 id);
