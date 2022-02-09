#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "core.h"
#include "lsp.h"
#include "res.h"

#define chunk_size 1024
#define chunk_max_constants UINT8_MAX
#define stack_size 1024
#define max_objs 1024
#define max_natives UINT8_MAX

enum {
	op_halt = 0,
	op_push,
	op_push_nil,
	op_push_true,
	op_push_false,
	op_pop,
	op_pop_n,
	op_add,
	op_sub,
	op_mul,
	op_div,
	op_lt,
	op_lte,
	op_gt,
	op_gte,
	op_cat,
	op_print,
	op_put,
	op_set,
	op_get,
	op_set_arg,
	op_get_arg,
	op_jump_if_false,
	op_jump,
	op_back_jump,
	op_not,
	op_and,
	op_or,
	op_neg,
	op_eq,
	op_call,
	op_call_nat
};

struct lsp_chunk {
	u8 code[chunk_size];
	u32 lines[chunk_size];

	u32 count;

	struct lsp_val consts[chunk_max_constants];
	u32 const_count;
};

struct lsp_nat {
	char* name;
	u32 argc;
	lsp_nat_fun_t fun;
};

struct lsp_state {
	struct lsp_val stack[stack_size];
	struct lsp_val* stack_top;

	struct lsp_val* frame_start;

	struct lsp_nat natives[max_natives];
	u32 nat_count;

	bool simple_errors;

	u8* ip;

	struct lsp_chunk* chunk;

	bool exception;

	FILE* error;
	FILE* info;

	struct lsp_obj objs[max_objs];
	u32 obj_count;
};

static void lsp_chunk_add_op(struct lsp_state* ctx, struct lsp_chunk* chunk, u8 op, u32 line) {
	if (chunk->count >= chunk_size) {
		fprintf(ctx->error, "Too many instructions in one chunk. Maximum %d.\n", chunk_size);
		chunk->code[chunk_size - 1] = op_halt;
		return;
	}

	chunk->code[chunk->count] = op;
	chunk->lines[chunk->count] = line;
	chunk->count++;
}

static u8 lsp_chunk_add_const(struct lsp_state* ctx, struct lsp_chunk* chunk, struct lsp_val val) {
	if (chunk->const_count >= chunk_max_constants) {
		fprintf(ctx->error, "Too many constants in one chunk. Maximum %d.\n", chunk_max_constants);
		return 0;
	}

	chunk->consts[chunk->const_count] = val;

	return chunk->const_count++;
}

static struct lsp_obj* lsp_new_obj(struct lsp_state* ctx, u8 type) {
	for (u32 i = 0; i < ctx->obj_count; i++) {
		if (ctx->objs[i].recyclable) {
			ctx->objs[i].recyclable = false;
			ctx->objs[i].type = type;
			ctx->objs[i].ref = 1;
			return ctx->objs + i;
		}
	}

	if (ctx->obj_count >= max_objs) {
		fprintf(ctx->error, "Too many objects. Maximum: %d.\n", max_objs);
		return null;
	}

	ctx->objs[ctx->obj_count].type = type;
	ctx->objs[ctx->obj_count].ref = 1;

	return ctx->objs + ctx->obj_count++;
}

static void lsp_free_obj(struct lsp_state* ctx, struct lsp_obj* obj) {	
	switch (obj->type) {
		case lsp_obj_str:
			core_free(obj->as.str.chars);
			break;
		case lsp_obj_fun:
			core_free(obj->as.fun.chunk);
			break;
		default: break;
	}

	memset(obj, 0, sizeof(struct lsp_obj));
	obj->recyclable = true;
}

struct lsp_val lsp_make_str(struct lsp_state* ctx, const char* start, u32 len) {
	struct lsp_obj* obj = lsp_new_obj(ctx, lsp_obj_str);

	struct lsp_val v = {
		.type = lsp_val_obj,
		.as.obj = obj
	};

	obj->as.str.chars = core_alloc(len);
	obj->as.str.len = len;
	memcpy(obj->as.str.chars, start, len);

	return v;
}

struct lsp_val lsp_make_fun(struct lsp_state* ctx, struct lsp_chunk* chunk, u32 argc) {
	struct lsp_obj* obj = lsp_new_obj(ctx, lsp_obj_fun);

	struct lsp_val v = {
		.type = lsp_val_obj,
		.as.obj = obj
	};

	obj->as.fun.chunk = chunk;
	obj->as.fun.argc = argc;

	return v;
}

struct lsp_state* new_lsp_state(void* error, void* info) {
	if (!error) { error = stderr; }
	if (!info)  { info = stdout; }

	struct lsp_state* state = core_calloc(1, sizeof(struct lsp_state));

	state->error = error;
	state->info = info;
	state->stack_top = state->stack;

	return state;
}

void free_lsp_state(struct lsp_state* state) {
	for (u32 i = 0; i < state->obj_count; i++) {
		if (!state->objs[i].recyclable) {
			lsp_free_obj(state, state->objs + i);
		}
	}

	for (u32 i = 0; i < state->nat_count; i++) {
		core_free(state->natives[i].name);
	}

	core_free(state);
}

