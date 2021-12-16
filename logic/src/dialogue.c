#include "dialogue.h"
#include "menu.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

struct dialogue_script {
	lua_State* L;

	bool want_next;

	char* ask_name;
};

static void check_lua(lua_State* L, int r) {
	if (r != LUA_OK) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
	}
}

static void call_on_init(lua_State* L) {
	lua_getglobal(L, "on_init");

	if (lua_isnil(L, -1)) { goto end; }

	if (lua_isfunction(L, -1)) {
		check_lua(L, lua_pcall(L, 0, 1, 0));
	} else {
		fprintf(stderr, "`on_init' must be a function.\n");
	}

end:
	lua_pop(L, 1);

	lua_settop(L, 0);
}

static void call_on_play(lua_State* L) {
	lua_getglobal(L, "on_play");

	if (lua_isnil(L, -1)) { goto end; }

	if (lua_isfunction(L, -1)) {
		check_lua(L, lua_pcall(L, 0, 1, 0));
	} else {
		fprintf(stderr, "`on_play' must be a function.\n");
	}

end:
	lua_pop(L, 1);

	lua_settop(L, 0);
}

static void call_on_next(lua_State* L) {
	lua_getglobal(L, "on_next_message");

	if (lua_isnil(L, -1)) { goto end; }

	if (lua_isfunction(L, -1)) {
		check_lua(L, lua_pcall(L, 0, 1, 0));
	} else {
		fprintf(stderr, "`on_next_message' must be a function.\n");
	}

end:
	lua_pop(L, 1);

	lua_settop(L, 0);
}

static void on_message_finish(void* udata) {
	((struct dialogue_script*)udata)->want_next = true;
}

static void* l_alloc(void *ud, void *ptr, u64 osize, u64 nsize) {
	(void)ud;  (void)osize;
	if (nsize == 0) {
		if (ptr) { core_free(ptr); }
		return null;
	} else {
		return core_realloc(ptr, nsize);
	}
}

static i32 l_message(lua_State* L) {
	lua_getglobal(L, "_g_script");
	struct dialogue_script* script = *((struct dialogue_script**)lua_touserdata(L, -1));

	const char* text = luaL_checkstring(L, 1);
	message_prompt_ex(text, on_message_finish, script);

	script->want_next = false;

	return 0;
}

static void on_ask(bool yes, void* udata) {
	struct dialogue_script* script = udata;

	script->want_next = true;

	lua_State* L = script->L;
	lua_getglobal(L, script->ask_name);

	if (lua_isnil(L, -1)) { goto end; }

	if (lua_isfunction(L, -1)) {
		lua_pushboolean(L, yes);
		check_lua(L, lua_pcall(L, 1, 1, 0));
	} else {
		fprintf(stderr, "`%s' must be a function.\n", script->ask_name);
	}

end:
	lua_pop(L, 1);

	lua_settop(L, 0);

	core_free(script->ask_name);
}

static i32 l_ask(lua_State* L) {
	lua_getglobal(L, "_g_script");
	struct dialogue_script* script = *((struct dialogue_script**)lua_touserdata(L, -1));

	script->ask_name = copy_string(luaL_checkstring(L, 2));

	const char* text = luaL_checkstring(L, 1);
	prompt_ask(text, on_ask, script);

	script->want_next = false;

	return 0;
}

struct dialogue_script* new_dialogue_script(const char* source) {
	struct dialogue_script* script = core_calloc(1, sizeof(struct dialogue_script));

	lua_State* L = lua_newstate(l_alloc, null);
	script->L = L;

	luaL_openlibs(L);

	void** s = lua_newuserdata(L, sizeof(void*));
	*s = script;
	lua_setglobal(L, "_g_script");

	lua_pushcfunction(L, l_message);
	lua_setglobal(L, "message");

	lua_pushcfunction(L, l_ask);
	lua_setglobal(L, "ask");

	check_lua(L, luaL_dostring(L, source));

	call_on_init(script->L);

	return script;
}

void free_dialogue_script(struct dialogue_script* script) {
	lua_close(script->L);

	core_free(script);
}

void play_dialogue(struct dialogue_script* script) {
	call_on_play(script->L);

	script->want_next = true;
}

void update_dialogue(struct dialogue_script* script) {
	if (script->want_next) {
		call_on_next(script->L);
		script->want_next = false;
	}
}
