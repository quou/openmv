#include <math.h>
#include <stdio.h>
#include <string.h>

#include "consts.h"
#include "coresys.h"
#include "enemy.h"
#include "fx.h"
#include "keymap.h"
#include "logic_store.h"
#include "physics.h"
#include "platform.h"
#include "player.h"
#include "res.h"
#include "savegame.h"
#include "sprites.h"

static struct rect prim_numbers[] = {
	{ 66, 0, 4, 5 },
	{ 19, 0, 4, 5 },
	{ 24, 0, 1, 5 },
	{ 26, 0, 4, 5 },
	{ 31, 0, 4, 5 },
	{ 36, 0, 4, 5 },
	{ 41, 0, 4, 5 },
	{ 46, 0, 4, 5 },
	{ 51, 0, 4, 5 },
	{ 56, 0, 4, 5 },
	{ 61, 0, 4, 5 }
};

struct player_constants {
	float move_speed;
	float jump_force;
	float gravity;
	float accel;
	float friction;
	float slow_friction;
	float dash_force;
	float low_jump_mul;
	i32 ground_hit_range;
	double max_dash;
	double dash_fx_interval;
	double dash_cooldown;

	double invul_time;
	double invul_flash_interval;

	double projectile_lifetime;
	double projectile_speed;

	double shoot_cooldown;

	v2f left_muzzle_pos;
	v2f right_muzzle_pos;

	v2f knockback;

	i32 max_air_dash;

	i32 default_hp;
	i32 health_pack_value;
	i32 health_boost_value;

	struct rect left_collider;
	struct rect right_collider;
};

const struct player_constants player_constants = {
	.move_speed = 300,
	.jump_force = -800,
	.gravity = 1500,
	.accel = 2000,
	.friction = 800,
	.slow_friction = 100,
	.dash_force = 1000,
	.ground_hit_range = 12,
	.max_dash = 0.15,
	.dash_fx_interval = 0.045,
	.dash_cooldown = 0.3,
	.low_jump_mul = 3000.0f,

	.invul_time = 1.5,
	.invul_flash_interval = 0.05,
	
	.projectile_lifetime = 1.0,
	.projectile_speed = 1000.0,

	.shoot_cooldown = 0.1,

	.left_muzzle_pos =  { 11 * sprite_scale, 11 * sprite_scale },
	.right_muzzle_pos = { 4  * sprite_scale, 11 * sprite_scale },

	.knockback = { 300.0f, 300.0f },

	.max_air_dash = 3,

	.default_hp = 3,
	.health_pack_value = 3,
	.health_boost_value = 3,

	.right_collider = { 4 * sprite_scale, 1 * sprite_scale, 9 * sprite_scale, 15 * sprite_scale },
	.left_collider =  { 3 * sprite_scale, 1 * sprite_scale, 9 * sprite_scale, 15 * sprite_scale }
};

static void on_player_die(bool yes, void* udata) {
	if (yes) {
		loadgame();
	} else {
		set_window_should_close(main_window, true);
	}
}

entity new_player_entity(struct world* world) {
	struct texture* tex = load_texture("res/bmp/char.bmp");

	entity e = new_entity(world);
	add_componentv(world, e, struct transform, .dimentions = { 64, 64 });
	add_componentv(world, e, struct player,
		.collider = player_constants.left_collider,
		.visible = true,

		.hp = player_constants.default_hp,
		.max_hp = player_constants.default_hp,

		.jump_sound = load_audio_clip("res/aud/jump.wav"),
		.shoot_sound = load_audio_clip("res/aud/shoot.wav"),
		.hurt_sound = load_audio_clip("res/aud/hurt.wav"),
		.fly_sound = load_audio_clip("res/aud/fly.wav"),
		.upgrade_sound = load_audio_clip("res/aud/upgrade.wav"),
		.heart_sound = load_audio_clip("res/aud/heart.wav"));
	add_component(world, e, struct animated_sprite, get_animated_sprite(animsprid_player_run_right));
	
	return e;
}