void lsp_set_simple_errors(struct lsp_state* state, bool simple) {
	state->simple_errors = simple;
}

static void lsp_exception(struct lsp_state* ctx, const char* message, ...) {
	u32 instruction = (u32)(ctx->ip - ctx->chunk->code - 1);

	fprintf(ctx->error, "Exception [line %d]: ", ctx->chunk->lines[instruction]);
	
	va_list l;
	va_start(l, message);
	vfprintf(ctx->error, message, l);
	va_end(l);

	fprintf(ctx->error, "\n");

	ctx->exception = true;
}

u32 lsp_get_stack_count(struct lsp_state* ctx) {
	return (u32)(ctx->stack_top - ctx->stack) / sizeof(struct lsp_val);
}

void lsp_push(struct lsp_state* ctx, struct lsp_val val) {
	if (ctx->stack_top >= ctx->stack + stack_size) {
		lsp_exception(ctx, "Stack overflow.");
		return;
	}

	*(++ctx->stack_top) = val;
}

struct lsp_val lsp_pop(struct lsp_state* ctx) {
	if (ctx->stack_top - 1 < ctx->stack) {	
		lsp_exception(ctx, "Stack underflow.");
		return lsp_make_nil();
	}

	return *(ctx->stack_top--);
}

struct lsp_val lsp_peek(struct lsp_state* ctx) {
	return *ctx->stack_top;
}

bool objs_eq(struct lsp_state* ctx, struct lsp_obj* a, struct lsp_obj* b) {
	if (a->type != b->type) { return false; }

	switch (a->type) {
		case lsp_obj_str:
			if (a->as.str.len != b->as.str.len) { return false; }
			return memcmp(a->as.str.chars, b->as.str.chars, b->as.str.len) == 0;
		default: return a == b;
	}
}

bool lsp_vals_eq(struct lsp_state* ctx, struct lsp_val a, struct lsp_val b) {
	if (a.type != b.type) { return false; }

	switch (a.type) {
		case lsp_val_nil:
			return true;
		case lsp_val_num:
			return a.as.num == b.as.num;
		case lsp_val_bool:
			return a.as.boolean == b.as.boolean;
		case lsp_val_obj:
			return objs_eq(ctx, a.as.obj, b.as.obj);
		default: return false;
	}
}

#define arith_op(op_) do { \
		struct lsp_val b = lsp_pop(ctx); \
		struct lsp_val a = lsp_pop(ctx); \
		if (a.type != lsp_val_num || b.type != lsp_val_num) { \
			lsp_exception(ctx, "Operands to `%s' must be numbers.", #op_); \
			return lsp_make_nil(); \
		} \
		lsp_push(ctx, lsp_make_num(a.as.num op_ b.as.num)); \
	} while (0)

#define bool_op(op_) do { \
		struct lsp_val b = lsp_pop(ctx); \
		struct lsp_val a = lsp_pop(ctx); \
		if (a.type != lsp_val_num || b.type != lsp_val_num) { \
			lsp_exception(ctx, "Operands to `%s' must be numbers.", #op_); \
			return lsp_make_nil(); \
		} \
		lsp_push(ctx, lsp_make_bool(a.as.num op_ b.as.num)); \
	} while (0)

static void print_obj(FILE* out, struct lsp_obj* obj) {
	switch (obj->type) {
		case lsp_obj_str:
			fprintf(out, "%.*s", obj->as.str.len, obj->as.str.chars);
			break;
		case lsp_obj_fun:
			fprintf(out, "<function %p>", obj->as.fun.chunk);
			break;
		default:
			fprintf(out, "<object %p>", obj);
			break;
	}
}

static void print_val(FILE* out, struct lsp_val val) {
	switch (val.type) {
		case lsp_val_nil:
			fprintf(out, "nil");
			break;
		case lsp_val_num:
			fprintf(out, "%g", val.as.num);
			break;
		case lsp_val_bool:
			fprintf(out, val.as.boolean ? "true" : "false");
			break;
		case lsp_val_obj:
			print_obj(out, val.as.obj);
			break;
		default:
			fprintf(out, "<unkown value type>");
			break;
	}
}

static bool is_falsey(struct lsp_val val) {
	switch (val.type) {
		case lsp_val_nil: return true;
		case lsp_val_bool: return !val.as.boolean;
		case lsp_val_num: return val.as.num == 0.0 ? true : false;
		case lsp_val_obj: return false;
		default: break;
	}

	return true;
}

static struct lsp_val lsp_eval(struct lsp_state* ctx, struct lsp_chunk* chunk) {
	ctx->chunk = chunk;
	ctx->ip = chunk->code;

