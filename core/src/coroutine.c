#include "coroutine.h"
#include "core.h"

struct coroutine new_coroutine(coroutine_func func, void* udata) {
	return (struct coroutine) {
		.func = func,
		.udata = udata,
		.state = 0,
	};
}
