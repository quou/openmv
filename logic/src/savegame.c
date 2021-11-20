#include <stdio.h>

#include "common.h"
#include "core.h"
#include "coresys.h"
#include "logic_store.h"
#include "player.h"

static void write_player(FILE* file) {
	struct world* world = logic_store->world;

	struct player* player = get_component(world, logic_store->player, struct player);
	struct transform* transform = get_component(world, logic_store->player, struct transform);

	fwrite(&transform->position.x, sizeof(transform->position.x), 1, file);
	fwrite(&transform->position.y, sizeof(transform->position.y), 1, file);

	fwrite(&player->items, sizeof(player->items), 1, file);
}

static void read_player(FILE* file) {
	struct world* world = logic_store->world;

	struct player* player = get_component(world, logic_store->player, struct player);
	struct transform* transform = get_component(world, logic_store->player, struct transform);

	fread(&transform->position.x, sizeof(transform->position.x), 1, file);
	fread(&transform->position.y, sizeof(transform->position.y), 1, file);

	player->position = make_v2f(transform->position.x, transform->position.y);

	fread(&player->items, sizeof(player->items), 1, file);
}

void savegame() {
	struct world* world = logic_store->world;

	FILE* file = fopen("savegame", "wb");

	write_player(file);

	fclose(file);
}

void loadgame() {
	struct world* world = logic_store->world;

	FILE* file = fopen("savegame", "rb");

	read_player(file);

	fclose(file);
}
