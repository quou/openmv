#include <stdio.h>

#include "core.h"
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

int main() {
	struct script_value_table table;
	init_script_value_table(&table);

	script_value_table_set(&table, 100, script_number_value(30));
	script_value_table_set(&table, 140, script_number_value(50));
	script_value_table_set(&table, 110, script_number_value(80));

	print_script_value(script_value_table_get(&table, 100)); printf("\n");
	print_script_value(script_value_table_get(&table, 140)); printf("\n");
	print_script_value(script_value_table_get(&table, 110)); printf("\n");

	deinit_script_value_table(&table);

	struct script_engine* engine = new_script_engine();

	engine->debug = true;

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
