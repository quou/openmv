#include <stdio.h>

#include "coresys.h"
#include "keymap.h"
#include "platform.h"
#include "player.h"
#include "res.h"
#include "sprites.h"

struct player_constants {
	float move_speed;
	float jump_force;
};

const struct player_constants player_constants;

entity new_player_entity(struct world* world) {
	struct texture* tex = load_texture("res/bmp/char.bmp");

	entity e = new_entity(world);
	add_componentv(world, e, struct transform, .dimentions = { 64, 64 });
	add_componentv(world, e, struct player, 0);
	add_component(world, e, struct animated_sprite, get_animated_sprite(animsprid_player_run));
	
	return e;
}

void player_system(struct world* world, double ts) {
	if (key_just_pressed(main_window, mapped_key("jump"))) {
		printf("Jump just pressed.\n");
	}
}
