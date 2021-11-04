#include "player.h"
#include "res.h"
#include "coresys.h"

entity new_player_entity(struct world* world) {
	struct texture* tex = load_texture("res/bmp/map.bmp");

	entity e = new_entity(world);
	add_componentv(world, e, struct transform, .dimentions = { 100, 100 });
	add_componentv(world, e, struct player, 0);
	add_componentv(world, e, struct sprite,
		.texture = tex,
		.rect = { 0, 0, 100, 100 },
		.origin = { 0.0f, 0.0f },
		.color = { 255, 255, 255, 255 });
}

void player_system(struct world* world, double ts) {

}
