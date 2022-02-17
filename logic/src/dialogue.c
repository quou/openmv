#include <string.h>
#include "common.h"
#include "coroutine.h"
#include "logic_store.h"
#include "player.h"
#include "room.h"
#include "savegame.h"
#include "table.h"

struct coroutine broken_robot_co;
struct coroutine gunsmith_co;

static coroutine_decl(broken_robot_play)
	if (!get_persistent("npc_broken_robot") || !get_persistent("npc_broken_robot")->as.b) {
		dialogue_message("Hello!", co->udata);
		coroutine_yield();

		dialogue_message("I see you were lucky enough to fall out of the incinerator, as well.", co->udata);
		coroutine_yield();

		dialogue_message("This is the incinerator room.", co->udata);
		coroutine_yield();

		dialogue_message("Here is where they dispose of machines that don't meet the standard for quality...", co->udata);
		coroutine_yield();

		dialogue_message("See? My right eye is broken.", co->udata);
		coroutine_yield();

		dialogue_message("There doesn't seem to be anything wrong with you, though...", co->udata);
		coroutine_yield();

		dialogue_message("......", co->udata);
		coroutine_yield();

		dialogue_message("...Maybe a computer bug?", co->udata);
		coroutine_yield();

		dialogue_message("We were built as scavengers to scout out resources.", co->udata);
		coroutine_yield();

		dialogue_message("......", co->udata);
		coroutine_yield();

		dialogue_message("What's that? You want to get out of here?", co->udata);
		coroutine_yield();

		dialogue_message("That's impossible. You will be destroyed...", co->udata);
		coroutine_yield();

		dialogue_message("Or else roam this ancient complex until your computer dies and your batteries fail.", co->udata);
		coroutine_yield();

		dialogue_message("......", co->udata);
		coroutine_yield();

		bool v = true;
		set_persistent("npc_broken_robot", persist_bool, &v);
		return;
	}

	dialogue_message("......", co->udata);
	coroutine_yield();
coroutine_end

int upgrade_price = 0;

static void on_gun_ask(bool yes, void* ctx) {
	struct player* player = get_player();

	if (yes) {
		if (player->money >= upgrade_price) {
			dialogue_message("Done.", ctx);
			player->money -= upgrade_price;
			player->level++;
		} else {
			dialogue_message("You don't have enough money. Get out of my sight.", ctx);
		}
	} else {
		dialogue_message("Get out of my sight.", ctx);
	}
}

static coroutine_decl(gunsmith_play)
	if (!get_persistent("gunsmith_intro") || !get_persistent("gunsmith_intro")->as.b) {
		bool v = true;
		set_persistent("gunsmith_intro", persist_bool, &v);

		dialogue_message("Hello.", co->udata);
		coroutine_yield();

		dialogue_message("Welcome.", co->udata);
		coroutine_yield();

		dialogue_message("......", co->udata);
		coroutine_yield();
	}

	struct player* player = get_player();
	if (player->level == 1) {
		upgrade_price = 40;
		dialogue_ask("Would you like me to upgrade your gun for 40 %c?", on_gun_ask, co->udata);
		coroutine_yield();
	} else if (player->level == 2) {
		upgrade_price = 250;
		dialogue_ask("Would you like me to upgrade your again gun for 250 %c?", on_gun_ask, co->udata);
		coroutine_yield();
	} else {	
		dialogue_message("I have nothing left to offer you.", co->udata);
		coroutine_yield();
	}
coroutine_end

void broken_robot_on_next() {
	coroutine_resume(broken_robot_co);
}

void broken_robot_on_play(void* ctx) {
	broken_robot_co = new_coroutine(broken_robot_play, ctx);
}

void gunsmith_on_next() {
	coroutine_resume(gunsmith_co);
}

void gunsmith_on_play(void* ctx) {
	gunsmith_co = new_coroutine(gunsmith_play, ctx);
}

void miner_robot_on_play(void* ctx) {
	dialogue_message("......", ctx);
}

struct dialogue_func {
	char* name;
	void* ptr;
};

#define set_dialogue(f_) \
	do { \
		u32 idx = logic_store->dialogue_func_count++; \
		((struct dialogue_func*)logic_store->dialogue_funcs)[idx].name = copy_string(#f_); \
		((struct dialogue_func*)logic_store->dialogue_funcs)[idx].ptr = (f_); \
	} while (0)

void init_dialogue() {
	logic_store->dialogue_funcs = core_alloc(64 * sizeof(struct dialogue_func));

	set_dialogue(broken_robot_on_next);
	set_dialogue(broken_robot_on_play);
	set_dialogue(gunsmith_on_next);
	set_dialogue(gunsmith_on_play);
	set_dialogue(miner_robot_on_play);
}

void deinit_dialogue() {
	for (u32 i = 0; i < logic_store->dialogue_func_count; i++) {
		core_free(((struct dialogue_func*)logic_store->dialogue_funcs)[i].name);
	}

	core_free(logic_store->dialogue_funcs);
}

void* get_dialogue_fun(const char* name) {
	struct dialogue_func* fs = logic_store->dialogue_funcs;

	for (u32 i = 0; i < logic_store->dialogue_func_count; i++) {
		if (strcmp(name, fs[i].name) == 0) {
			return fs[i].ptr;
		}
	}

	fprintf(stderr, "No such dialogue function: `%s'\n", name);
	return null;
}
