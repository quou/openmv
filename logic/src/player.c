#include <math.h>
#include <stdio.h>
#include <string.h>

#include "consts.h"
#include "coresys.h"
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

	double projectile_lifetime;
	double projectile_speed;

	v2f left_muzzle_pos;
	v2f right_muzzle_pos;

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
	
	.projectile_lifetime = 1.0,
	.projectile_speed = 1000.0,

	.left_muzzle_pos =  { 12 * sprite_scale, 11 * sprite_scale },
	.right_muzzle_pos = { 3  * sprite_scale, 11 * sprite_scale },

	.max_air_dash = 3,

	.right_collider = { 4 * sprite_scale, 1 * sprite_scale, 9 * sprite_scale, 15 * sprite_scale },
	.left_collider =  { 3 * sprite_scale, 1 * sprite_scale, 9 * sprite_scale, 15 * sprite_scale }
};

entity new_player_entity(struct world* world) {
	struct texture* tex = load_texture("res/bmp/char.bmp");

	entity e = new_entity(world);
	add_componentv(world, e, struct transform, .dimentions = { 64, 64 });
	add_componentv(world, e, struct player, .position = { 128, 128 }, .collider = player_constants.left_collider);
	add_component(world, e, struct animated_sprite, get_animated_sprite(animsprid_player_run_right));
	
	return e;
}

void player_system(struct world* world, struct renderer* renderer, struct room** room, double ts) {
	struct entity_buffer* to_destroy = new_entity_buffer();

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
				entity_buffer_push(to_destroy, up_view.e);
			}
		}

		if (key_just_pressed(main_window, mapped_key("fire"))) {
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

		transform->position = make_v2i((i32)player->position.x, (i32)player->position.y);

		float distance_to_player = sqrtf(powf(logic_store->camera_position.x - player->position.x, 2)
			+ powf(logic_store->camera_position.y - player->position.y, 2));

		v2f camera_dir = v2f_normalised(v2f_sub(player->position, logic_store->camera_position));

		logic_store->camera_position.x += camera_dir.x * distance_to_player * ts * 10.0f;
		logic_store->camera_position.y += camera_dir.y * distance_to_player * ts * 10.0f;

		renderer->camera_pos = make_v2i((i32)logic_store->camera_position.x, (i32)logic_store->camera_position.y);
	}

	entity_buffer_clear(to_destroy, world);
}

void projectile_system(struct world* world, struct room* room, double ts) {
	struct entity_buffer* to_delete = new_entity_buffer();

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
			entity_buffer_push(to_delete, view.e);
		} else {
			struct rect rect = {
				projectile->collider.x + transform->position.x,
				projectile->collider.y + transform->position.y,
				projectile->collider.w, projectile->collider.h
			};

			if (rect_room_overlap(room, rect, null)) {
				new_impact_effect(world, transform->position);
				entity_buffer_push(to_delete, view.e);
			}
		}
	}

	entity_buffer_clear(to_delete, world);
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
	struct entity_buffer* to_delete = new_entity_buffer();

	for (view(world, view, type_info(struct transform), type_info(struct anim_fx), type_info(struct animated_sprite))) {
		struct transform* transform = view_get(&view, struct transform);
		struct animated_sprite* anim = view_get(&view, struct animated_sprite);

		if (anim->current_frame >= anim->frame_count - 1) {
			entity_buffer_push(to_delete, view.e);
		}
	}

	entity_buffer_clear(to_delete, world);
}