void player_system(struct world* world, struct renderer* renderer, struct room** room, double ts) {
	for (view(world, view,
			type_info(struct transform),
			type_info(struct player),
			type_info(struct animated_sprite))) {
		struct transform* transform = view_get(&view, struct transform);
		struct player* player = view_get(&view, struct player);
		struct animated_sprite* sprite = view_get(&view, struct animated_sprite);

		if (!player->dashing) {
			player->velocity.y += player_constants.gravity * ts;
		}

		if (key_pressed(main_window, mapped_key("right"))) {
			if (player->velocity.x < player_constants.move_speed) {
				player->velocity.x += player_constants.accel * ts;
			}
			
			player->face = player_face_right;
		} else if (key_pressed(main_window, mapped_key("left"))) {
			if (player->velocity.x > -player_constants.move_speed) {
				player->velocity.x -= player_constants.accel * ts;
			}

			player->face = player_face_left;
		} else {
			if (player->velocity.x > 1.0f) {
				player->velocity.x -= player_constants.friction * ts;
			} else if (player->velocity.x < -1.0f) {
				player->velocity.x += player_constants.friction * ts;
			}
		}

		player->dash_cooldown_timer += ts;
		if (player->items & upgrade_jetpack &&
			!player->on_ground &&
			player->dash_cooldown_timer > player_constants.dash_cooldown &&
			!player->dashing &&
			player->dash_count < player_constants.max_air_dash &&
			key_just_pressed(main_window, mapped_key("dash"))) {
			player->dashing = true;
			player->dash_time = 0.0;
			player->dash_count++;
			player->dash_cooldown_timer = 0.0;

			play_audio_clip(player->fly_sound);

			if (key_pressed(main_window, mapped_key("up"))) {
				player->velocity.y = -player_constants.dash_force;
				player->velocity.x = 0;
				player->dash_dir = make_v2i(0, -1);
			} else if (key_pressed(main_window, mapped_key("down"))) {
				player->velocity.y = player_constants.dash_force;
				player->velocity.x = 0;
				player->dash_dir = make_v2i(0, 1);
			} else if (player->face == player_face_left) {
				player->velocity.x = -player_constants.dash_force;
				player->velocity.y = 0;
				player->dash_dir = make_v2i(-1, 0);
			} else if (player->face == player_face_right) {
				player->velocity.x = player_constants.dash_force;
				player->velocity.y = 0;
				player->dash_dir = make_v2i(1, 0);
			}
		}

		if (player->face == player_face_left) {
			player->collider = player_constants.left_collider;
		} else {
			player->collider = player_constants.right_collider;
		}

		player->dash_time += ts;
		if (player->dashing) {
			player->dash_fx_time += ts;
			if (player->dash_fx_time >= player_constants.dash_fx_interval) {
				player->dash_fx_time = 0.0;
				new_jetpack_particle(world, v2f_add(transform->position,
					v2f_div(make_v2f(transform->dimentions.x, transform->dimentions.y), make_v2f(2, 2))));
			}

			if (player->dash_time >= player_constants.max_dash) {
				player->dashing = false;
				if (player->dash_dir.x > 0) {
					player->velocity.x = 0;
				} else if (player->dash_dir.x < 0) {
					player->velocity.x = 0;
				} else if (player->dash_dir.y > 0) {
					player->velocity.y = 0;
				} else if (player->dash_dir.y < 0) {
					player->velocity.y = 0;
				}
			}
		}

		transform->position = v2f_add(transform->position, v2f_mul(player->velocity, make_v2f(ts, ts)));

		handle_body_collisions(room, player->collider, &transform->position, &player->velocity);
		handle_body_interactions(room, player->collider, view.e, player->on_ground);

		struct rect player_rect = {
			(i32)transform->position.x + player->collider.x,
			(i32)transform->position.y + player->collider.y,
			player->collider.w, player->collider.h
		};
		if (player->on_ground && key_just_pressed(main_window, mapped_key("interact"))) {
			for (single_view(world, up_view, struct upgrade)) {
				struct upgrade* upgrade = single_view_get(&up_view);

				struct rect up_rect = {
					upgrade->collider.x * sprite_scale,
					upgrade->collider.y * sprite_scale,
					upgrade->collider.w * sprite_scale,
					upgrade->collider.h * sprite_scale,
				};

				if (rect_overlap(player_rect, up_rect, null)) {
					player->items |= upgrade->id;
					play_audio_clip(player->upgrade_sound);

					char buf[256];
					sprintf(buf, "Obtained %s %s.", upgrade->prefix, upgrade->name);
					message_prompt(buf);

					destroy_entity(world, up_view.e);
				}
			}
		}

		for (single_view(world, up_view, struct health_upgrade)) {
			struct health_upgrade* upgrade = single_view_get(&up_view);

			if (rect_overlap(player_rect, upgrade->collider, null)) {
				if (upgrade->booster) {
					if (player->on_ground && key_just_pressed(main_window, mapped_key("interact"))) {
						player->max_hp += player_constants.health_boost_value;
						player->hp = player->max_hp;

						char buf[256];
						sprintf(buf, "Max health increased by %d!", player_constants.health_boost_value);
						message_prompt(buf);

						play_audio_clip(player->upgrade_sound);
					} else {
						continue;
					}
				} else {
					player->hp += upgrade->value == 0 ? player_constants.health_pack_value : upgrade->value;

					if (player->hp > player->max_hp) {
						player->hp = player->max_hp;
					}

					play_audio_clip(player->heart_sound);
				}
				if (upgrade->id != 0) {
					player->hp_ups |= upgrade->id;
				}
				destroy_entity(world, up_view.e);
			}
		}

		for (view(world, c_view, type_info(struct transform), type_info(struct coin_pickup))) {
			struct transform* c_transform = view_get(&c_view, struct transform);
			struct coin_pickup* coin = view_get(&c_view, struct coin_pickup);

			coin->velocity.y += player_constants.gravity * ts;

			c_transform->position = v2f_add(c_transform->position, v2f_mul(coin->velocity, make_v2f(ts, ts)));
	
			handle_body_collisions(room, coin->collider, &c_transform->position, &coin->velocity);

			struct rect c_rect = {
				c_transform->position.x + coin->collider.x,
				c_transform->position.y + coin->collider.y,
				coin->collider.w, coin->collider.h
			};

			if (rect_overlap(player_rect, c_rect, null)) {
				player->money++;
				play_audio_clip(player->heart_sound);
				destroy_entity(world, c_view.e);
			}
		}

		if (!player->invul) {
			for (view(world, e_view, type_info(struct transform), type_info(struct enemy))) {
				struct transform* e_transform = view_get(&e_view, struct transform);
				struct enemy* enemy = view_get(&e_view, struct enemy);

				struct rect e_rect = {
					e_transform->position.x + enemy->collider.x,
					e_transform->position.y + enemy->collider.y,
					enemy->collider.w, enemy->collider.h
				};

				v2i n;
				if (rect_overlap(player_rect, e_rect, &n)) {
					player->velocity = v2f_mul(make_v2f(-n.x, -n.y), player_constants.knockback);

					player->invul = true;
					player->invul_timer = player_constants.invul_time;
					player->invul_flash_timer = player_constants.invul_flash_interval;
					player->visible = false;

					new_damage_number(world, transform->position, -enemy->damage);

					player->hp -= enemy->damage;
					play_audio_clip(player->hurt_sound);

					if (player->hp <= 0) {
						prompt_ask("You have died. Want to retry?", on_player_die, null);
					}

					break;
				}
			}
		} else {
			player->invul_flash_timer -= ts;
			if (player->invul_flash_timer <= 0.0) {
				player->invul_flash_timer = player_constants.invul_flash_interval;
				player->visible = !player->visible;
			}

			player->invul_timer -= ts;
			if (player->invul_timer <= 0.0) {
				player->invul = false;
				player->visible = true;
			}
		}
		
		player->shoot_timer -= ts;
		if (key_just_pressed(main_window, mapped_key("fire")) && player->shoot_timer <= 0.0) {
			player->shoot_timer = player_constants.shoot_cooldown;

			play_audio_clip(player->shoot_sound);

			/* Spawn the projectile */
			struct sprite sprite = get_sprite(sprid_projectile);
			entity projectile = new_entity(world);
			add_componentv(world, projectile, struct transform,
				.z = 100,
				.position = v2f_add(transform->position,
					player->face == player_face_left ? player_constants.left_muzzle_pos : player_constants.right_muzzle_pos),
				.dimentions = v2i_mul(make_v2i(sprite_scale, sprite_scale), make_v2i(sprite.rect.w, sprite.rect.h)));
			add_component(world, projectile, struct sprite, sprite);
			add_componentv(world, projectile, struct projectile,
				.face = player->face,
				.lifetime = player_constants.projectile_lifetime,
				.speed = player_constants.projectile_speed,
				.damage = 4);

			/* Spawn the muzzle flash */
			struct animated_sprite f_sprite = get_animated_sprite(animsprid_muzzle_flash);
			entity flash = new_entity(world);
			add_componentv(world, flash, struct transform,
				.z = 100,
				.position = v2f_add(transform->position,
					player->face == player_face_right ?
						make_v2f(player_constants.left_muzzle_pos.x, player_constants.left_muzzle_pos.y)
						: make_v2f(player_constants.right_muzzle_pos.x, player_constants.right_muzzle_pos.y)),
				.dimentions = v2i_mul(make_v2i(sprite_scale, sprite_scale), make_v2i(8, 8)));
			add_component(world, flash, struct animated_sprite, f_sprite);
			add_componentv(world, flash, struct anim_fx);
		}

		/* Update pointers because the pools might have been reallocated. */
		transform = view_get(&view, struct transform);
		player = view_get(&view, struct player);
		sprite = view_get(&view, struct animated_sprite);

		{
			struct rect ground_test_rect = {
				transform->position.x + player->collider.x + 1,
				transform->position.y + player->collider.y + player->collider.h,
				player->collider.w - 2,
				player_constants.ground_hit_range
			};

			v2i normal;
			player->on_ground = rect_room_overlap(*room, ground_test_rect, &normal);
		}

		if (key_just_pressed(main_window, mapped_key("jump")) && player->on_ground) {
			player->velocity.y = player_constants.jump_force;
			play_audio_clip(player->jump_sound);
		}
		
		if (!player->on_ground && player->velocity.y < 0.0f && !key_pressed(main_window, mapped_key("jump"))) {
				player->velocity.y += player_constants.low_jump_mul * ts;
		}

		if (player->on_ground) {
			player->dash_count = 0;

			if (player->velocity.x > 50.0f || player->velocity.x < -50.0f) {
				if (player->face == player_face_left) {
					if (sprite->id != animsprid_player_run_left) {
						*sprite = get_animated_sprite(animsprid_player_run_left);
					}
				} else {
					if (sprite->id != animsprid_player_run_right) {
						*sprite = get_animated_sprite(animsprid_player_run_right);
					}
				}
			} else {
				if (player->face == player_face_left) {
					*sprite = get_animated_sprite(animsprid_player_idle_left);
				} else {
					*sprite = get_animated_sprite(animsprid_player_idle_right);
				}
			}
		} else {
			if (player->velocity.y < 0.0f) {
				if (player->face == player_face_left) {
					*sprite = get_animated_sprite(animsprid_player_jump_left);
				} else {
					*sprite = get_animated_sprite(animsprid_player_jump_right);
				}
			} else {
				if (player->face == player_face_left) {
					*sprite = get_animated_sprite(animsprid_player_fall_left);
				} else {
					*sprite = get_animated_sprite(animsprid_player_fall_right);
				}
			}
		}

		sprite->hidden = !player->visible;

		/* Camera movement. */
		float distance_to_player = sqrtf(powf(logic_store->camera_position.x - transform->position.x, 2)
			+ powf(logic_store->camera_position.y - transform->position.y, 2));

		v2f camera_dir = v2f_normalised(v2f_sub(transform->position, logic_store->camera_position));

		logic_store->camera_position.x += camera_dir.x * distance_to_player * ts * 10.0f;
		logic_store->camera_position.y += camera_dir.y * distance_to_player * ts * 10.0f;

		renderer->camera_pos = make_v2i((i32)logic_store->camera_position.x, (i32)logic_store->camera_position.y);

		v2i camera_corner = v2i_sub(renderer->camera_pos, v2i_div(renderer->dimentions, make_v2i(2, 2)));
		struct rect camera_bounds = room_get_camera_bounds(*room);
		if (camera_corner.x < camera_bounds.x) {
			renderer->camera_pos.x = camera_bounds.x + renderer->dimentions.x / 2;
		} else if (camera_corner.x + renderer->dimentions.x > camera_bounds.w)  {
			if (camera_bounds.w > renderer->dimentions.x) {
				renderer->camera_pos.x = camera_bounds.w - renderer->dimentions.x / 2;
			} else {
				renderer->camera_pos.x = camera_bounds.x + renderer->dimentions.x / 2;
			}
		}

		if (camera_corner.y <= camera_bounds.y) {
			renderer->camera_pos.y = camera_bounds.y + renderer->dimentions.y / 2;
		} else if (camera_corner.y + renderer->dimentions.y > camera_bounds.h)  {
			if (camera_bounds.h > renderer->dimentions.y) {
				renderer->camera_pos.y = camera_bounds.h - renderer->dimentions.y / 2;
			} else {
				renderer->camera_pos.y = camera_bounds.y + renderer->dimentions.y / 2;
			}
		}

		logic_store->camera_position = make_v2f(renderer->camera_pos.x, renderer->camera_pos.y);
	}
}

