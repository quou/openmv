#include "common.h"
#include "coroutine.h"
#include "room.h"
#include "savegame.h"

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

struct coroutine broken_robot_co;

API void CDECL broken_robot_on_next() {
	coroutine_resume(broken_robot_co);
}

API void CDECL broken_robot_on_play(void* ctx) {
	broken_robot_co = new_coroutine(broken_robot_play, ctx);
}

API void CDECL miner_robot_on_play(void* ctx) {
	dialogue_message("......", ctx);
}
