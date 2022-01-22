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

bool coroutine_test() {
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
#include <stdio.h>

bool lsp_add_test() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx,
		"(print (+ (* 4 10) (+ 40 2)))\n"
		"82");

	free_lsp_state(ctx);
	return v.as.num == 82.0f;
}

i32 main() {
	struct test_func funcs[] = {
		make_test_func(coroutine_test),
		make_test_func(lsp_add_test),
	};

	run_tests(funcs, sizeof(funcs) / sizeof(*funcs));
}
