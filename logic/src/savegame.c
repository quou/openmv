#include <stdio.h>
#include <string.h>

#include "common.h"
#include "platform.h"
#include "core.h"
#include "coresys.h"
#include "logic_store.h"
#include "player.h"
#include "savegame.h"
#include "table.h"

void savegame_init() {
	logic_store->savegame_persist = new_table(sizeof(struct persistent));
}

void savegame_deinit() {
	struct table* savegame_persist = logic_store->savegame_persist;

	for (struct table_iter i = new_table_iter(savegame_persist); table_iter_next(&i);) {
		struct persistent* p = i.value;
		if (p->type == persist_str) {
			core_free(p->as.str);
		}
	}

	free_table(savegame_persist);
}

void set_persistent(const char* name, u32 type, const void* val) {
	struct table* savegame_persist = logic_store->savegame_persist;

	struct persistent p = {
		.type = type
	};

	switch (type) {
		case persist_i32:
			p.as.i = *(i32*)val;
			break;
		case persist_u32:
			p.as.u = *(u32*)val;
			break;
		case persist_bool:
			p.as.b = *(bool*)val;
			break;
		case persist_f32:
			p.as.f = *(f64*)val;
			break;
		case persist_str:
			p.as.str = copy_string(val);
			break;
		default: break;
	}

	table_set(savegame_persist, name, &p);
}

struct persistent* get_persistent(const char* name) {
	struct table* savegame_persist = logic_store->savegame_persist;

	return (struct persistent*)table_get(savegame_persist, name);
}

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
	fwrite(&player->level, sizeof(player->level), 1, file);

	fwrite(&player->items, sizeof(player->items), 1, file);
	fwrite(&player->hp_ups, sizeof(player->hp_ups), 1, file);
}

static void read_player(FILE* file) {
	struct world* world = logic_store->world;

	struct player* player = get_component(world, logic_store->player, struct player);
	struct transform* transform = get_component(world, logic_store->player, struct transform);

	player->invul_timer = 0.0;
	player->invul = false;
	player->visible = true;

	fread(&transform->position.x, sizeof(transform->position.x), 1, file);
	fread(&transform->position.y, sizeof(transform->position.y), 1, file);
	fread(&player->velocity.x, sizeof(player->velocity.x), 1, file);
	fread(&player->velocity.y, sizeof(player->velocity.y), 1, file);
	fread(&player->hp, sizeof(player->hp), 1, file);
	fread(&player->max_hp, sizeof(player->max_hp), 1, file);
	fread(&player->money, sizeof(player->money), 1, file);
	fread(&player->level, sizeof(player->level), 1, file);

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
	struct table* savegame_persist = logic_store->savegame_persist;
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

	u32 table_count = get_table_count(savegame_persist);
	fwrite(&table_count, sizeof(table_count), 1, file);

	for (struct table_iter i = new_table_iter(savegame_persist); table_iter_next(&i);) {
		u32 key_len = (u32)strlen(i.key);
		fwrite(&key_len, sizeof(key_len), 1, file);
		fwrite(i.key, 1, key_len, file);
		
		struct persistent* p = i.value;
		fwrite(&p->type, sizeof(p->type), 1, file);

		switch (p->type) {
			case persist_i32:
				fwrite(&p->as.i, sizeof(p->as.i), 1, file);
				break;
			case persist_u32:
				fwrite(&p->as.u, sizeof(p->as.u), 1, file);
				break;
			case persist_bool:
				fwrite(&p->as.b, sizeof(p->as.b), 1, file);
				break;
			case persist_f32:
				fwrite(&p->as.f, sizeof(p->as.f), 1, file);
				break;
			case persist_str: {
				u32 len = (u32)strlen(p->as.str);
				fwrite(&len, sizeof(len), 1, file);
				fwrite(&p->as.str, 1, len, file);
			} break;
			default: break;
		}
	}

	fclose(file);
}

void loadgame() {
	struct table* savegame_persist = logic_store->savegame_persist;
	struct world* world = logic_store->world;

	FILE* file = fopen("savegame", "rb");
	if (!file) {
		fprintf(stderr, "No savegame!\n");
		message_prompt("No savegame!");
		return;
	}

	read_player(file);
	
	savegame_deinit();
	savegame_init();

	savegame_persist = logic_store->savegame_persist;

	/* Read the current room path and load it. */
	char* room_path = read_string(file);
	if (logic_store->room) {
		free_room(logic_store->room);
	}
	logic_store->room = load_room(world, room_path);
	core_free(room_path);

	u32 persist_count = 0;
	fread(&persist_count, sizeof(persist_count), 1, file);

	for (u32 i = 0; i < persist_count; i++) {
		u32 key_len = 0;
		fread(&key_len, sizeof(key_len), 1, file);
		char* key = core_alloc(key_len + 1);
		key[key_len] = '\0';
		fread(key, 1, key_len, file);
		
		struct persistent p = { 0 };
		fread(&p.type, sizeof(p.type), 1, file);

		switch (p.type) {
			case persist_i32:
				fread(&p.as.i, sizeof(p.as.i), 1, file);
				break;
			case persist_u32:
				fread(&p.as.u, sizeof(p.as.u), 1, file);
				break;
			case persist_bool:
				fread(&p.as.b, sizeof(p.as.b), 1, file);
				break;
			case persist_f32:
				fread(&p.as.f, sizeof(p.as.f), 1, file);
				break;
			case persist_str: {
				u32 len = 0;
				fread(&len, sizeof(len), 1, file);
				p.as.str = core_alloc(len);
				p.as.str[len] = '\0';
				fread(p.as.str, len, 1, file);
			} break;
			default: break;
		}

		table_set(savegame_persist, key, &p);

		core_free(key);
	}

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

bool savegame_exists() {
	return file_exists("savegame");
}