void hud_system(struct world* world, struct renderer* renderer) {
	i32 win_w, win_h;
	query_window(main_window, &win_w, &win_h);

	for (single_view(world, view, struct player)) {
		struct player* player = single_view_get(&view);

		struct sprite hp_sprite = get_sprite(sprid_hud_hp);
		struct sprite hp_bar_sprite = get_sprite(sprid_hud_hp_bar);
		struct sprite coin_sprite = get_sprite(sprid_coin);
		struct textured_quad hp_quad = {
			.texture = hp_sprite.texture,
			.position = { win_w - (hp_sprite.rect.w + 1) * sprite_scale, sprite_scale },
			.dimentions = { hp_sprite.rect.w * sprite_scale, hp_sprite.rect.h * sprite_scale },
			.rect = hp_sprite.rect,
			.color = hp_sprite.color,
		};
		struct textured_quad hp_bar_quad = {
			.texture = hp_sprite.texture,
			.position = {
				win_w - (hp_bar_sprite.rect.w + 35) * sprite_scale,
				2 * sprite_scale
			},
			.dimentions = {
				(i32)(((float)player->hp / (float)player->max_hp) * 25.0f) * sprite_scale,
				hp_bar_sprite.rect.h * sprite_scale
			},
			.rect = hp_bar_sprite.rect,
			.color = hp_bar_sprite.color,
		};
		struct textured_quad coin_quad = {
			.texture = coin_sprite.texture,
			.position = { win_w - (coin_sprite.rect.w + 1) * sprite_scale, sprite_scale * 2 + hp_quad.dimentions.y },
			.dimentions = { coin_sprite.rect.w * sprite_scale, coin_sprite.rect.h * sprite_scale },
			.rect = coin_sprite.rect,
			.color = coin_sprite.color,
		};
		renderer_push(renderer, &hp_quad);
		renderer_push(renderer, &hp_bar_quad);
		renderer_push(renderer, &coin_quad);

		char coin_text[32];
		sprintf(coin_text, "%d", player->money);

		i32 coin_text_w = 0;
		for (char* c = coin_text; *c; c++) {	
			char idx = (*c - '0') + 1;
			coin_text_w += prim_numbers[idx].w + 1;
		}

		i32 xpos = 0;
		for (char* c = coin_text; *c; c++) {
			char idx = (*c - '0') + 1;
			struct rect rect = prim_numbers[idx];

			struct textured_quad num_quad = {
				.texture = get_texture(texid_icon),
				.position = {
					win_w - ((coin_text_w + coin_sprite.rect.w + 1) - xpos) * sprite_scale,
					sprite_scale + coin_quad.dimentions.y + 3
				},
				.dimentions = { rect.w * sprite_scale, rect.h * sprite_scale },
				.rect = rect,
				.color = make_color(0xffffffff, 255),
			};
			renderer_push(renderer, &num_quad);

			xpos += rect.w + 1;
		}
	}
}

