#include <stdio.h>

#include "core.h"
#include "res.h"
#include "script_engine.h"

void compile_file(struct script_engine* engine, const char* path) {
	u8* src;
	read_raw(path, &src, null, true);

	compile_script(engine, (const char*)src);

	core_free(src);
}

int main() {
	struct script_engine* engine = new_script_engine();

	engine->debug = true;

	compile_file(engine, "util/script_tester/res/test.osc");

	struct script_chunk* main = engine->chunks + script_get_global(engine, "main").as.function;

	execute_chunk(engine, main);

	free_script_engine(engine);
}
