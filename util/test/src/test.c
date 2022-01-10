#include <stdio.h>

#include "test.h"

void run_tests(struct test_func* funcs, u32 func_count) {
	for (u32 i = 0; i < func_count; i++) {
		printf("Running test: %s", funcs[i].name);
		if (funcs[i].func()) {
			printf("\t\t[ \033[1;32mOK\033[0m ]\n");
		} else {
			printf("\t\t[\033[1;31mFAIL\033[0m]\n");
		}
	}
}
