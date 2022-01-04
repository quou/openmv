#include "core.h"
#include "script_engine.h"

int main() {
	struct script_engine* engine = new_script_engine();

	engine->debug = true;

	u64 a_ptr = new_constant(engine, script_number_value(3));
	u64 b_ptr = new_constant(engine, script_number_value(6));

	chunk_add_instruction(&engine->main, op_push); /* Push the 3 */
	chunk_add_address(&engine->main, a_ptr);

	chunk_add_instruction(&engine->main, op_push); /* Push the 6 */
	chunk_add_address(&engine->main, b_ptr);

	chunk_add_instruction(&engine->main, op_mul);

	chunk_add_instruction(&engine->main, op_return);

	execute_chunk(engine, &engine->main);

	free_script_engine(engine);
}
