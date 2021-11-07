#include "res.h"
#include "sprites.h"

static const char* texture_paths[] = {
	[texid_player] = "res/bmp/char.bmp"
};

struct sprite sprites[] = {
	{ 0 }
};

struct animated_sprite anim_sprites[] = {
	[animsprid_player_run_right] = {
		.texture = texid_player,
		.frames = { { 0, 0, 16, 16 }, { 16, 0, 16, 16 }, { 32, 0, 16, 16 } },
		.speed = 8.9,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
	[animsprid_player_run_left] = {
		.texture = texid_player,
		.frames = { { 0, 16, 16, 16 }, { 16, 16, 16, 16 }, { 32, 16, 16, 16 } },
		.speed = 8.9,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 }
	},
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
