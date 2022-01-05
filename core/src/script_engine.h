#pragma once

/* Simple bytecode interpreter.
 *
 * This VM is stack-based.
 *
 * Instruction set:
 * - halt			Stop the execution of the current chunk.
 * - push	<addr>	Push a value at <addr> onto the stack.
 * - pop			Pops the value off the top of the stack.
 * - call	<hash>	Call a global value with the hash <hash>.
 * - jump	<offst>	Peeks and jumps to IP + <offst> if the value is zero.
 * - add			Pops two values off the stack, adds them and pushes the result.
 * - sub			Pops two values off the stack, subtracts the last from the first and pushes the result.
 * - div			Pops two values off the stack, divides the last by the first and pushes the result.
 * - mul			Pops two values off the stack, multiplies them and pushes the result.
 * - mod			Pops two values off the stack, truncates them, divides the last by the first and pushes the remainder.
 */

#include "common.h"

#define script_engine_stack_size 1024

enum {
	op_halt = 0,
	op_push,
	op_pop,
	op_call,
	op_jump,
	op_add,
	op_sub,
	op_div,
	op_mul,
	op_mod
};

enum {
	script_value_null = 0,
	script_value_number,
	script_value_function
};

struct script_value {
	u32 type;

	union {
		double number;
		u64 function;
	} as;
};

#define script_null_value 			(struct script_value) { .type = script_value_null }
#define script_number_value(n_)		(struct script_value) { .type = script_value_number,	.as.number = n_ }
#define script_function_value(a_)	(struct script_value) { .type = script_value_function,	.as.function = a_ }

void print_script_value(struct script_value val);

struct script_chunk {
	u8* code;
	u64 count;
	u64 capacity;
};

void init_chunk(struct script_chunk* chunk);
void deinit_chunk(struct script_chunk* chunk);
void chunk_add_instruction(struct script_chunk* chunk, u8 instruction);
void chunk_add_address(struct script_chunk* chunk, u64 address);

struct script_value_table_entry {
	u64 key;
	struct script_value value;
	bool taken;
};

/* Faster hash table implementation than OpenMV's default hash table. */
struct script_value_table {
	struct script_value_table_entry* entries;
	u64 count;
	u64 capacity;
};

void init_script_value_table(struct script_value_table* table);
void deinit_script_value_table(struct script_value_table* table);
void script_value_table_set(struct script_value_table* table, u64 key, struct script_value value);
struct script_value script_value_table_get(struct script_value_table* table, u64 key);

struct script_engine {
	bool debug;

	u8* ip;

	bool panic;

	/* Constants. */
	struct script_value* data;
	u64 data_count;
	u64 data_capacity;

	struct script_chunk* chunks;
	u64 chunk_count;
	u64 chunk_capacity;

	struct script_value stack[script_engine_stack_size];
	struct script_value* stack_top;

	struct script_value_table globals;
};

struct script_engine* new_script_engine();
void free_script_engine(struct script_engine* engine);

void script_runtime_error(struct script_engine* engine, const char* fmt, ...);

u64 new_chunk(struct script_engine* engine);
void execute_chunk(struct script_engine* engine, struct script_chunk* chunk);

void script_engine_push(struct script_engine* engine, struct script_value value);
struct script_value script_engine_pop(struct script_engine* engine);
struct script_value script_engine_peek(struct script_engine* engine, u64 offset);

struct script_value script_get_global(struct script_engine* engine, const char* name);

/* Returns the address of the new constant. */
u64 new_constant(struct script_engine* engine, struct script_value value);
struct script_value get_value(struct script_engine* engine, u64 address);

void compile_script(struct script_engine* engine, const char* source);