void projectile_system(struct world* world, struct room* room, double ts) {
	for (view(world, view, type_info(struct transform), type_info(struct projectile))) {
		struct transform* transform = view_get(&view, struct transform);
		struct projectile* projectile = view_get(&view, struct projectile);

		if (projectile->face == player_face_left) {
			transform->position.x -= projectile->speed * ts;
		} else {
			transform->position.x += projectile->speed * ts;
		}

		projectile->lifetime -= ts;
		if (projectile->lifetime <= 0.0) {
			destroy_entity(world, view.e);
		} else {
			struct rect rect = {
				projectile->collider.x + transform->position.x,
				projectile->collider.y + transform->position.y,
				projectile->collider.w, projectile->collider.h
			};

			if (rect_room_overlap(room, rect, null)) {
				new_impact_effect(world, transform->position, animsprid_projectile_impact);
				destroy_entity(world, view.e);
			}
		}
	}
}

entity new_impact_effect(struct world* world, v2f position, u32 anim_id) {
	struct animated_sprite f_sprite = get_animated_sprite(anim_id);
	entity e = new_entity(world);
	add_componentv(world, e, struct transform,
		.z = 100,
		.position = position,
		.dimentions = v2i_mul(make_v2i(sprite_scale, sprite_scale), make_v2i(f_sprite.frames[0].w, f_sprite.frames[0].h)));
	add_component(world, e, struct animated_sprite, f_sprite);
	add_componentv(world, e, struct anim_fx);
	return e;
}

