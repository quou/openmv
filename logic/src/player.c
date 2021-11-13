#include <stdio.h>
#include <string.h>

#include "coresys.h"
#include "keymap.h"
#include "platform.h"
#include "player.h"
#include "res.h"
#include "sprites.h"

struct player_constants {
	float move_speed;
	float jump_force;
	float gravity;
};

const struct player_constants player_constants = {
	.move_speed = 300,
	.jump_force = -550,
	.gravity = 1000
};

entity new_player_entity(struct world* world) {
	struct texture* tex = load_texture("res/bmp/char.bmp");

	entity e = new_entity(world);
	add_componentv(world, e, struct transform, .dimentions = { 64, 64 });
	add_componentv(world, e, struct player, .position = { 128, 128 }, .collider = { 4*4, 1*4, 9*4, 15*4 });
	add_component(world, e, struct animated_sprite, get_animated_sprite(animsprid_player_run_right));
	
	return e;
}

void player_system(struct world* world, struct room** room, double ts) {
	for (view(world, view,
			type_info(struct transform),
			type_info(struct player),
			type_info(struct animated_sprite))) {
		struct transform* transform = view_get(&view, struct transform);
		struct player* player = view_get(&view, struct player);
		struct animated_sprite* sprite = view_get(&view, struct animated_sprite);

		player->velocity.y += player_constants.gravity * ts;

		if (key_just_pressed(main_window, mapped_key("jump"))) {
			player->velocity.y = player_constants.jump_force;
		}

		if (key_pressed(main_window, mapped_key("right"))) {
			player->velocity.x = player_constants.move_speed;

			struct animated_sprite new_sprite = get_animated_sprite(animsprid_player_run_right);
			memcpy(sprite->frames, new_sprite.frames,
				new_sprite.frame_count * sizeof(*new_sprite.frames));
		} else if (key_pressed(main_window, mapped_key("left"))) {
			player->velocity.x = -player_constants.move_speed;

			struct animated_sprite new_sprite = get_animated_sprite(animsprid_player_run_left);
			memcpy(sprite->frames, new_sprite.frames,
				new_sprite.frame_count * sizeof(*new_sprite.frames));
		} else {
			player->velocity.x = 0.0f;
		}

		player->position = v2f_add(player->position, v2f_mul(player->velocity, make_v2f(ts, ts)));

		handle_body_collisions(room, player->collider, &player->position, &player->velocity);

		transform->position = make_v2i((i32)player->position.x, (i32)player->position.y);
	}
}
