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

#define coroutine_yield() \
	do { co->state = __LINE__; return; case __LINE__:; } while (0) 

#define coroutine_resume(co_) \
		(co_).func(&(co_), (co_).udata)

struct coroutine {
	coroutine_func func;

	void* udata;

	i32 state;
};

struct coroutine new_coroutine(coroutine_func func, void* udata);