void anim_fx_system(struct world* world, double ts) {
	for (view(world, view, type_info(struct transform), type_info(struct anim_fx), type_info(struct animated_sprite))) {
		struct transform* transform = view_get(&view, struct transform);
		struct animated_sprite* anim = view_get(&view, struct animated_sprite);

		if (anim->current_frame >= anim->frame_count - 1) {
			destroy_entity(world, view.e);
		}
	}
}

void damage_fx_system(struct world* world, struct renderer* renderer, double ts) {
	struct texture* atlas = get_texture(texid_icon);

	for (single_view(world, view, struct damage_num_fx)) {
		struct damage_num_fx* d = single_view_get(&view);

		d->velocity += 30.0 * ts;
		d->position.y -= d->velocity * ts;

		float x = 0.0f;

		for (char* c = d->text; *c; c++) {
			char idx = (*c - '0') + 1;
			struct rect rect = prim_numbers[idx];
			if (*c == '-') {
				rect = prim_numbers[0];
			}

			struct textured_quad quad = {
				.texture = atlas,
				.position = make_v2i(x + d->position.x, d->position.y),
				.dimentions = make_v2i(rect.w * sprite_scale, rect.h * sprite_scale),
				.rect = rect,
				.color = { 197, 53, 53, d->timer > 1.0 ? 255 : (i32)(d->timer * 255) }
			};

			x += rect.w * sprite_scale;

			renderer_push(renderer, &quad);
		}

		d->timer -= ts;
		if (d->timer <= 0.0) {
			destroy_entity(world, view.e);
		}
	}
}