	while (1) {
		if (ctx->exception) {
			return lsp_make_nil();
		}

		switch (*ctx->ip) {
			case op_halt:
				/*for (u32 i = 0; i < ctx->obj_count; i++) { Collect garbage
					struct lsp_obj* obj = ctx->objs + i;

					if (obj->ref == 0) {
						lsp_free_obj(ctx, obj);
					}
				}*/
				goto eval_halt;
			case op_push:
				ctx->ip++;
				lsp_push(ctx, chunk->consts[*ctx->ip]);
				break;
			case op_push_nil:   lsp_push(ctx, lsp_make_nil()); break;
			case op_push_true:  lsp_push(ctx, lsp_make_bool(true)); break;
			case op_push_false: lsp_push(ctx, lsp_make_bool(false)); break;
			case op_pop_n: {
				ctx->ip++;
				for (u8 i = 0; i < *ctx->ip; i++) {
					lsp_pop(ctx);
				}
			} break;
			case op_pop:
				lsp_pop(ctx);
				break;
			case op_add: arith_op(+); break;
			case op_sub: arith_op(-); break;
			case op_div: arith_op(/); break;
			case op_mul: arith_op(*); break;
			case op_lt:  bool_op(<);  break;
			case op_lte: bool_op(<=); break;
			case op_gt:  bool_op(>);  break;
			case op_gte: bool_op(>=); break;
			case op_eq:
				lsp_push(ctx, lsp_make_bool(lsp_vals_eq(ctx, lsp_pop(ctx), lsp_pop(ctx))));
				break;
			case op_cat: {
				struct lsp_val b = lsp_pop(ctx);
				struct lsp_val a = lsp_pop(ctx);
				if (a.type != lsp_val_obj || b.type != lsp_val_obj ||
					a.as.obj->type != lsp_obj_str || b.as.obj->type != lsp_obj_str) {
					lsp_exception(ctx, "Operands to `cat' must be strings.");
					return lsp_make_nil();
				}

				u32 new_len = a.as.obj->as.str.len + b.as.obj->as.str.len;
				char* new = core_alloc(new_len);
				memcpy(new,                        a.as.obj->as.str.chars, a.as.obj->as.str.len);
				memcpy(new + a.as.obj->as.str.len, b.as.obj->as.str.chars, b.as.obj->as.str.len);

				struct lsp_obj* obj = lsp_new_obj(ctx, lsp_obj_str);
				obj->as.str.chars = new;
				obj->as.str.len = new_len;

				lsp_push(ctx, (struct lsp_val) { .type = lsp_val_obj, .as.obj = obj });
			} break;
			case op_print:
				print_val(ctx->info, lsp_pop(ctx));
				fprintf(ctx->info, "\n");
				break;
			case op_put:
				print_val(ctx->info, lsp_pop(ctx));
				break;
			case op_set:
				ctx->ip++;
				ctx->stack[*ctx->ip] = lsp_peek(ctx);
				break;
			case op_get:
				ctx->ip++;
				lsp_push(ctx, ctx->stack[*ctx->ip]);
				break;
			case op_set_arg:
				ctx->ip++;
				ctx->frame_start[*ctx->ip] = lsp_peek(ctx);
				break;
			case op_get_arg:
				ctx->ip++;
				lsp_push(ctx, ctx->frame_start[*ctx->ip]);
				break;
			case op_jump_if_false: {
				ctx->ip++;
				u16 offset = *((u16*)ctx->ip);
				ctx->ip += 1;

				if (is_falsey(lsp_peek(ctx))) { ctx->ip += offset; } 
			} break;
			case op_jump: {
				ctx->ip++;
				u16 offset = *((u16*)ctx->ip);
				ctx->ip += 2;

				ctx->ip += offset;
				continue;
			} break;
			case op_back_jump: {
				ctx->ip++;
				u16 offset = *((u16*)ctx->ip);

				ctx->ip -= offset;
				continue;
			} break;
			case op_not:
				lsp_push(ctx, lsp_make_bool(is_falsey(lsp_pop(ctx))));
				break;
			case op_and:
				lsp_push(ctx, lsp_make_bool(is_falsey(lsp_pop(ctx)) && is_falsey(lsp_pop(ctx))));
				break;
			case op_or:
				lsp_push(ctx, lsp_make_bool(is_falsey(lsp_pop(ctx)) || is_falsey(lsp_pop(ctx))));
				break;
			case op_neg: {
				struct lsp_val v = lsp_pop(ctx);

				if (v.type != lsp_val_num) {
					lsp_exception(ctx, "Operand to `neg' must be a number.");
					return lsp_make_nil();
				}

				lsp_push(ctx, lsp_make_num(-v.as.num));
			} break;
			case op_call: {
				ctx->ip++;
				u8 argc = *ctx->ip;

				struct lsp_val v = lsp_pop(ctx);

				ctx->frame_start = (ctx->stack_top - argc) + 1;

				u8* old_ip = ctx->ip;
				struct lsp_val* old_stack_top = ctx->stack_top;
				struct lsp_chunk* old_chunk = ctx->chunk;

				if (!(v.type == lsp_val_obj && v.as.obj->type == lsp_obj_fun)) {
					lsp_exception(ctx, "Value not callable.");
					return lsp_make_nil();
				}

				if (v.as.obj->as.fun.argc != argc) {
					lsp_exception(ctx, "Incorrect number of arguments to function.");
				}

				struct lsp_val ret = lsp_eval(ctx, v.as.obj->as.fun.chunk);

				ctx->stack_top = old_stack_top - argc;

				lsp_push(ctx, ret);

				ctx->chunk = old_chunk;
				ctx->ip = old_ip;

				ctx->frame_start = null;
			} break;
			case op_call_nat: {
				ctx->ip++;
				u8 idx = *ctx->ip;

				struct lsp_nat* nat = ctx->natives + idx;

				struct lsp_val* old_stack_top = ctx->stack_top;

				struct lsp_val ret = nat->fun(ctx, nat->argc, (ctx->stack_top - nat->argc) + 1);

				ctx->stack_top = old_stack_top - nat->argc;

				lsp_push(ctx, ret);
			} break;
			default: break;
		}

		ctx->ip++;
	}

eval_halt:
	return *ctx->stack_top;
}

