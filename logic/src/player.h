#pragma once

#include "audio.h"
#include "entity.h"
#include "maths.h"
#include "room.h"

enum {
	player_face_left = 0,
	player_face_right = 1
};

struct player {
	v2f velocity;

	struct rect collider;

	bool on_ground;
	bool dashing;

	double dash_time;
	double dash_cooldown_timer;
	double dash_fx_time;

	double shoot_timer;

	double accum;

	bool invul;
	double invul_timer;
	double invul_flash_timer;
	bool visible;

	i32 dash_count;

	v2i dash_dir;

	i32 face;

	u32 items;
	u32 hp_ups;

	i32 hp;
	i32 max_hp;

	struct audio_clip* jump_sound;
	struct audio_clip* shoot_sound;
	struct audio_clip* hurt_sound;
	struct audio_clip* fly_sound;
	struct audio_clip* upgrade_sound;
};

entity new_player_entity(struct world* world);
void player_system(struct world* world, struct renderer* renderer, struct room** room, double ts);
void hud_system(struct world* world, struct renderer* renderer);

enum {
	upgrade_jetpack = 1 << 0
};

struct upgrade {
	u32 id;
	struct rect collider;
};

struct health_upgrade {
	bool booster;
	u32 id;
	struct rect collider;
};

struct projectile {
	i32 face;
	double lifetime;
	float speed;
	struct rect collider;

	i32 damage;
};

void projectile_system(struct world* world, struct room* room, double ts);

struct anim_fx {
	u32 d;
};

void anim_fx_system(struct world* world, double ts);

entity new_impact_effect(struct world* world, v2f position);

struct damage_num_fx {
	char text[32];
	v2f position;
	float velocity;
	double timer;
};

void damage_fx_system(struct world* world, struct renderer* renderer, double ts);

entity new_damage_number(struct world* world, v2f position, i32 number);
