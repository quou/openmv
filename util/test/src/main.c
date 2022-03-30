#include <string.h>
#include <stdio.h>

#include "common.h"
#include "core.h"
#include "coroutine.h"
#include "lsp.h"
#include "maths.h"
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

static struct lsp_val print_test_ptr(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
	printf("%d\n", *(int*)args[0].as.obj->as.ptr.ptr);

	return lsp_make_nil();
}

static void test_ptr_create(struct lsp_state* ctx, void** ptr) {
	*ptr = core_alloc(sizeof(int));
	**(int**)ptr = 10;
}

static void test_ptr_destroy(struct lsp_state* ctx, void** ptr) {
	core_free(*ptr);
}

bool lsp() {
	struct lsp_state* ctx = new_lsp_state(null, null);
	lsp_register_std(ctx);

	lsp_register_ptr(ctx, "TestPointer", test_ptr_create, test_ptr_destroy);

	lsp_register(ctx, "native_fun", 1, native);
	lsp_register(ctx, "print_test_ptr", 1, print_test_ptr);

	struct lsp_val v = lsp_do_file_no_pck(ctx, "util/test/scripts/test.lsp");

	printf("Stack size: %d\n", lsp_get_stack_count(ctx));

	free_lsp_state(ctx);
	return true;
}

bool lsp_add() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(+ 10 20)");

	bool good = v.as.num == 30.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_sub() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(- 20 10)");

	bool good = v.as.num == 10.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_mul() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(* 20 10)");

	bool good = v.as.num == 200.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_div() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(/ 20 10)");

	bool good = v.as.num == 2.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_not() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(! true)");

	bool good = v.as.boolean == false;

	free_lsp_state(ctx);

	return good;
}

bool lsp_neg() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(neg 10)");

	bool good = v.as.num == -10.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_cat() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(cat \"Hello, \" \"World!\")");

	bool good = memcmp(v.as.obj->as.str.chars, "Hello, World!", v.as.obj->as.str.len) == 0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_var() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(set a 14)\na");

	bool good = v.as.num == 14.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_fun() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(set add_nums (fun (x y) ( (ret (+ x y)))))(set a (add_nums 10 20)) a a");

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

	struct lsp_val v = lsp_do_string(ctx, "test", "(add_nums 10 20)");

	bool good = v.as.num == 30.0;

	free_lsp_state(ctx);

	return good;
}

bool lsp_lt() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(< 1 2)");

	bool good = v.as.boolean;

	free_lsp_state(ctx);

	return good;
}

bool lsp_lte() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v1 = lsp_do_string(ctx, "test", "(<= 1 1)");
	struct lsp_val v2 = lsp_do_string(ctx, "test", "(<= 1 2)");

	bool good = v1.as.boolean && v2.as.boolean;

	free_lsp_state(ctx);

	return good;
}

bool lsp_gt() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(> 2 1)");

	bool good = v.as.boolean;

	free_lsp_state(ctx);

	return good;
}

bool lsp_gte() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v1 = lsp_do_string(ctx, "test", "(>= 2 2)");
	struct lsp_val v2 = lsp_do_string(ctx, "test", "(>= 2 1)");

	bool good = v1.as.boolean && v2.as.boolean;

	free_lsp_state(ctx);

	return good;
}

bool lsp_eq() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(= 2 2)");

	bool good = v.as.boolean;

	free_lsp_state(ctx);

	return good;
}

bool lsp_while() {
	struct lsp_state* ctx = new_lsp_state(null, null);

	struct lsp_val v = lsp_do_string(ctx, "test", "(set i 0)(while (< i 10)((set i (+ i 1)))) i i");

	bool good = v.as.num == 10.0;

	free_lsp_state(ctx);

	return good;
}

bool m_make_v2f() {
	v2f a = make_v2f(12.0f, 10.0f);
	return a.x == 12.0f && a.y == 10.0f;
}

bool m_v2f_zero() {
	v2f a = v2f_zero();
	return a.x == 0.0f && a.y == 0.0f;
}

bool m_v2f_add() {
	v2f a = make_v2f(2.0f, 4.0f);
	v2f b = make_v2f(10.0f, 12.0f);
	v2f c = v2f_add(a, b);
	return c.x == 12.0f && c.y == 16.0f;
}

bool m_v2f_sub() {
	v2f a = make_v2f(10.0f, 12.0f);
	v2f b = make_v2f(3.0f, 4.0f);
	v2f c = v2f_sub(a, b);
	return c.x == 7.0f && c.y == 8.0f;
}

bool m_v2f_mul() {
	v2f a = make_v2f(10.0f, 12.0f);
	v2f b = make_v2f(3.0f, 4.0f);
	v2f c = v2f_mul(a, b);
	return c.x == 30.0f && c.y == 48.0f;
}

bool m_v2f_div() {
	v2f a = make_v2f(10.0f, 8.0f);
	v2f b = make_v2f(2.0f, 4.0f);
	v2f c = v2f_div(a, b);
	return c.x == 5.0f && c.y == 2.0f;
}

bool m_v2f_mag_sqrd() {
	f32 m = v2f_mag_sqrd(make_v2f(4.0f, 7.0f));
	return m == 65.0f;
}

bool m_v2f_mag() {
	f32 m = v2f_mag(make_v2f(4.0f, 7.0f));
	return m == sqrtf(65.0f);
}

bool m_v2f_normalised() {
	v2f a = v2f_normalised(make_v2f(0.0f, 10.0f));
	return a.x == 0.0f && a.y == 1.0f;
}

bool m_v2f_eq() {
	v2f a = v2f_normalised(make_v2f(10.0f, 10.0f));
	v2f b = v2f_normalised(make_v2f(10.0f, 10.0f));
	v2f c = v2f_normalised(make_v2f(10.0f, 10.0f));
	v2f d = v2f_normalised(make_v2f(13.0f, 11.0f));

	return v2f_eq(a, b) && !v2f_eq(c, d);
}

#include "platform.h"

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
		make_test_func(m_make_v2f),
		make_test_func(m_v2f_zero),
		make_test_func(m_v2f_add),
		make_test_func(m_v2f_sub),
		make_test_func(m_v2f_mul),
		make_test_func(m_v2f_div),
		make_test_func(m_v2f_mag_sqrd),
		make_test_func(m_v2f_mag),
		make_test_func(m_v2f_normalised),
		make_test_func(m_v2f_eq),
	};

	run_tests(funcs, sizeof(funcs) / sizeof(*funcs));
}