enum {
	tok_left_paren = 0,
	tok_right_paren,
	tok_add,
	tok_mul,
	tok_div,
	tok_sub,
	tok_lt,
	tok_lte,
	tok_gt,
	tok_gte,
	tok_eq,
	tok_and,
	tok_or,
	tok_not,
	tok_number,
	tok_str,
	tok_iden,
	tok_print,
	tok_put,
	tok_set,
	tok_nil,
	tok_true,
	tok_false,
	tok_cat,
	tok_if,
	tok_neg,
	tok_fun,
	tok_ret,
	tok_while,
	tok_keyword_count,
	tok_end,
	tok_error
};

struct token {
	u32 type;
	u32 line;
	const char* start;
	u32 len;
};

#define max_locals 256

struct local {
	const char* start;
	u32 len;

	u32 hash;

	u32 depth;

	u32 pos;

	bool is_arg;
};

struct parser {
	u32 line;
	const char* cur;
	const char* start;
	const char* source;
	struct token token;

	struct lsp_chunk* chunk;

	u32 scope_depth;

	struct local locals[max_locals];
	u32 local_count;
};

static const char* keywords[] = {
	[tok_print] = "print",
	[tok_put]   = "put",
	[tok_set]   = "set",
	[tok_nil]   = "nil",
	[tok_true]  = "true",
	[tok_false] = "false",
	[tok_cat]   = "cat",
	[tok_if]    = "if",
	[tok_neg]   = "neg",
	[tok_fun]   = "fun",
	[tok_ret]   = "ret",
	[tok_while] = "while"
};

static struct token make_token(struct parser* parser, u32 type) {
	return (struct token) {
		.type = type,
		.line = parser->line,
		.start = parser->start,
		.len = (u32)(parser->cur - parser->start)
	};
}

static struct token error_token(struct parser* parser, const char* message) {
	return (struct token) {
		.type = tok_error,
		.start = message,
		.len = strlen(message),
		.line = parser->line
	};
}

static char parser_advance(struct parser* parser) {
	parser->cur++;
	return parser->cur[-1];
}

static char parser_peek(struct parser* parser) {
	return *parser->cur;
}

static char parser_peek_next(struct parser* parser) {
	if (!*parser->cur) { return '\0'; }
	return parser->cur[1];
}

static bool parser_at_end(struct parser* parser) {
	return *parser->cur == '\0';
}

static bool parser_match(struct parser* parser, char expected) {
	if (parser_at_end(parser)) return false;
	if (*parser->cur != expected) return false;
	parser->cur++;
	return true;
}

static void skip_whitespace(struct parser* parser) {
	/* Skip over whitespace and comments */
	while (1) {
		switch (parser_peek(parser)) {
			case ' ':
			case '\r':
			case '\t':
				parser_advance(parser);
				break;
			case '\n':
				parser_advance(parser);
				parser->line++;
				break;
			case ';':
				while (parser_peek(parser) != '\n' && !parser_at_end(parser)) {
					parser_advance(parser);
				}
				break;
			default: return;
		}
	}
}

static bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
	return
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		 c == '_';
}

static struct token next_tok(struct parser* parser) {
	skip_whitespace(parser);

	parser->start = parser->cur;

	if (parser_at_end(parser)) { return (struct token) { .type = tok_end }; }

	char c = parser_advance(parser);

	if (is_alpha(c)) {
		while (is_alpha(parser_peek(parser)) || is_digit(parser_peek(parser))) {
			parser_advance(parser);
		}

		struct token tok = make_token(parser, tok_iden);

		for (u32 i = tok_print; i < tok_keyword_count; i++) { /* Check for keywords. */
			if (strlen(keywords[i]) == tok.len && memcmp(parser->start, keywords[i], tok.len) == 0) {
				tok.type = i;
				return tok;
			}
		}

		return tok;
	}

	switch (c) {
		case '(': return make_token(parser, tok_left_paren);
		case ')': return make_token(parser, tok_right_paren);
		case '+': return make_token(parser, tok_add);
		case '*': return make_token(parser, tok_mul);
		case '/': return make_token(parser, tok_div);
		case '-': return make_token(parser, tok_sub);

		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9': {
			while (is_digit(parser_peek(parser))) {
				parser_advance(parser);
			}

			if (parser_peek(parser) == '.' && is_digit(parser_peek_next(parser))) {
				parser_advance(parser);

				while (is_digit(parser_peek(parser))) {
					parser_advance(parser);
				}
			}

			return make_token(parser, tok_number);
		} break;

