#pragma once

#include "common.h"

struct coroutine;

typedef void (*coroutine_func)(struct coroutine*, void*);

#define coroutine_decl(n_) \
	void n_(struct coroutine* co, void* co_udata) { \
		switch (co->state) { \
			case 0:

#define coroutine_end \
	} }

/* This causes an error that reads `case expression not constant' on
 * the Microsoft compilers, because they do something stupid and
 * non-standard with their __LINE__ define.
 *
 * To fix it: under `Debug info:' in the project settings, select anything =
 * other than `Program Database for Edit and Continue'. */
#define coroutine_yield() \
	do { co->state = __LINE__; return; case __LINE__:; } while (0) 

#define coroutine_resume(co_) \
		(co_).func(&(co_), (co_).udata)

struct coroutine {
	coroutine_func func;

	void* udata;

	i32 state;
};

API struct coroutine new_coroutine(coroutine_func func, void* udata);
