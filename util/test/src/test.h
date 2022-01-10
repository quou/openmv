#pragma once

#include "common.h"

typedef bool(*test_func)();

struct test_func {
	test_func func;
	const char* name;
};

#define make_test_func(f_) \
	(struct test_func) { .func = f_, .name = #f_ }

void run_tests(struct test_func* funcs, u32 func_count);
