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

	bool on_ground;
	bool dashing;

	f64 dash_time;
	f64 dash_cooldown_timer;
	f64 dash_fx_time;

	f64 shoot_timer;

	f64 accum;

	bool invul;
	f64 invul_timer;
	f64 invul_flash_timer;
	bool visible;

	i32 dash_count;

	v2i dash_dir;

	i32 face;

	u32 items;
	u32 hp_ups;

	i32 hp;
	i32 max_hp;

	i32 money;

	f32 projectile_distance;
	f32 projectile_speed;
	f64 shoot_cooldown;
	i32 level;

	bool played_step;

	struct audio_clip* land_sound;
	struct audio_clip* shoot_sound;
	struct audio_clip* hurt_sound;
	struct audio_clip* fly_sound;
	struct audio_clip* upgrade_sound;
	struct audio_clip* heart_sound;
	struct audio_clip* step_sound;
};

entity new_player_entity(struct world* world);
void kill_player(struct world* world, entity player_handle);
void player_system(struct world* world, struct renderer* renderer, struct room** room, f64 ts);
void update_player_light(struct world* world, struct renderer* renderer, entity player);
void camera_system(struct world* world, struct renderer* renderer, struct room* room, f64 ts);
void hud_system(struct world* world, struct renderer* renderer);

entity new_heart(struct world* world, struct room* room, v2f position, i32 value);

enum {
	upgrade_jetpack = 1 << 1,
	item_coal_lump = 1 << 2,
};

struct upgrade {
	u32 id;

	char* prefix;
	char* name;
};

void on_upgrade_destroy(struct world* world, entity e, void* component);

struct health_upgrade {
	bool booster;
	u32 id;
	i32 value;

	v2f velocity;
};

struct coin_pickup {
	v2f velocity;
};

entity new_coin_pickup(struct world* world, struct room* room, v2f position);

struct projectile {
	i32 face;
	bool up;
	f32 distance;
	f32 speed;

	v2f original_position;

	entity from;

	i32 damage;
};

void projectile_system(struct world* world, struct room* room, f64 ts);

struct anim_fx {
	u32 d;
};

void anim_fx_system(struct world* world, f64 ts);

entity new_impact_effect(struct world* world, v2f position, u32 anim_id);

struct damage_num_fx {
	char text[32];
	f32 velocity;
	f64 timer;
};

void damage_fx_system(struct world* world, struct renderer* renderer, f64 ts);

entity new_damage_number(struct world* world, v2f position, i32 number);