		case '&': return make_token(parser, tok_and);
		case '|': return make_token(parser, tok_or);
		case '!': return make_token(parser, tok_not);
		case '=': return make_token(parser, tok_eq);

		case '<': return make_token(parser, parser_match(parser, '=') ? tok_lte : tok_lt);
		case '>': return make_token(parser, parser_match(parser, '=') ? tok_gte : tok_gt);

		case '"': {
			parser->start++;
			while (parser_peek(parser) != '"' && !parser_at_end(parser)) {
				if (parser_peek(parser) == '\n') { parser->line++; }
				parser_advance(parser);
			}

			if (parser_at_end(parser)) {
				return error_token(parser, "Unterminated string.");
			}

			struct token tok = make_token(parser, tok_str);
			parser_advance(parser);
			return tok;
		} break;
	}

	return error_token(parser, "Unexpected character.");
}

#define advance() \
	tok = next_tok(parser)

#define expect_tok(t_, err_) \
	do { \
		if ((tok).type != t_) { \
			parse_error(ctx, parser, err_); \
			return false; \
		} \
	} while (0)

#define parser_recurse() do { if (!parse(ctx, parser, chunk)) { return false; } } while (0)

/* Over-engineered function to print an error message. It does GCC-style error printing if
 * ctx->simple_errors is disabled, where it points to the position of the error on the line,
 * like this:
 *
 * error: Unexpected token.
 *    1 | (+ + 10)
 *           ^
 *
 * Simple error printing is for when only a single line is allowed for the error message,
 * such as for a status bar.
 *
 * ASCII escape codes are used to give coloured output if printing to stdout or stderr. Otherwise,
 * colours are not printed, to avoid garbled text if dumping errors to a file instead of standard
 * output. TODO: Use the Win32 API to give colored output on the Windows console, which doesn't
 * support ASCII escape codes. */
static void parse_error(struct lsp_state* ctx, struct parser* parser, const char* message, ...) {
	const char* line_start = parser->cur - 1;
	while (line_start != parser->source && *line_start != '\n') {
		line_start--;
	}

	line_start++;

	u32 line_len = 0;
	for (const char* c = line_start; *c && *c != '\n'; c++) {
		line_len++;
	}

	u32 col = 0;
	for (const char* c = line_start; *c && c != parser->cur; c++) {
		col++;
	}

	if (!ctx->simple_errors && (ctx->error == stdout || ctx->error == stderr)) {
		fprintf(ctx->error, "\033[1;31merror \033[0m");
		fprintf(ctx->error, "[line %d:%d]: ", parser->line, col);

		va_list l;
		va_start(l, message);
		vfprintf(ctx->error, message, l);
		va_end(l);

		fprintf(ctx->error, "\n");

		char to_print[256];
		memcpy(to_print, line_start, 256 > line_len ? line_len : 256);

		fprintf(ctx->error, "\033[0;33m%10d\033[0m %.*s\n", parser->line, line_len, to_print);
		fprintf(ctx->error, "           ");
		fprintf(ctx->error, "\033[1;35m");
		for (u32 i = 0; i < col - 1; i++) {
			if (to_print[i] == '\t') {
				fprintf(ctx->error, "\t");
			} else {
				fprintf(ctx->error, " ");
			}
		}
		fprintf(ctx->error, "^\033[0m\n");
	} else if (!ctx->simple_errors) {
		fprintf(ctx->error, "\033[1;31merror \033[0m");
		fprintf(ctx->error, "[line %d:%d]: ", parser->line, col);

		va_list l;
		va_start(l, message);
		vfprintf(ctx->error, message, l);
		va_end(l);

		fprintf(ctx->error, "\n");

		char to_print[256];
		memcpy(to_print, line_start, 256 > line_len ? line_len : 256);

		fprintf(ctx->error, "%10d | %.*s\n", parser->line, line_len, to_print);
		fprintf(ctx->error, "             ");
		fprintf(ctx->error, "\033[1;35m");
		for (u32 i = 0; i < col - 1; i++) {
			if (to_print[i] == '\t') {
				fprintf(ctx->error, "\t");
			} else {
				fprintf(ctx->error, " ");
			}
		}
		fprintf(ctx->error, "^\033[0m\n");
	} else { 
		fprintf(ctx->error, "error [line %d]: ", parser->line);

		va_list l;
		va_start(l, message);
		vfprintf(ctx->error, message, l);
		va_end(l);

		fprintf(ctx->error, "\n");
	}
}

static void parser_begin_scope(struct lsp_state* ctx, struct parser* parser) {
	parser->scope_depth++;
}

static void parser_end_scope(struct lsp_state* ctx, struct parser* parser) {
	parser->scope_depth--;

	u8 pop_count = 0;

	while (parser->local_count > 0 &&
		parser->locals[parser->local_count - 1].depth > parser->scope_depth) {
		pop_count++;
		parser->local_count--;
	}

	lsp_chunk_add_op(ctx, parser->chunk, op_pop_n, parser->line);
	lsp_chunk_add_op(ctx, parser->chunk, pop_count, parser->line);
}

