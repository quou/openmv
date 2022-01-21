#include <stdio.h>

#include "test.h"

void run_tests(struct test_func* funcs, u32 func_count) {
	for (u32 i = 0; i < func_count; i++) {
		printf("[....]\tRunning test: %s", funcs[i].name);
		printf("\033 7");
		
		bool r = funcs[i].func();

		printf("\r");

		fflush(stdout);

		if (r) {
			printf("[ \033[1;32mOK\033[0m ]");
		} else {
			printf("[\033[1;31mFAIL\033[0m]");
		}

		printf("\033 8\n");
	}
}
