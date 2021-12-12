#include <math.h>

#include "consts.h"
#include "enemy.h"
#include "physics.h"
#include "player.h"
#include "sprites.h"

static void on_bat_create(struct world* world, entity e, void* component) {
	struct bat* bat = component;

	bat->old_position = get_component(world, e, struct transform)->position;
	bat->offset = get_component(world, e, struct transform)->position.y;
}

entity new_bat(struct world* world, struct room* room, v2f position) {
	entity e = new_entity(world);

	struct animated_sprite sprite = get_animated_sprite(animsprid_bat);

	set_component_create_func(world, struct bat, on_bat_create);

	add_componentv(world, e, struct transform,
		.position = position,
		.dimentions = { sprite.frames[0].w * sprite_scale, sprite.frames[0].h * sprite_scale });
	add_component(world, e, struct animated_sprite, sprite);
	add_componentv(world, e, struct room_child, .parent = room);
	add_componentv(world, e, struct bat);
	add_componentv(world, e, struct enemy, .collider = {
		0, 0,
		sprite.frames[0].w * sprite_scale,
		sprite.frames[0].h * sprite_scale },
		.hp = 1, .damage = 1);

	return e;
}

void enemy_system(struct world* world, double ts) {
	for (view(world, view, type_info(struct transform), type_info(struct bat))) {
		struct transform* transform = view_get(&view, struct transform);
		struct bat* bat = view_get(&view, struct bat);

		bat->offset += ts;

		transform->position.y = bat->old_position.y + (float)sin(bat->offset) * 100.0f;
	}

	for (view(world, view, type_info(struct transform), type_info(struct enemy))) {
		struct transform* transform = view_get(&view, struct transform);
		struct enemy* enemy = view_get(&view, struct enemy);

		struct rect e_rect = {
			transform->position.x + enemy->collider.x,
			transform->position.y + enemy->collider.y,
			enemy->collider.w, enemy->collider.h
		};

		for (view(world, p_view, type_info(struct transform), type_info(struct projectile))) {
			struct transform* p_transform = view_get(&p_view, struct transform);
			struct projectile* projectile = view_get(&p_view, struct projectile);

			struct rect p_rect = {
				p_transform->position.x + projectile->collider.x,
				p_transform->position.y + projectile->collider.y,
				projectile->collider.w, projectile->collider.h
			};

			if (rect_overlap(e_rect, p_rect, null)) {
				new_impact_effect(world, p_transform->position);
				i32 dmg = projectile->damage;
				if (dmg > enemy->hp) { dmg = enemy->hp; }
				new_damage_number(world, transform->position, -dmg);
				destroy_entity(world, p_view.e);

				enemy->hp -= projectile->damage;

				if (enemy->hp <= 0) {
					destroy_entity(world, view.e);
				}
			}
		}
	}
}