static u16 emit_jump(struct lsp_state* ctx, struct parser* parser, struct lsp_chunk* chunk, u8 op) {
	lsp_chunk_add_op(ctx, chunk, op, parser->line);
	lsp_chunk_add_op(ctx, chunk, 0, parser->line);
	lsp_chunk_add_op(ctx, chunk, 0, parser->line);
	return chunk->count - 2;
}

static void patch_jump(struct lsp_state* ctx, struct parser* parser, struct lsp_chunk* chunk, u16 offset) {
	u16 jump = (u16)chunk->count - offset - 2;

	*((u16*)(chunk->code + offset)) = jump;
}

#define resolve_variable() \
	do { \
		bool resolved = false; \
		for (u32 i = 0; i < parser->local_count; i++) { \
			struct local* l = parser->locals + i; \
			if (l->len == tok.len && \
				memcmp(l->start, tok.start, tok.len) == 0) { \
				if (l->is_arg) { \
					lsp_chunk_add_op(ctx, chunk, op_get_arg, parser->line); \
				} else { \
					lsp_chunk_add_op(ctx, chunk, op_get, parser->line); \
				} \
				lsp_chunk_add_op(ctx, chunk, (u8)l->pos, parser->line); \
				resolved = true; \
				break; \
			} \
		} \
		if (!resolved) { \
			parse_error(ctx, parser, "Failed to resolve identifier: `%.*s'.", tok.len, tok.start); \
			return false; \
		} \
	} while (0)

#define parse_block() \
	do { \
		while (1) { \
			if (!parse(ctx, parser, chunk)) { \
				return false; \
			} \
			struct token tok = parser->token; \
			const char* cur = parser->cur; \
			u32 p_line = parser->line; \
			parser->token = next_tok(parser); \
			bool found = false; \
			if (parser->token.type == tok_right_paren) { \
				found = true; \
			} \
			parser->cur = cur; \
			parser->token = tok; \
			parser->line = p_line; \
			if (found) { \
				break; \
			} \
		} \
	} while (0)

#define count_args() \
	do { \
		while (1) { \
			struct token tok = parser->token; \
			const char* cur = parser->cur; \
			parser->token = next_tok(parser); \
			if (parser->token.type == tok_right_paren) { \
				parser->cur = cur; \
				parser->token = tok; \
				break; \
			} \
			parser->cur = cur; \
			parser->token = tok; \
			if (!parse(ctx, parser, chunk)) { \
				return false; \
			} \
			tok = parser->token; \
			cur = parser->cur; \
			parser->token = next_tok(parser); \
			bool end = false; \
			if (parser->token.type == tok_right_paren) { \
				end = true; \
			} \
			parser->cur = cur; \
			parser->token = tok; \
			argc++; \
			if (end) { \
				break; \
			} \
		} \
	} while (0) 

