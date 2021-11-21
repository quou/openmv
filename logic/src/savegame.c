#include <stdio.h>
#include <string.h>

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

static void write_string(FILE* file, const char* str) {
	u32 len = (u32)strlen(str);
	fwrite(&len, sizeof(len), 1, file);
	fwrite(str, 1, len, file);
}

static char* read_string(FILE* file) {
	u32 len = 0;
	fread(&len, sizeof(len), 1, file);
	char* buf = core_alloc(len + 1);
	buf[len] = 0;
	fread(buf, 1, len, file);
	return buf;
}

void savegame() {
	struct world* world = logic_store->world;

	FILE* file = fopen("savegame", "wb");

	write_player(file);

	/* Write the current room path*/
	write_string(file, get_room_path(logic_store->room));

	fclose(file);
}

void loadgame() {
	struct world* world = logic_store->world;

	FILE* file = fopen("savegame", "rb");

	read_player(file);

	/* Read the current room path and load it. */
	char* room_path = read_string(file);
	free_room(logic_store->room);
	logic_store->room = load_room(world, room_path);
	core_free(room_path);

	fclose(file);
}
