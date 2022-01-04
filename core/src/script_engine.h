#pragma once

/* Simple bytecode interpreter. */

#include "common.h"

#define script_engine_stack_size 1024

enum {
	op_return = 0,
	op_push,
	op_add,
	op_sub,
	op_div,
	op_mul
};

enum {
	script_value_null = 0,
	script_value_number
};

struct script_value {
	u32 type;

	union {
		double number;
	} as;
};

#define script_null_value 		(struct script_value) { .type = script_value_null }
#define script_number_value(n_)	(struct script_value) { .type = script_value_number, .as.number = n_ }

struct script_chunk {
	u8* code;
	u64 count;
	u64 capacity;
};

void init_chunk(struct script_chunk* chunk);
void deinit_chunk(struct script_chunk* chunk);
void chunk_add_instruction(struct script_chunk* chunk, u8 instruction);
void chunk_add_address(struct script_chunk* chunk, u64 address);

struct script_engine {
	struct script_chunk main;

	bool debug;

	u8* ip;

	/* Constants. */
	struct script_value* data;
	u64 data_count;
	u64 data_capacity;

	struct script_value stack[script_engine_stack_size];
	struct script_value* stack_top;
};

struct script_engine* new_script_engine();
void free_script_engine(struct script_engine* engine);
void execute_chunk(struct script_engine* engine, struct script_chunk* chunk);

void script_engine_push(struct script_engine* engine, struct script_value value);
struct script_value script_engine_pop(struct script_engine* engine);

/* Returns the address of the new constant. */
u64 new_constant(struct script_engine* engine, struct script_value value);
struct script_value get_value(struct script_engine* engine, u64 address);