static bool parse(struct lsp_state* ctx, struct parser* parser, struct lsp_chunk* chunk) {
	struct token tok;

	parser->chunk = chunk;

	advance();

	if (tok.type == tok_error) {
		parse_error(ctx, parser, tok.start);
	} else if (tok.type == tok_left_paren) {
		advance();

		if (tok.type == tok_add) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_add, parser->line);
		} else if (tok.type == tok_sub) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_sub, parser->line);
		} else if (tok.type == tok_div) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_div, parser->line);
		} else if (tok.type == tok_mul) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_mul, parser->line);
		} else if (tok.type == tok_lt) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_lt, parser->line);
		} else if (tok.type == tok_lte) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_lte, parser->line);
		} else if (tok.type == tok_gt) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_gt, parser->line);
		} else if (tok.type == tok_gte) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_gte, parser->line);
		} else if (tok.type == tok_eq) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_eq, parser->line);
		} else if (tok.type == tok_and) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_and, parser->line);
		} else if (tok.type == tok_or) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_or, parser->line);
		 } else if (tok.type == tok_cat) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_cat, parser->line);
		} else if (tok.type == tok_print) {
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_print, parser->line);
		} else if (tok.type == tok_put) {
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_put, parser->line);
		} else if (tok.type == tok_not) {
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_not, parser->line);
		} else if (tok.type == tok_neg) {
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_neg, parser->line);
		} else if (tok.type == tok_set) {
			advance();
			expect_tok(tok_iden, "Expected an identifier.");

			bool declare = true;

			struct local l = {
				.start = tok.start,
				.len = tok.len,
				.hash = elf_hash((const u8*)tok.start, tok.len)
			};

			for (u32 i = 0; i < parser->local_count; i++) {
				struct local* lo = parser->locals + i;

				if (lo->len == tok.len &&
					memcmp(lo->start, tok.start, tok.len) == 0) {
					declare = false;
					l = *lo;
					break;
				}
			}

			if (declare) {
				l.pos = parser->local_count;
				l.depth = parser->scope_depth;

				parser->locals[parser->local_count++] = l;
			}

			parser_recurse();

			if (l.is_arg) {
				lsp_chunk_add_op(ctx, chunk, op_set_arg, parser->line);
			} else {
				lsp_chunk_add_op(ctx, chunk, op_set, parser->line);
			}
			lsp_chunk_add_op(ctx, chunk, (u8)l.pos, parser->line);

			if (!declare) {
				lsp_chunk_add_op(ctx, chunk, op_pop, parser->line);
			}
		} else if (tok.type == tok_if) {
			parser_recurse(); /* Condition */

			u16 then_jump = emit_jump(ctx, parser, chunk, op_jump_if_false);
			lsp_chunk_add_op(ctx, chunk, op_pop, parser->line); /* Pop the condition */

			parser_begin_scope(ctx, parser);
			
			/* Then clause */
			advance();
			expect_tok(tok_left_paren, "Expected a block after condition.");
			parse_block();
			advance();
			expect_tok(tok_right_paren, "Expected `)' after block.");

			parser_end_scope(ctx, parser);

			u16 else_jump = emit_jump(ctx, parser, chunk, op_jump);

			patch_jump(ctx, parser, chunk, then_jump);
			lsp_chunk_add_op(ctx, chunk, op_pop, parser->line);

			parser_begin_scope(ctx, parser);

			/* Else clause */
			advance();
			expect_tok(tok_left_paren, "Expected an else clause.");
			parse_block();
			advance();
			expect_tok(tok_right_paren, "Expected `)' after block.");

			parser_end_scope(ctx, parser);

			tok = parser->token;

			patch_jump(ctx, parser, chunk, else_jump);
		} else if (tok.type == tok_while) {
			u16 start = chunk->count;

			parser_recurse(); /* Condition */

			u16 cond_jump = emit_jump(ctx, parser, chunk, op_jump_if_false);
			lsp_chunk_add_op(ctx, chunk, op_pop, parser->line); /* Pop the condition */

			parser_begin_scope(ctx, parser);

			/* Clause */
			advance();
			expect_tok(tok_left_paren, "Expected a block after condition.");
			parse_block();
			advance();
			expect_tok(tok_right_paren, "Expected `)' after block.");

			parser_end_scope(ctx, parser);

			tok = parser->token;

			lsp_chunk_add_op(ctx, chunk, op_back_jump, parser->line);
			u16 offset = chunk->count - start;
			*((u16*)(chunk->code + chunk->count)) = offset;
			chunk->count += 2;

			patch_jump(ctx, parser, chunk, cond_jump);

			lsp_chunk_add_op(ctx, chunk, op_pop, parser->line);
		} else if (tok.type == tok_fun) {
			advance();
			expect_tok(tok_left_paren, "Expected `(' after `fun'.");

			parser_begin_scope(ctx, parser);

			u32 argc = 0;

			while (1) {
				advance();

				if (tok.type == tok_right_paren) {
					break;
				}

				expect_tok(tok_iden, "Expected an identifier.");

				struct local l = {
					.start = tok.start,
					.len = tok.len,
					.hash = elf_hash((const u8*)tok.start, tok.len),
					.is_arg = true
				};

				for (u32 i = 0; i < parser->local_count; i++) {
					struct local* lo = parser->locals + i;

					if (lo->len == tok.len &&
						memcmp(lo->start, tok.start, tok.len) == 0) {
						parse_error(ctx, parser, "A variable with the name `%.*s' already exists.", tok.len, tok.start);
						return false;
					}
				}

				l.pos = argc;
				l.depth = parser->scope_depth;

				parser->locals[parser->local_count++] = l;

				argc++;
			}

			advance();
			expect_tok(tok_left_paren, "Expected a block after argument list.");

			struct lsp_chunk* old_chunk = chunk;
			struct lsp_chunk* new_chunk = core_calloc(1, sizeof(struct lsp_chunk));

			chunk = new_chunk;

			parse_block();

			/* Default return value is nil. */
			lsp_chunk_add_op(ctx, chunk, op_push_nil, parser->line);
			lsp_chunk_add_op(ctx, chunk, op_halt, parser->line);

			parser_end_scope(ctx, parser);

			chunk = old_chunk;

			advance();
			expect_tok(tok_right_paren, "Expected `)' after block.");

			u8 a = lsp_chunk_add_const(ctx, chunk, lsp_make_fun(ctx, new_chunk, argc));
			lsp_chunk_add_op(ctx, chunk, op_push, parser->line);
			lsp_chunk_add_op(ctx, chunk, a, parser->line);
		} else if (tok.type == tok_ret) {
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_halt, parser->line);
		} else if (tok.type == tok_nil) {
			/* Empty statement. */
		} else if (tok.type == tok_iden) {
			/* Resolve function call */

			bool resolved = false;

			for (u32 i = 0; i < ctx->nat_count; i++) {
				struct lsp_nat* nat = ctx->natives + i;

				if (memcmp(nat->name, tok.start, tok.len) == 0) {
					u32 argc = 0;

					count_args();

					if (argc != nat->argc) {
						parse_error(ctx, parser, "Incorrect number of arguments to native function. Expected %d; found %d.", nat->argc, argc);
						return false;
					}

					lsp_chunk_add_op(ctx, chunk, op_call_nat, parser->line);
					lsp_chunk_add_op(ctx, chunk, (u8)i, parser->line);

					resolved = true;
					goto resolved_l;
				}
			}

			for (u32 i = 0; i < parser->local_count; i++) {
				struct local* l = parser->locals + i;

				if (l->len == tok.len &&
					memcmp(l->start, tok.start, tok.len) == 0) {

					u32 argc = 0;

					count_args();

					lsp_chunk_add_op(ctx, chunk, op_push, parser->line);
					lsp_chunk_add_op(ctx, chunk, (u8)l->pos, parser->line);
					lsp_chunk_add_op(ctx, chunk, op_call, parser->line);
					lsp_chunk_add_op(ctx, chunk, (u8)argc, parser->line);

					resolved = true;
					break;
				}
			}

resolved_l:
			if (!resolved) {
				parse_error(ctx, parser, "Failed to resolve function: `%.*s'.", tok.len, tok.start);
				return false;
			}
		} else {
			parse_error(ctx, parser, "Unexpected token.");
		}

		advance();
		expect_tok(tok_right_paren, "Expected `)'.");
	} else if (tok.type == tok_iden) {
		/* Resolve variable */
		resolve_variable();
	} else if (tok.type == tok_number) {
		double n = strtod(tok.start, null);
		u8 a = lsp_chunk_add_const(ctx, chunk, lsp_make_num(n));
		lsp_chunk_add_op(ctx, chunk, op_push, parser->line);
		lsp_chunk_add_op(ctx, chunk, a, parser->line);
	} else if (tok.type == tok_str) {
		u8 a = lsp_chunk_add_const(ctx, chunk, lsp_make_str(ctx, tok.start, tok.len));
		lsp_chunk_add_op(ctx, chunk, op_push, parser->line);
		lsp_chunk_add_op(ctx, chunk, a, parser->line);
	} else if (tok.type == tok_nil) {
		lsp_chunk_add_op(ctx, chunk, op_push_nil, parser->line);
	} else if (tok.type == tok_true) {
		lsp_chunk_add_op(ctx, chunk, op_push_true, parser->line);
	} else if (tok.type == tok_false) {	
		lsp_chunk_add_op(ctx, chunk, op_push_false, parser->line);
	} else if (tok.type == tok_end) {
		parser->token = tok;
		return true;
	} else {
		parse_error(ctx, parser, "Unexpected token.");
		return false;
	}

	parser->token = tok;

	return true;
}

