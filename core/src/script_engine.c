#include <stdio.h>
#include <string.h>

#include "core.h"
#include "script_engine.h"

void print_script_value(struct script_value val) {
	switch (val.type) {
		case script_value_null:
			printf("null");
			break;
		case script_value_number:
			printf("%g", val.as.number);
			break;
		case script_value_function:
			printf("<%ld>", val.as.function);
			break;
		default: break;
	}
}

void print_script_value_type(struct script_value val) {
	switch (val.type) {
		case script_value_null:
			printf("(null)");
			break;
		case script_value_number:
			printf("(number)");
			break;
		case script_value_function:
			printf("(function)");
			break;
		default: break;
	}
}

static void print_ip(struct script_engine* engine, u8* ip) {
	switch (*ip) {
		case op_halt:
			puts("HALT");
			break;
		case op_push:
			printf("PUSH <%lu> (", *(u64*)(ip + 1));
			print_script_value(get_value(engine, *(u64*)(ip + 1)));
			printf(")\n");
			break;
		case op_pop:
			puts("POP");
			break;
		case op_call:
			printf("CALL <%lu>\n", *(u64*)(ip + 1));
			break;
		case op_jump:
			printf("JUMP <%lu>\n", *(u64*)(ip + 1));
			break;
		case op_add:
			puts("ADD");
			break;
		case op_sub:
			puts("SUB");
			break;
		case op_div:
			puts("DIV");
			break;
		case op_mod:
			puts("MOD");
			break;
		case op_mul:
			puts("MUL");
			break;
		case op_print:
			puts("PRINT");
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

void init_script_value_table(struct script_value_table* table) {
	*table = (struct script_value_table) { 0 };
}

void deinit_script_value_table(struct script_value_table* table) {
	if (table->entries) {
		core_free(table->entries);
	}

	*table = (struct script_value_table) { 0 };
}

static struct script_value_table_entry* find_el(struct script_value_table_entry* els, u64 capacity, u64 key) {
	u64 idx = key % capacity;

	for (;;) {
		struct script_value_table_entry* el = els + idx;
		if (!el->taken) {
			return el;
		} else if (el->key == key) {
			return el;
		}

		idx = (idx + 1) % capacity;
	}
}

static void table_resize(struct script_value_table* table, u64 capacity) {
	struct script_value_table_entry* els = core_calloc(capacity, sizeof(struct script_value_table_entry));

	for (u64 i = 0; i < table->capacity; i++) {
		struct script_value_table_entry* el = table->entries + i;
		if (!el->taken) { continue; }

		struct script_value_table_entry* dst = find_el(els, capacity, el->key);
		dst->key = el->key;
		dst->value = el->value;
		dst->taken = true;
	}

	if (table->entries) { core_free(table->entries); }

	table->entries = els;
	table->capacity = capacity;
}

void script_value_table_set(struct script_value_table* table, u64 key, struct script_value value) {
	if (table->count >= table->capacity * 0.75) {
		u64 capacity = table->capacity < 8 ? 8 : table->capacity * 2;
		table_resize(table, capacity);
	}

	struct script_value_table_entry* el = find_el(table->entries, table->capacity, key);
	if (!el->taken) {
		table->count++;
	}

	el->taken = true;
	el->key = key;
	el->value = value;
}

struct script_value script_value_table_get(struct script_value_table* table, u64 key) {
	if (table->count == 0) { return script_null_value; }

	struct script_value_table_entry* el = find_el(table->entries, table->capacity, key);
	if (!el || !el->taken) { return script_null_value; }

	return el->value;
}

struct script_engine* new_script_engine() {
	struct script_engine* engine = core_calloc(1, sizeof(struct script_engine));

	engine->stack_top = engine->stack;
	*engine->stack_top = script_null_value;

	init_script_value_table(&engine->globals);

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

void script_runtime_error(struct script_engine* engine, const char* fmt, ...) {
	fprintf(stderr, "Fatal Runtime Error: ");

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fprintf(stderr, "\nExecution Halted.\n");

	engine->panic = true;
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

struct script_value script_get_global(struct script_engine* engine, const char* name) {
	return script_value_table_get(&engine->globals, elf_hash((const u8*)name, (u32)strlen(name)));
}

void script_engine_push(struct script_engine* engine, struct script_value value) {
	engine->stack_top++;

	if (engine->stack_top >= engine->stack + script_engine_stack_size) {
		script_runtime_error(engine, "Stack Overflow.");
		return;
	}

	*engine->stack_top = value;
}

struct script_value script_engine_pop(struct script_engine* engine) {
	return *engine->stack_top--;
}

struct script_value script_engine_peek(struct script_engine* engine, u64 offset) {
	return *engine->stack_top;
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

static bool is_zero(struct script_value val) {
	switch (val.type) {
		case script_value_null:
			return true;
		case script_value_number:
			if (val.as.number == 0.0) {
				return true;
			}
			return false;
		default: return false;
	}
}

void execute_chunk(struct script_engine* engine, struct script_chunk* chunk) {
	engine->ip = chunk->code;

	while (1) {
#ifdef DEBUG
		if (engine->debug) {
			print_ip(engine, engine->ip);
		}
#endif

		switch (*engine->ip) {
			case op_halt:
				goto finished;
			case op_push: {
				script_engine_push(engine, get_value(engine, *((u64*)(engine->ip + 1))));
				engine->ip += sizeof(u64);
			} break;
			case op_pop: {
				script_engine_pop(engine);
			} break;
			case op_call: {
				u64 function_hash = *((u64*)(engine->ip + 1));

				struct script_value val = script_value_table_get(&engine->globals, function_hash);

				if (val.type != script_value_function) {
					script_runtime_error(engine, "Value not callable.");
					break;
				}

				u64 chunk_addr = val.as.function;

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
			} break;
			case op_jump: {
				u64 offset = *((u64*)(engine->ip + 1));
				engine->ip += sizeof(u64);

				if (is_zero(script_engine_peek(engine, 0))) {
					engine->ip += offset;
				}
			} break;
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
			case op_mod: {
				struct script_value b = script_engine_pop(engine);
				struct script_value a = script_engine_pop(engine);

				script_engine_push(engine, script_number_value((i64)a.as.number % (i64)b.as.number));
			} break;
			case op_print: {
				struct script_value v = script_engine_pop(engine);
				print_script_value(v);
				printf("\n");
			} break;
		}

		engine->ip++;

		if (engine->panic) {
			return;
		}
	}

finished:
#ifdef DEBUG
	if (engine->debug) {
		printf("Stack top: ");
		print_script_value(*engine->stack_top);
		printf("(");
		print_script_value_type(*engine->stack_top);
		printf(")\n");
	}
#endif

	return;
}
