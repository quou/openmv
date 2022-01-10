#include "coroutine.h"
#include "core.h"

struct coroutine new_coroutine(coroutine_func func, void* udata) {
	return (struct coroutine) {
		.func = func,
		.udata = udata,
		.progress = 0,
		.is_dead = false,
	};
}