struct lsp_val lsp_do_string(struct lsp_state* ctx, const char* str) {
	struct lsp_chunk chunk = { 0 };

	struct parser parser = { 0 };
	parser.line = 1;
	parser.start = str;
	parser.cur = str;
	parser.source = str;

	parser_begin_scope(ctx, &parser);

	while (parser.token.type != tok_end) {
		if (!parse(ctx, &parser, &chunk)) {
			return lsp_make_nil();
		}
	}

	parser_end_scope(ctx, &parser);

	lsp_chunk_add_op(ctx, &chunk, op_halt, parser.line);
	return lsp_eval(ctx, &chunk);
}

struct lsp_val lsp_do_file(struct lsp_state* ctx, const char* file_path) {
	char* buf;
	u64 size;

	if (read_raw(file_path, (u8**)&buf, &size, true)) {
		struct lsp_val r = lsp_do_string(ctx, buf);

		core_free(buf);

		return r;
	}

	return lsp_make_nil();
}

void lsp_register(struct lsp_state* ctx, const char* name, u32 argc, lsp_nat_fun_t fun) {
	if (ctx->nat_count >= max_natives) {
		fprintf(ctx->error, "Failed to register native function `%s': Too many natives. Max: %d\n", name, max_natives);
		return;
	}

	struct lsp_nat* nat = ctx->natives + ctx->nat_count++;
	nat->name = copy_string(name);
	nat->fun = fun;
	nat->argc = argc;
}

/* Standard library */
struct lsp_val std_get_mem(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
	return lsp_make_num(core_get_memory_usage());
}

struct lsp_val std_get_stack_count(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
	return lsp_make_num(lsp_get_stack_count(ctx));
}

struct lsp_val std_bit_and(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
	return lsp_make_num((u64)args[0].as.num & (u64)args[1].as.num);
}

struct lsp_val std_bit_or(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
	return lsp_make_num((u64)args[0].as.num | (u64)args[1].as.num);
}

struct lsp_val std_shift_left(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
	return lsp_make_num((u64)args[0].as.num << (u64)args[1].as.num);
}

struct lsp_val std_shift_right(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
	return lsp_make_num((u64)args[0].as.num >> (u64)args[1].as.num);
}

struct lsp_val std_mod(struct lsp_state* ctx, u32 argc, struct lsp_val* args) {
	return lsp_make_num((u64)args[0].as.num % (u64)args[1].as.num);
}

void lsp_register_std(struct lsp_state* ctx) {
	lsp_register(ctx, "memory_usage", 0, std_get_mem);
	lsp_register(ctx, "stack_count", 0, std_get_stack_count);
	lsp_register(ctx, "bit_and", 2, std_bit_and);
	lsp_register(ctx, "bit_or", 2, std_bit_or);
	lsp_register(ctx, "shift_left", 2, std_shift_left);
	lsp_register(ctx, "shift_right", 2, std_shift_right);
	lsp_register(ctx, "mod", 2, std_mod);
}