entity new_damage_number(struct world* world, v2f position, i32 number) {
	entity e = new_entity(world);
	add_componentv(world, e, struct damage_num_fx,
		.position = position,
		.velocity = 30.0,
		.timer = 1.2);

	struct damage_num_fx* d = get_component(world, e, struct damage_num_fx);
	sprintf(d->text, "%d", number);

	return e;
}

entity new_coin_pickup(struct world* world, struct room* room, v2f position) {
	entity e = new_entity(world);
	
	struct animated_sprite sprite = get_animated_sprite(animsprid_coin);

	struct rect rect = sprite.frames[0];

	add_componentv(world, e, struct transform, .position = position,
		.dimentions = { rect.w * sprite_scale, rect.h * sprite_scale });
	add_component(world, e, struct animated_sprite, sprite);
	add_componentv(world, e, struct room_child, .parent = room);
	add_componentv(world, e, struct coin_pickup,
		.collider = { 0, 0, rect.w * sprite_scale, rect.h * sprite_scale });

	return e;
}

entity new_heart(struct world* world, struct room* room, v2f position, i32 value) {
	struct sprite sprite = get_sprite(sprid_upgrade_health_pack);

	entity pickup = new_entity(world);
	add_componentv(world, pickup, struct room_child, .parent = room);
	add_componentv(world, pickup, struct transform, .position = position,
		.dimentions = { sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale });
	add_component(world, pickup, struct sprite, sprite);
	add_componentv(world, pickup, struct health_upgrade, .id = 0,
		.collider = { position.x, position.y, sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale },
		.booster = false, .value = value == 0 ? player_constants.health_pack_value : value);

	return pickup;
}

void on_upgrade_destroy(struct world* world, entity e, void* component) {
	struct upgrade* upgrade = component;

	core_free(upgrade->name);
	core_free(upgrade->prefix);
}
