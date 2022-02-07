#include <string.h>
#include <stdio.h>

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

static struct lsp_val native(struct lsp_state* ctx, u32 argc, struct lsp_val* argv) {
	printf("From native function: %.*s\n", argv[0].as.obj->as.str.len, argv[0].as.obj->as.str.chars);

	return lsp_make_nil();
}

bool lsp() {
	struct lsp_state* ctx = new_lsp_state(null, null);
	lsp_register_std(ctx);

	lsp_register(ctx, "native_fun", 1, native);

	struct lsp_val v = lsp_do_file(ctx, "util/test/scripts/test.omv");

	printf("Stack size: %d\n", lsp_get_stack_count(ctx));

	free_lsp_state(ctx);
	return true;
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

	struct lsp_val v = lsp_do_string(ctx, "(! true)");

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

	bool good = memcmp(v.as.obj->as.str.chars, "Hello, World!", v.as.obj->as.str.len) == 0;

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

bool lsp_fun() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(set add_nums (fun (x y) ( (ret (+ x y)))))(set a (add_nums 10 20)) a a");

	bool good = v.as.num == 30.0;

	free_lsp_state(ctx);

	return good;
}

static struct lsp_val native_add_nums(struct lsp_state* ctx, u32 argc, struct lsp_val* argv) {
	return lsp_make_num(argv[0].as.num + argv[1].as.num);
}

bool lsp_nat_fun() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	lsp_register(ctx, "add_nums", 2, native_add_nums);

	struct lsp_val v = lsp_do_string(ctx, "(add_nums 10 20)");

	bool good = v.as.num == 30.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_lt() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(< 1 2)");

	bool good = v.as.boolean;

	free_lsp_state(ctx);

	return good;
}

bool lsp_lte() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v1 = lsp_do_string(ctx, "(<= 1 1)");
	struct lsp_val v2 = lsp_do_string(ctx, "(<= 1 2)");

	bool good = v1.as.boolean && v2.as.boolean;

	free_lsp_state(ctx);

	return good;
}

bool lsp_gt() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(> 2 1)");

	bool good = v.as.boolean;

	free_lsp_state(ctx);

	return good;
}

bool lsp_gte() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v1 = lsp_do_string(ctx, "(>= 2 2)");
	struct lsp_val v2 = lsp_do_string(ctx, "(>= 2 1)");

	bool good = v1.as.boolean && v2.as.boolean;

	free_lsp_state(ctx);

	return good;
}

bool lsp_eq() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(= 2 2)");

	bool good = v.as.boolean;

	free_lsp_state(ctx);

	return good;
}

bool lsp_while() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "(set i 0)(while (< i 10)((set i (+ i 1)))) i i");

	bool good = v.as.num == 10.0;

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
		make_test_func(lsp_var),
		make_test_func(lsp_fun),
		make_test_func(lsp_nat_fun),
		make_test_func(lsp_lt),
		make_test_func(lsp_lte),
		make_test_func(lsp_gt),
		make_test_func(lsp_gte),
		make_test_func(lsp_eq),
		make_test_func(lsp_while),
		make_test_func(lsp),
	};

	run_tests(funcs, sizeof(funcs) / sizeof(*funcs));
}
