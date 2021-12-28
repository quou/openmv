#include <math.h>

#include "consts.h"
#include "enemy.h"
#include "logic_store.h"
#include "physics.h"
#include "player.h"
#include "sprites.h"

static void on_bat_create(struct world* world, entity e, void* component) {
	struct bat* bat = component;

	bat->old_position = get_component(world, e, struct transform)->position;
	bat->offset = get_component(world, e, struct transform)->position.y;
}

static void on_path_follow_create(struct world* world, entity e, void* component) {
	((struct path_follow*)component)->first_frame = true;
}

static void on_path_follow_destroy(struct world* world, entity e, void* component) {
	struct path_follow* f = component;

	if (f->path_name) {
		core_free(f->path_name);
	}
}

entity new_bat(struct world* world, struct room* room, v2f position, char* path_name) {
	entity e = new_entity(world);

	struct animated_sprite sprite = get_animated_sprite(animsprid_bat);

	set_component_create_func(world, struct bat, on_bat_create);
	set_component_create_func(world, struct path_follow, on_path_follow_create);
	set_component_destroy_func(world, struct path_follow, on_path_follow_destroy);

	add_componentv(world, e, struct transform,
		.position = position,
		.dimentions = { sprite.frames[0].w * sprite_scale, sprite.frames[0].h * sprite_scale });
	add_component(world, e, struct animated_sprite, sprite);
	add_componentv(world, e, struct room_child, .parent = room);
	add_componentv(world, e, struct bat, .path_name = path_name);
	add_componentv(world, e, struct enemy, .collider = {
		0, 0,
		sprite.frames[0].w * sprite_scale,
		sprite.frames[0].h * sprite_scale },
		.hp = 1, .damage = 1, .money_drop = 1);

	if (path_name) {
		add_componentv(world, e, struct path_follow, .path_name = path_name, .room = room, .speed = 100.0f);
	}

	return e;
}

entity new_spider(struct world* world, struct room* room, v2f position) {
	entity e = new_entity(world);

	struct sprite sprite = get_sprite(sprid_spider);

	add_componentv(world, e, struct transform,
		.position = { position.x - sprite.rect.w * sprite_scale, position.y - sprite.rect.h * sprite_scale },
		.dimentions = { sprite.rect.w * sprite_scale, sprite.rect.h * sprite_scale });
	add_component(world, e, struct sprite, sprite);
	add_componentv(world, e, struct room_child, .parent = room);
	add_componentv(world, e, struct enemy, .collider = {
		0, 0,
		sprite.rect.w * sprite_scale,
		sprite.rect.h * sprite_scale },
		.hp = 1, .damage = 1, .money_drop = 1);
	add_componentv(world, e, struct spider, .room = room);

	return e;
}

void enemy_system(struct world* world, struct room* room, double ts) {
	/* Bat system */
	for (view(world, view, type_info(struct transform), type_info(struct bat))) {
		struct transform* transform = view_get(&view, struct transform);
		struct bat* bat = view_get(&view, struct bat);

		if (!has_component(world, view.e, struct path_follow)) {
			bat->offset += ts;

			transform->position.y = bat->old_position.y + (float)sin(bat->offset) * 100.0f;
		}
	}

	for (view(world, view, type_info(struct transform), type_info(struct path_follow))) {
		struct transform* transform = view_get(&view, struct transform);
		struct path_follow* follow = view_get(&view, struct path_follow);

		struct path* path = get_path(follow->room, follow->path_name);

		if (follow->first_frame) {
			transform->position = path->points[0];
			follow->first_frame = false;
		}

		v2f target = path->points[follow->node];

		v2f dir = v2f_normalised(v2f_sub(target, transform->position));
		transform->position = v2f_add(transform->position, v2f_mul(dir, make_v2f(follow->speed * ts, follow->speed * ts)));

		float dist_sqrd = powf(target.x - transform->position.x, 2.0f) + powf(target.y - transform->position.y, 2.0f);
		if (dist_sqrd < 10.0f) {
			follow->node += follow->reverse ? -1 : 1;

			if (!follow->reverse && follow->node >= path->count) {
				follow->reverse = true;
				follow->node = path->count - 1;
			} else if (follow->reverse && follow->node < 0) {
				follow->reverse = false;
				follow->node = 0;
			}
		}
	}

	/* Spider system */
	for (view(world, view, type_info(struct transform), type_info(struct spider), type_info(struct enemy))) {
		struct transform* transform = view_get(&view, struct transform);
		struct spider* spider = view_get(&view, struct spider);
		struct enemy* enemy = view_get(&view, struct enemy);

		struct transform* p_transform = get_component(world, logic_store->player, struct transform);

		if (!spider->triggered) {
			float dist_sqrd = powf(p_transform->position.x - transform->position.x, 2.0f) + powf(p_transform->position.y - transform->position.y, 2.0f);

			if (dist_sqrd < 100000) {
				spider->triggered = true;
			}
		}
		else {
			spider->velocity.y += g_gravity * ts; 

			transform->position = v2f_add(transform->position, v2f_mul(spider->velocity, make_v2f(ts, ts)));

			struct rect e_rect = {
				transform->position.x + enemy->collider.x,
				transform->position.y + enemy->collider.y,
				enemy->collider.w, enemy->collider.h
			};

			v2i normal;
			if (rect_room_overlap(spider->room, e_rect, &normal) && normal.y == 1) {
				spider->velocity.y = -800;

				if (p_transform->position.x < transform->position.x) {
					spider->velocity.x = -100;
				} else {
					spider->velocity.x = 100;
				}
			} else {
				if (handle_body_collisions(spider->room, enemy->collider, &transform->position, &spider->velocity)) {
					spider->velocity.x = 0.0;
				}
			}
		}
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
				new_impact_effect(world, p_transform->position, animsprid_blood);
				i32 dmg = projectile->damage;
				if (dmg > enemy->hp) { dmg = enemy->hp; }
				new_damage_number(world, transform->position, -dmg);
				destroy_entity(world, p_view.e);

				enemy->hp -= projectile->damage;

				struct player* player = get_component(world, logic_store->player, struct player);

				enemy->invul = true;
				enemy->invul_timer = 0.1;

				if (enemy->hp <= 0) {
					/* Chance to get a heart is much higher if the player has low hp. */
					double chance = 5;
					if (player->hp < player->max_hp) {
						chance = 30;
					}

					if (random_chance(chance)) {
						struct rect heart_rect = get_sprite(sprid_upgrade_health_pack).rect;

						new_heart(world, room, v2f_sub(transform->position,
							make_v2f((heart_rect.w * sprite_scale) / 2, (heart_rect.h * sprite_scale) / 2)), 1);
					} else {
						struct rect coin_rect = get_sprite(sprid_coin).rect;

						for (i32 i = 0; i < enemy->money_drop; i++) {
							new_coin_pickup(world, room, v2f_sub(transform->position,
								make_v2f((coin_rect.w * sprite_scale) / 2, (coin_rect.h * sprite_scale) / 2)));
						}
					}

					new_impact_effect(world, transform->position, animsprid_poof);
					destroy_entity(world, view.e);
				}
			}
		}
	}

	for (view(world, view, type_info(struct enemy), type_info(struct animated_sprite))) {
		struct enemy* enemy = view_get(&view, struct enemy);
		struct animated_sprite* sprite = view_get(&view, struct animated_sprite);

		if (enemy->invul) {
			enemy->invul_timer -= ts;
			if (enemy->invul_timer <= 0.0) {
				enemy->invul = false;
			}
		}

		sprite->inverted = enemy->invul;
	}
}
