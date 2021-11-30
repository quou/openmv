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
#include "sprites.h"

struct player_constants {
	float move_speed;
	float jump_force;
	float gravity;
	float accel;
	float friction;
	float slow_friction;
	float dash_force;
	i32 ground_hit_range;
	double max_jump;
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

	struct rect left_collider;
	struct rect right_collider;
};

const struct player_constants player_constants = {
	.move_speed = 300,
	.jump_force = -350,
	.gravity = 1000,
	.accel = 1000,
	.friction = 1000,
	.slow_friction = 100,
	.dash_force = 1000,
	.ground_hit_range = 12,
	.max_jump = 0.3,
	.max_dash = 0.15,
	.dash_fx_interval = 0.045,
	.dash_cooldown = 0.3,

	.invul_time = 1.5,
	.invul_flash_interval = 0.05,
	
	.projectile_lifetime = 1.0,
	.projectile_speed = 1000.0,

	.shoot_cooldown = 0.1,

	.left_muzzle_pos =  { 12 * sprite_scale, 11 * sprite_scale },
	.right_muzzle_pos = { 3  * sprite_scale, 11 * sprite_scale },

	.knockback = { 300.0f, 300.0f },

	.max_air_dash = 3,

	.right_collider = { 4 * sprite_scale, 1 * sprite_scale, 9 * sprite_scale, 15 * sprite_scale },
	.left_collider =  { 3 * sprite_scale, 1 * sprite_scale, 9 * sprite_scale, 15 * sprite_scale }
};

entity new_player_entity(struct world* world) {
	struct texture* tex = load_texture("res/bmp/char.bmp");

	entity e = new_entity(world);
	add_componentv(world, e, struct transform, .dimentions = { 64, 64 });
	add_componentv(world, e, struct player,
		.position = { 128, 128 },
		.collider = player_constants.left_collider,
		.visible = true);
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
				new_jetpack_particle(world, make_v2i(
					transform->position.x + transform->dimentions.x / 2,
					transform->position.y + transform->dimentions.y / 2));
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

		player->position = v2f_add(player->position, v2f_mul(player->velocity, make_v2f(ts, ts)));

		handle_body_collisions(room, player->collider, &player->position, &player->velocity);
		handle_body_transitions(room, player->collider, &player->position);

		struct rect player_rect = {
			(i32)player->position.x + player->collider.x,
			(i32)player->position.y + player->collider.y,
			player->collider.w, player->collider.h
		};
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
				destroy_entity(world, up_view.e);
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

			/* Spawn the projectile */
			struct sprite sprite = get_sprite(sprid_projectile);
			entity projectile = new_entity(world);
			add_componentv(world, projectile, struct transform,
				.z = 100,
				.position = make_v2i(player->position.x, player->position.y),
				.dimentions = v2i_mul(make_v2i(sprite_scale, sprite_scale), make_v2i(sprite.rect.w, sprite.rect.h)));
			add_component(world, projectile, struct sprite, sprite);
			add_componentv(world, projectile, struct projectile,
				.face = player->face,
				.lifetime = player_constants.projectile_lifetime,
				.speed = player_constants.projectile_speed,
				.position = v2f_add(player->position,
					player->face == player_face_left ? player_constants.left_muzzle_pos : player_constants.right_muzzle_pos),
				.damage = 4);

			/* Spawn the muzzle flash */
			struct animated_sprite f_sprite = get_animated_sprite(animsprid_muzzle_flash);
			entity flash = new_entity(world);
			add_componentv(world, flash, struct transform,
				.z = 100,
				.position = v2i_add(transform->position,
					player->face == player_face_right ?
						make_v2i(player_constants.left_muzzle_pos.x, player_constants.left_muzzle_pos.y)
						: make_v2i(player_constants.right_muzzle_pos.x, player_constants.right_muzzle_pos.y)),
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
				player->position.x + player->collider.x + 1,
				player->position.y + player->collider.y + player->collider.h,
				player->collider.w - 2,
				player_constants.ground_hit_range
			};

			v2i normal;
			player->on_ground = rect_room_overlap(*room, ground_test_rect, &normal);
		}

		if (key_just_pressed(main_window, mapped_key("jump")) && player->on_ground) {
			player->velocity.y = player_constants.jump_force;
			player->jump_time = 0.0;
		}

		player->jump_time += ts;
		if (!player->on_ground && key_pressed(main_window, mapped_key("jump")) && player->jump_time < player_constants.max_jump) {
			player->velocity.y += player_constants.jump_force * 5 * ts;
		}

		if (player->on_ground) {
			player->dash_count = 0;

			if (player->velocity.x > 5.0f || player->velocity.x < -5.0f) {
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

		transform->position = make_v2i((i32)player->position.x, (i32)player->position.y);

		float distance_to_player = sqrtf(powf(logic_store->camera_position.x - player->position.x, 2)
			+ powf(logic_store->camera_position.y - player->position.y, 2));

		v2f camera_dir = v2f_normalised(v2f_sub(player->position, logic_store->camera_position));

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

void projectile_system(struct world* world, struct room* room, double ts) {
	for (view(world, view, type_info(struct transform), type_info(struct projectile))) {
		struct transform* transform = view_get(&view, struct transform);
		struct projectile* projectile = view_get(&view, struct projectile);

		if (projectile->face == player_face_left) {
			projectile->position.x -= projectile->speed * ts;
		} else {
			projectile->position.x += projectile->speed * ts;
		}

		transform->position = make_v2i(projectile->position.x, projectile->position.y);

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
				new_impact_effect(world, transform->position);
				destroy_entity(world, view.e);
			}
		}
	}
}

entity new_impact_effect(struct world* world, v2i position) {
	struct animated_sprite f_sprite = get_animated_sprite(animsprid_projectile_impact);
	entity e = new_entity(world);
	add_componentv(world, e, struct transform,
		.z = 100,
		.position = position,
		.dimentions = v2i_mul(make_v2i(sprite_scale, sprite_scale), make_v2i(8, 8)));
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

static struct rect damage_numbers[] = {
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

void damage_fx_system(struct world* world, struct renderer* renderer, double ts) {
	struct texture* atlas = get_texture(texid_icon);

	for (single_view(world, view, struct damage_num_fx)) {
		struct damage_num_fx* d = single_view_get(&view);

		d->velocity += 30.0 * ts;
		d->position.y -= d->velocity * ts;

		float x = 0.0f;

		for (char* c = d->text; *c; c++) {
			char idx = (*c - '0') + 1;
			struct rect rect = damage_numbers[idx];
			if (*c == '-') {
				rect = damage_numbers[0];
			}

			struct textured_quad quad = {
				.texture = atlas,
				.position = make_v2i(x + d->position.x, d->position.y),
				.dimentions = make_v2i(rect.w * sprite_scale, rect.h * sprite_scale),
				.rect = rect,
				.color = { 255, 255, 255, d->timer > 1.0 ? 255 : (i32)(d->timer * 255) }
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

entity new_damage_number(struct world* world, v2i position, i32 number) {
	entity e = new_entity(world);
	add_componentv(world, e, struct damage_num_fx,
		.position = make_v2f(position.x, position.y),
		.velocity = 30.0,
		.timer = 1.2);

	struct damage_num_fx* d = get_component(world, e, struct damage_num_fx);
	sprintf(d->text, "%d", number);

	return e;
}
