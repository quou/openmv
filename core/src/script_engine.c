#include <stdio.h>

#include "core.h"
#include "script_engine.h"

static void print_value(struct script_value val) {
	printf("Value (");

	switch (val.type) {
		case script_value_null:
			printf("null)");
			break;
		case script_value_number:
			printf("number): %g", val.as.number);
			break;
		default: break;
	}

	printf("\n");
}

static void print_ip(u8* ip) {
	switch (*ip) {
		case op_return:
			puts("RETURN");
			break;
		case op_push: {
			puts("PUSH");
			break;
		}
		case op_add:
			puts("ADD");
			break;
		default: break;
	}
}

void init_chunk(struct script_chunk* chunk) {
	*chunk = (struct script_chunk) { 0 };
}

void deinit_chunk(struct script_chunk* chunk) {
	if (chunk->code) {
		core_free(chunk->code);
	}
	
	*chunk = (struct script_chunk) { 0 };
}

void chunk_add_instruction(struct script_chunk* chunk, u8 instruction) {
	if (chunk->count >= chunk->capacity) {
		chunk->capacity = chunk->capacity < 8 ? 8 : chunk->capacity * 2;
		chunk->code = core_realloc(chunk->code, chunk->capacity * sizeof(*chunk->code));
	}

	chunk->code[chunk->count++] = instruction;
}

void chunk_add_address(struct script_chunk* chunk, u64 address) {
	u64 new_count = chunk->count + sizeof(u64);

	if (new_count >= chunk->capacity) {	
		chunk->capacity = chunk->capacity < 8 ? 8 : chunk->capacity * 2;
		chunk->code = core_realloc(chunk->code, chunk->capacity * sizeof(u8));
	}

	*((u64*)(chunk->code + chunk->count)) = address;
	chunk->count = new_count;
}

struct script_engine* new_script_engine() {
	struct script_engine* engine = core_calloc(1, sizeof(struct script_engine));

	engine->stack_top = engine->stack;
	*engine->stack_top = script_null_value;

	return engine;
}

void free_script_engine(struct script_engine* engine) {	
	if (engine->data) {
		core_free(engine->data);
	}

	if (engine->chunks) {
		for (u64 i = 0; i < engine->chunk_count; i++) {
			deinit_chunk(engine->chunks + i);
		}

		core_free(engine->chunks);
	}

	core_free(engine);
}

u64 new_constant(struct script_engine* engine, struct script_value value) {
	if (engine->data_count >= engine->data_capacity) {
		engine->data_capacity = engine->data_capacity < 8 ? 8 : engine->data_capacity * 2;
		engine->data = core_realloc(engine->data, sizeof(*engine->data) * engine->data_capacity);
	}

	engine->data[engine->data_count] = value;
	return (engine->data_count++ + 1);
}

struct script_value get_value(struct script_engine* engine, u64 address) {
	if (address == 0) {
		return script_null_value;
	}

	return engine->data[address - 1];
}

void script_engine_push(struct script_engine* engine, struct script_value value) {
	engine->stack_top++;

	/* TODO: Runtime error management (Stack overflow, in this case). */

	*engine->stack_top = value;
}

struct script_value script_engine_pop(struct script_engine* engine) {
	return *engine->stack_top--;
}

u64 new_chunk(struct script_engine* engine) {
	if (engine->chunk_count >= engine->chunk_capacity) {
		engine->chunk_capacity = engine->chunk_capacity < 8 ? 8 : engine->chunk_capacity * 2;
		engine->chunks = core_realloc(engine->chunks, sizeof(*engine->chunks) * engine->chunk_capacity);
	}

	struct script_chunk* chunk = engine->chunks + engine->chunk_count;

	init_chunk(chunk);

	return engine->chunk_count++;
}

void execute_chunk(struct script_engine* engine, struct script_chunk* chunk) {
	engine->ip = chunk->code;

	while (1) {
#ifdef DEBUG
		if (engine->debug) {
			print_ip(engine->ip);
		}
#endif

		switch (*engine->ip) {
			case op_return:
				goto finished;
			case op_push: {
				script_engine_push(engine, get_value(engine, *((u64*)(engine->ip + 1))));
				engine->ip += sizeof(u64);
			} break;
			case op_pop: {
				script_engine_pop(engine);
			} break;
			case op_call: {
				u64 chunk_addr = *((u64*)(engine->ip + 1));

#ifdef DEBUG
				if (engine->debug) {
					printf("Call chunk at: %lu\n", chunk_addr);
				}
#endif

				struct script_chunk* to_call = engine->chunks + chunk_addr;
				engine->ip += sizeof(u64);
				u8* old_ip = engine->ip;

				execute_chunk(engine, to_call);

				engine->ip = old_ip;
			}
			case op_add: {
				struct script_value b = script_engine_pop(engine);
				struct script_value a = script_engine_pop(engine);

				script_engine_push(engine, script_number_value(a.as.number + b.as.number));
			} break;
			case op_sub: {
				struct script_value b = script_engine_pop(engine);
				struct script_value a = script_engine_pop(engine);

				script_engine_push(engine, script_number_value(a.as.number - b.as.number));
			} break;
			case op_div: {
				struct script_value b = script_engine_pop(engine);
				struct script_value a = script_engine_pop(engine);

				script_engine_push(engine, script_number_value(a.as.number / b.as.number));
			} break;
			case op_mul: {
				struct script_value b = script_engine_pop(engine);
				struct script_value a = script_engine_pop(engine);

				script_engine_push(engine, script_number_value(a.as.number * b.as.number));
			} break;
		}

		engine->ip++;
	}

finished:
#ifdef DEBUG
	if (engine->debug) {
		printf("Stack top: ");
		print_value(*engine->stack_top);
	}
#endif

	return;
}
