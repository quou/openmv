#include "res.h"
#include "sprites.h"

static const char* texture_paths[] = {
	[texid_player]   = "res/bmp/char.bmp",
	[texid_upgrades] = "res/bmp/item.bmp",
	[texid_tsblue]   = "res/bmp/tsblue.bmp",
	[texid_fx]       = "res/bmp/fx.bmp",
	[texid_icon]     = "res/bmp/icon.bmp",
	[texid_arms]     = "res/bmp/arms.bmp",
	[texid_bad]      = "res/bmp/bad.bmp",
	[texid_back]     = "res/bmp/back.bmp"
};

struct sprite sprites[] = {
	[sprid_upgrade_jetpack] = {
		.texture = (struct texture*)texid_upgrades,
		.rect = { 0, 0, 16, 16 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_upgrade_health_pack] = {
		.texture = (struct texture*)texid_upgrades,
		.rect = { 32, 0, 16, 16 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_upgrade_health_booster] = {
		.texture = (struct texture*)texid_upgrades,
		.rect = { 16, 0, 16, 16 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_fx_jetpack] = {
		.texture = (struct texture*)texid_fx,
		.rect = { 0, 0, 8, 8 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_icon_ptr] = {
		.texture = (struct texture*)texid_icon,
		.rect = { 0, 0, 16, 16 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_projectile] = {
		.texture = (struct texture*)texid_arms,
		.rect = { 0, 0, 5, 2 },
		.origin = { 0.5f, 0.5f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[sprid_hud_hp] = {
		.texture = (struct texture*)texid_icon,
		.rect = { 19, 7, 36, 5 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_hud_hp_bar] = {
		.texture = (struct texture*)texid_icon,
		.rect = { 56, 7, 1, 3 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_coin] = {	
		.texture = (struct texture*)texid_icon,
		.rect = { 60, 7, 6, 6 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_door] = {
		.texture = (struct texture*)texid_tsblue,
		.rect = { 48, 32, 16, 16 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_door_open] = {
		.texture = (struct texture*)texid_tsblue,
		.rect = { 64, 32, 16, 16 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_terminal] = {
		.texture = (struct texture*)texid_upgrades,
		.rect = { 48, 11, 16, 21 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_coal_lump] = {
		.texture = (struct texture*)texid_upgrades,
		.rect = { 0, 16, 16, 16 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_spider] = {
		.texture = (struct texture*)texid_bad,
		.rect = { 0, 7, 11, 5 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_broken_robot] = {
		.texture = (struct texture*)texid_upgrades,
		.rect = { 32, 16, 16, 16 },
		.origin = { 0.5f, 0.5f },
		.color = { 255, 255, 255, 255 }
	},
	[sprid_lava_particle] = {
		.texture = (struct texture*)texid_fx,
		.rect = { 9, 0, 2, 2 },
		.origin = { 0.5f, 0.5f },
		.color = { 255, 255, 255, 255 }
	}
};

struct animated_sprite anim_sprites[] = {
	[animsprid_player_run_right] = {
		.id = animsprid_player_run_right,
		.texture = (struct texture*)texid_player,
		.frames = { { 0, 0, 16, 16 }, { 16, 0, 16, 16 }, { 32, 0, 16, 16 } },
		.speed = 8.9,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_run_left] = {
		.id = animsprid_player_run_left,
		.texture = (struct texture*)texid_player,
		.frames = { { 0, 16, 16, 16 }, { 16, 16, 16, 16 }, { 32, 16, 16, 16 } },
		.speed = 8.9,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_jump_right] = {
		.id = animsprid_player_jump_right,
		.texture = (struct texture*)texid_player,
		.frames = { { 32, 0, 16, 16 } },
		.speed = 0,
		.frame_count = 1,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_jump_left] = {
		.id = animsprid_player_jump_left,
		.texture = (struct texture*)texid_player,
		.frames = { { 32, 16, 16, 16 } },
		.speed = 0,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_fall_right] = {
		.id = animsprid_player_fall_right,
		.texture = (struct texture*)texid_player,
		.frames = { { 16, 0, 16, 16 } },
		.speed = 0,
		.frame_count = 1,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_fall_left] = {
		.id = animsprid_player_fall_left,
		.texture = (struct texture*)texid_player,
		.frames = { { 16, 16, 16, 16 } },
		.speed = 0,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_idle_right] = {
		.id = animsprid_player_idle_right,
		.texture = (struct texture*)texid_player,
		.frames = { { 0, 0, 16, 16 } },
		.speed = 0,
		.frame_count = 1,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_idle_left] = {
		.id = animsprid_player_idle_left,
		.texture = (struct texture*)texid_player,
		.frames = { { 0, 16, 16, 16 } },
		.speed = 0,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_run_right_up] = {
		.id = animsprid_player_run_right_up,
		.texture = (struct texture*)texid_player,
		.frames = { { 48, 0, 16, 16 }, { 64, 0, 16, 16 }, { 80, 0, 16, 16 } },
		.speed = 8.9,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_run_left_up] = {
		.id = animsprid_player_run_left_up,
		.texture = (struct texture*)texid_player,
		.frames = { { 48, 16, 16, 16 }, { 64, 16, 16, 16 }, { 80, 16, 16, 16 } },
		.speed = 8.9,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_jump_right_up] = {
		.id = animsprid_player_jump_right_up,
		.texture = (struct texture*)texid_player,
		.frames = { { 64, 0, 16, 16 } },
		.speed = 0,
		.frame_count = 1,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_jump_left_up] = {
		.id = animsprid_player_jump_left_up,
		.texture = (struct texture*)texid_player,
		.frames = { { 64, 16, 16, 16 } },
		.speed = 0,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_fall_right_up] = {
		.id = animsprid_player_fall_right_up,
		.texture = (struct texture*)texid_player,
		.frames = { { 80, 0, 16, 16 } },
		.speed = 0,
		.frame_count = 1,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_fall_left_up] = {
		.id = animsprid_player_fall_left_up,
		.texture = (struct texture*)texid_player,
		.frames = { { 80, 16, 16, 16 } },
		.speed = 0,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_idle_right_up] = {
		.id = animsprid_player_idle_right_up,
		.texture = (struct texture*)texid_player,
		.frames = { { 48, 0, 16, 16 } },
		.speed = 0,
		.frame_count = 1,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_player_idle_left_up] = {
		.id = animsprid_player_idle_left_up,
		.texture = (struct texture*)texid_player,
		.frames = { { 48, 16, 16, 16 } },
		.speed = 0,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_muzzle_flash] = {
		.id = animsprid_muzzle_flash,
		.texture = (struct texture*)texid_fx,
		.frames = { { 0, 8, 8, 8 }, { 8, 8, 8, 8 }, { 16, 8, 8, 8 }, { 24, 8, 8, 8 } },
		.speed = 25,
		.frame_count = 4,
		.origin = { 0.5f, 0.5f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_projectile_impact] = {
		.id = animsprid_projectile_impact,
		.texture = (struct texture*)texid_fx,
		.frames = { { 0, 16, 8, 8 }, { 8, 16, 8, 8 }, { 16, 16, 8, 8 }, { 24, 16, 8, 8 }, { 24, 8, 8, 8 } },
		.speed = 60,
		.frame_count = 5,
		.origin = { 0.5f, 0.5f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_bat] = {
		.id = animsprid_bat,
		.texture = (struct texture*)texid_bad,
		.frames = {
			{ 0, 0, 11, 7 },  { 11, 0, 11, 7 }, { 22, 0, 11, 7 },
			{ 33, 0, 11, 7 }, { 22, 0, 11, 7 }, { 11, 0, 11, 7 } 
		},
		.speed = 25,
		.frame_count = 6,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[animsprid_coin] = {
		.id = animsprid_coin,
		.texture = (struct texture*)texid_icon,
		.frames = {
			{ 60, 7, 6, 6 }, { 66, 7, 6, 6 }, { 72, 7, 6, 6 },
			{ 78, 7, 6, 6 }, { 84, 7, 6, 6 }
		},
		.speed = 25,
		.frame_count = 5,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[animsprid_blood] = {
		.id = animsprid_coin,
		.texture = (struct texture*)texid_fx,
		.frames = {
			{ 0, 24, 16, 16 }, { 16, 24, 16, 16 }, { 0, 40, 16, 16 },
			{ 16, 40, 16, 16 }, { 0, 56, 16, 16 }, { 16, 56, 16, 16 }
		},
		.speed = 25,
		.frame_count = 6,
		.origin = { 0.5f, 0.5f },
		.color = { 255, 255, 255, 255 }	
	},
	[animsprid_poof] = {
		.id = animsprid_poof,
		.texture = (struct texture*)texid_fx,
		.frames = {
			{ 0, 72,  16, 16 }, { 16, 72,  16, 16 },
			{ 0, 88,  16, 16 }, { 16, 88,  16, 16 },
			{ 0, 104, 16, 16 }, { 16, 104, 16, 16 },
			{ 0, 120, 16, 16 }, { 16, 120, 16, 16 }
		},
		.speed = 25,
		.frame_count = 8,
		.origin = { 0.5f, 0.5f },
		.color = { 255, 255, 255, 255 },
		.unlit = true
	},
	[animsprid_drill_left] = {
		.id = animsprid_drill_left,
		.texture = (struct texture*)texid_bad,
		.frames = {
			{ 48, 32, 16, 16 },
			{ 64, 32, 16, 16 },
			{ 80, 32, 16, 16 },
			{ 64, 32, 16, 16 },
			{ 96, 32, 16, 16 },
		},
		.speed = 8.5,
		.frame_count = 5,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[animsprid_drill_right] = {
		.id = animsprid_drill_right,
		.texture = (struct texture*)texid_bad,
		.frames = {
			{ 48, 48, 16, 16 },
			{ 64, 48, 16, 16 },
			{ 80, 48, 16, 16 },
			{ 64, 48, 16, 16 },
			{ 96, 48, 16, 16 },
		},
		.speed = 8.5,
		.frame_count = 5,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	}
};

void preload_sprites() {
	for (u32 i = 0; i < sizeof(sprites) / sizeof(*sprites); i++) {
		sprites[i].texture = load_texture(texture_paths[(u64)sprites[i].texture]);
	}

	for (u32 i = 0; i < sizeof(anim_sprites) / sizeof(*anim_sprites); i++) {
		anim_sprites[i].texture = load_texture(texture_paths[(u64)anim_sprites[i].texture]);
	}
}

struct sprite get_sprite(u32 id) {
	return sprites[id];
}

struct animated_sprite get_animated_sprite(u32 id) {
	return anim_sprites[id];
}

struct texture* get_texture(u32 id) {
	return load_texture(texture_paths[id]);
}
