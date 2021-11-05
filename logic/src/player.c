#include "player.h"
#include "res.h"
#include "coresys.h"

entity new_player_entity(struct world* world) {
	struct texture* tex = load_texture("res/bmp/char.bmp");

	entity e = new_entity(world);
	add_componentv(world, e, struct transform, .dimentions = { 100, 100 });
	add_componentv(world, e, struct player, 0);
	add_componentv(world, e, struct animated_sprite,
		.texture = tex,
		.frames = { { 0, 0, 16, 16 }, { 16, 0, 16, 16 }, { 32, 0, 16, 16 } },
		.speed = 8.9,
		.frame_count = 3,
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 });
}

void player_system(struct world* world, double ts) {

}
