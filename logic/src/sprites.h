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
	texid_bad,
	texid_back
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
	animsprid_player_run_right_up,
	animsprid_player_run_left_up,
	animsprid_player_jump_right_up,
	animsprid_player_jump_left_up,
	animsprid_player_fall_right_up,
	animsprid_player_fall_left_up,
	animsprid_player_idle_right_up,
	animsprid_player_idle_left_up,
	animsprid_muzzle_flash,
	animsprid_projectile_impact,
	animsprid_blood,
	animsprid_bat,
	animsprid_coin,
	animsprid_heart,
	animsprid_poof,
	animsprid_drill_left,
	animsprid_drill_right,
	animsprid_scav_run_left,
	animsprid_scav_run_right,
	animsprid_scav_idle_left,
	animsprid_scav_idle_right,
	sprid_upgrade_jetpack,
	sprid_upgrade_health_pack,
	sprid_upgrade_health_booster,
	sprid_fx_jetpack,
	sprid_icon_ptr,
	sprid_projectile_lvl1,
	sprid_projectile_lvl2,
	sprid_projectile_lvl3,
	sprid_hud_hp,
	sprid_hud_hp_bar,
	sprid_coin,
	sprid_door,
	sprid_door_open,
	sprid_terminal,
	sprid_coal_lump,
	sprid_spider,
	sprid_broken_robot,
	sprid_lava_particle
};

void preload_sprites();
struct sprite get_sprite(u32 id);
struct animated_sprite get_animated_sprite(u32 id);
struct texture* get_texture(u32 id);
