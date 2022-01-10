#include "common.h"
#include "core.h"
#include "test.h"
#include "coroutine.h"

static coroutine_decl(test_coroutine)
	*(i32*)co_udata = 10;
	coroutine_yield();

	*(i32*)co_udata = 20;
	coroutine_yield();

	*(i32*)co_udata = 30;
	coroutine_yield();

	*(i32*)co_udata = 40;
}

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

i32 main() {
	struct test_func funcs[] = {
		make_test_func(coroutine_test),
	};

	run_tests(funcs, sizeof(funcs) / sizeof(*funcs));
}
