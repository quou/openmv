#include "consts.h"
#include "core.h"
#include "coresys.h"
#include "fx.h"
#include "sprites.h"

entity new_jetpack_particle(struct world* world, v2i position) {
	entity e = new_entity(world);
	add_componentv(world, e, struct transform,
		.position = position,
		.dimentions = make_v2i(8 * sprite_scale, 8 * sprite_scale));
	add_component(world, e, struct sprite, get_sprite(sprid_fx_jetpack));
	get_component(world, e, struct sprite)->origin = make_v2f(0.5f, 0.5f);
	add_componentv(world, e, struct jetpack_fx, .timer = 1.0);
}

void fx_system(struct world* world, double ts) {
	for (view(world, view,
		type_info(struct jetpack_fx),
		type_info(struct transform),
		type_info(struct sprite))) {
		struct jetpack_fx* fx = view_get(&view, struct jetpack_fx);
		struct transform* transform = view_get(&view, struct transform);
		struct sprite* sprite = view_get(&view, struct sprite);

		fx->timer -= ts;

		sprite->color.a = (u8)(fx->timer * 255.0);
		transform->dimentions.x = (8 * sprite_scale * (1.0 - fx->timer)) + 8 * sprite_scale;
		transform->dimentions.y = (8 * sprite_scale * (1.0 - fx->timer)) + 8 * sprite_scale;

		if (fx->timer <= 0.0) {
			destroy_entity(world, view.e);
		}
	}
}
