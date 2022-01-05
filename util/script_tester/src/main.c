#include <stdio.h>

#include "core.h"
#include "res.h"
#include "script_engine.h"

u64 decl_fun(struct script_engine* engine) {
	u64 fun_ptr = new_chunk(engine);
	struct script_chunk* chunk = engine->chunks + fun_ptr;

	u64 a_ptr = new_constant(engine, script_number_value(3));
	u64 b_ptr = new_constant(engine, script_number_value(3));

	chunk_add_instruction(chunk, op_push); /* Push the 3 */
	chunk_add_address(chunk, a_ptr);

	chunk_add_instruction(chunk, op_push); /* Push the 6 */
	chunk_add_address(chunk, b_ptr);

	chunk_add_instruction(chunk, op_sub);

	chunk_add_instruction(chunk, op_halt);

	return fun_ptr;
}

void do_file(struct script_engine* engine, const char* path) {
	u8* src;
	read_raw(path, &src, null, true);

	compile_script(engine, (const char*)src);

	core_free(src);
}

int main() {
	struct script_engine* engine = new_script_engine();

	engine->debug = true;

	do_file(engine, "util/script_tester/res/test.osc");

	u64 adder_func = decl_fun(engine);

	u64 main_ptr = new_chunk(engine);
	struct script_chunk* main = engine->chunks + main_ptr;

	u64 a_ptr = new_constant(engine, script_number_value(3));

	chunk_add_instruction(main, op_call); /* Call a function */
	chunk_add_address(main, adder_func);

	chunk_add_instruction(main, op_jump);
	chunk_add_address(main, 2 + sizeof(u64)); /* Jump over the push and add */

	chunk_add_instruction(main, op_push); /* Push the 3 */
	chunk_add_address(main, a_ptr);

	chunk_add_instruction(main, op_add);

	chunk_add_instruction(main, op_halt);

	execute_chunk(engine, main);

	free_script_engine(engine);
}
