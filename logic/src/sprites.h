#pragma once

#include "common.h"
#include "coresys.h"
#include "room.h"
#include "video.h"

/* Texture IDs */
enum {
	texid_player = 0,
	texid_upgrades,
	texid_tsblue,
	texid_fx,
	texid_icon,
	texid_arms,
	texid_bad
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
	animsprid_muzzle_flash,
	animsprid_projectile_impact,
	animsprid_bat,
	sprid_upgrade_jetpack,
	sprid_fx_jetpack,
	sprid_icon_ptr,
	sprid_projectile
};

void preload_sprites();
struct sprite get_sprite(u32 id);
struct animated_sprite get_animated_sprite(u32 id);
struct texture* get_texture(u32 id);
