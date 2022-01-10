#pragma once

#include "common.h"

#include <setjmp.h>

struct coroutine;

typedef void (*coroutine_func)(struct coroutine*, void*);

#define _coroutine_jmp() \
	if (co->progress != 0) { \
		longjmp(co->env, 0); \
	} \

#define coroutine_decl(n_) \
	void n_(struct coroutine* co, void* co_udata) { \
	_coroutine_jmp()

#define coroutine_yield() \
	do { \
		co->progress++; \
		co->r = setjmp(co->env); \
		if (co->r == 0) { return; } \
	} while (0)

#define coroutine_resume(co_) \
		(co_).func(&(co_), (co_).udata)

struct coroutine {
	coroutine_func func;

	void* udata;

	i32 r;

	u32 progress;
	bool is_dead;

	jmp_buf env;
};

struct coroutine new_coroutine(coroutine_func func, void* udata);
