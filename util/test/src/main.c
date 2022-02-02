#include <string.h>

#include "common.h"
#include "core.h"
#include "coroutine.h"
#include "lsp.h"
#include "test.h"

static coroutine_decl(test_coroutine)
	*(i32*)co_udata = 10;
	coroutine_yield();

	*(i32*)co_udata = 20;
	coroutine_yield();

	*(i32*)co_udata = 30;
	coroutine_yield();

	*(i32*)co_udata = 40;
coroutine_end

bool coroutine() {
	i32 val;

	struct coroutine co = new_coroutine(test_coroutine, &val);

	coroutine_resume(co);
	if (val != 10) { return false; }

	coroutine_resume(co);
	if (val != 20) { return false; }

	coroutine_resume(co);
	if (val != 30) { return false; }

	coroutine_resume(co);
	if (val != 40) { return false; }

	return true;
}

bool lsp() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_file(ctx, "util/test/scripts/test.omv");

	free_lsp_state(ctx);
	return v.as.num == 1.0;
}

bool lsp_add() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(+ 10 20)");

	bool good = v.as.num == 30.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_sub() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(- 20 10)");

	bool good = v.as.num == 10.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_mul() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(* 20 10)");

	bool good = v.as.num == 200.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_div() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(/ 20 10)");

	bool good = v.as.num == 2.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_not() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(not true)");

	bool good = v.as.boolean == false;

	free_lsp_state(ctx);

	return good;
}

bool lsp_neg() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(neg 10)");

	bool good = v.as.num == -10.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_cat() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(cat \"Hello, \" \"World!\")");

	bool good = strcmp(v.as.obj->as.str.chars, "Hello, World!") == 0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_if_true() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(if true 1 0)");

	bool good = v.as.num == 1.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_if_false() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(if false 0 1)");

	bool good = v.as.num == 1.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_var() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(set a 14)\na");

	bool good = v.as.num == 14.0;

	free_lsp_state(ctx);

	return good;
}

i32 main() {
	struct test_func funcs[] = {
		make_test_func(coroutine),
		make_test_func(lsp_add),
		make_test_func(lsp_sub),
		make_test_func(lsp_mul),
		make_test_func(lsp_div),
		make_test_func(lsp_not),
		make_test_func(lsp_neg),
		make_test_func(lsp_cat),
		make_test_func(lsp_if_true),
		make_test_func(lsp_if_false),
		make_test_func(lsp_var),
		make_test_func(lsp),
	};

	run_tests(funcs, sizeof(funcs) / sizeof(*funcs));
}
