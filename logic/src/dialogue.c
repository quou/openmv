#include "dialogue.h"
#include "menu.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

struct dialogue_script {
	lua_State* L;

	bool want_next;
};

static void check_lua(lua_State* L, int r) {
	if (r != LUA_OK) {
		luaL_error(L, "%s\n", lua_tostring(L, -1));
	}
}

static void call_on_init(lua_State* L) {
	lua_getglobal(L, "on_init");

	if (lua_isnil(L, -1)) { goto end; }

	if (lua_isfunction(L, -1)) {
		check_lua(L, lua_pcall(L, 0, 1, 0));
	} else {
		luaL_error(L, "`on_init' must be a function.\n");
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
		luaL_error(L, "`on_next_message' must be a function.\n");
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

	check_lua(L, luaL_dostring(L, source));

	return script;
}

void free_dialogue_script(struct dialogue_script* script) {
	lua_close(script->L);

	core_free(script);
}

void play_dialogue(struct dialogue_script* script) {
	call_on_init(script->L);

	script->want_next = true;
}

void update_dialogue(struct dialogue_script* script) {
	if (script->want_next) {
		call_on_next(script->L);
		script->want_next = false;
	}
}
