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
	fwrite(&player->velocity.x, sizeof(player->velocity.x), 1, file);
	fwrite(&player->velocity.y, sizeof(player->velocity.y), 1, file);
	fwrite(&player->hp, sizeof(player->hp), 1, file);
	fwrite(&player->max_hp, sizeof(player->max_hp), 1, file);
	fwrite(&player->money, sizeof(player->money), 1, file);

	fwrite(&player->items, sizeof(player->items), 1, file);
	fwrite(&player->hp_ups, sizeof(player->hp_ups), 1, file);
}

static void read_player(FILE* file) {
	struct world* world = logic_store->world;

	struct player* player = get_component(world, logic_store->player, struct player);
	struct transform* transform = get_component(world, logic_store->player, struct transform);

	fread(&transform->position.x, sizeof(transform->position.x), 1, file);
	fread(&transform->position.y, sizeof(transform->position.y), 1, file);
	fread(&player->velocity.x, sizeof(player->velocity.x), 1, file);
	fread(&player->velocity.y, sizeof(player->velocity.y), 1, file);
	fread(&player->hp, sizeof(player->hp), 1, file);
	fread(&player->max_hp, sizeof(player->max_hp), 1, file);
	fread(&player->money, sizeof(player->money), 1, file);

	fread(&player->items, sizeof(player->items), 1, file);
	fread(&player->hp_ups, sizeof(player->hp_ups), 1, file);
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
	if (!file) {
		fprintf(stderr, "Failed to open savegame!\n");
		message_prompt("Failed to open savegame!");
		return;
	}

	write_player(file);

	/* Write the current room path*/
	write_string(file, get_room_path(logic_store->room));

	fclose(file);
}

void loadgame() {
	struct world* world = logic_store->world;

	FILE* file = fopen("savegame", "rb");
	if (!file) {
		fprintf(stderr, "No savegame!\n");
		message_prompt("No savegame!");
		return;
	}

	read_player(file);

	/* Read the current room path and load it. */
	char* room_path = read_string(file);
	if (logic_store->room) {
		free_room(logic_store->room);
	}
	logic_store->room = load_room(world, room_path);
	core_free(room_path);

	fclose(file);
}

static void on_save_ask(bool yes, void* udata) {
	if (yes) {
		savegame();
		message_prompt("Game saved.");
	}
}

void ask_savegame() {
	prompt_ask("Do you want to save?", on_save_ask, null);
}
