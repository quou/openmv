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

enum {
	op_halt = 0,
	op_push,
	op_push_nil,
	op_push_true,
	op_push_false,
	op_pop,
	op_add,
	op_sub,
	op_mul,
	op_div,
	op_cat,
	op_print,
	op_set,
	op_get,
	op_jump_if_false,
	op_jump,
	op_not,
	op_neg,
	op_call
};

struct lsp_chunk {
	u8 code[chunk_size];
	u32 lines[chunk_size];

	u32 count;

	struct lsp_val consts[chunk_max_constants];
	u32 const_count;
};

struct lsp_state {
	struct lsp_val stack[stack_size];
	struct lsp_val* stack_top;

	bool simple_errors;

	u8* ip;

	struct lsp_chunk* chunk;

	bool exception;

	FILE* error;
	FILE* info;

	struct lsp_obj* objs;
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

struct lsp_val lsp_make_fun(struct lsp_state* ctx, struct lsp_chunk* chunk) {
	struct lsp_obj* obj = lsp_new_obj(ctx, lsp_obj_fun);

	struct lsp_val v = {
		.type = lsp_val_obj,
		.as.obj = obj
	};

	obj->as.fun.chunk = chunk;

	return v;
}

struct lsp_state* new_lsp_state(void* error, void* info) {
	if (!error) { error = stderr; }
	if (!info)  { info = stdout; }

	struct lsp_state* state = core_calloc(1, sizeof(struct lsp_chunk));

	state->objs = core_calloc(max_objs, sizeof(struct lsp_obj));

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

	core_free(state->objs);
	core_free(state);
}

void lsp_set_simple_errors(struct lsp_state* state, bool simple) {
	state->simple_errors = simple;
}

void lsp_push(struct lsp_state* ctx, struct lsp_val val) {
	*(++ctx->stack_top) = val;
}

struct lsp_val lsp_pop(struct lsp_state* ctx) {
	return *(ctx->stack_top--);
}

struct lsp_val lsp_peek(struct lsp_state* ctx) {
	return *ctx->stack_top;
}

static u32 lsp_get_stack_count(struct lsp_state* ctx) {
	return (u32)(ctx->stack_top - ctx->stack) / sizeof(struct lsp_val);
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

#define arith_op(op_) do { \
		struct lsp_val b = lsp_pop(ctx); \
		struct lsp_val a = lsp_pop(ctx); \
		if (a.type != lsp_val_num || b.type != lsp_val_num) { \
			lsp_exception(ctx, "Operands to `%s' must be numbers.", #op_); \
			return lsp_make_nil(); \
		} \
		lsp_push(ctx, lsp_make_num(a.as.num op_ b.as.num)); \
	} while (0)

static void print_obj(FILE* out, struct lsp_obj* obj) {
	switch (obj->type) {
		case lsp_obj_str:
			fprintf(out, "%.*s", obj->as.str.len, obj->as.str.chars);
			break;
		case lsp_obj_fun:
			fprintf(out, "<function %p>", obj->as.fun.chunk);
		default: break;
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
		default: break;
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
	if (ctx->exception) {
		return lsp_make_nil();
	}

	ctx->chunk = chunk;
	ctx->ip = chunk->code;

	while (1) {
		switch (*ctx->ip) {
			case op_halt:
				for (u32 i = 0; i < ctx->obj_count; i++) { /* Collect garbage */
					struct lsp_obj* obj = ctx->objs + i;

					if (obj->ref == 0) {
						lsp_free_obj(ctx, obj);
					}
				}
				goto eval_halt;
			case op_push:
				ctx->ip++;
				lsp_push(ctx, chunk->consts[*ctx->ip]);
				break;
			case op_push_nil:   lsp_push(ctx, lsp_make_nil()); break;
			case op_push_true:  lsp_push(ctx, lsp_make_bool(true)); break;
			case op_push_false: lsp_push(ctx, lsp_make_bool(false)); break;
			case op_pop: {
				ctx->ip++;
				for (u8 i = 0; i < *ctx->ip; i++) {
					struct lsp_val v = lsp_peek(ctx);

					if (v.type == lsp_val_obj) {
						v.as.obj->ref--;
					}

					lsp_pop(ctx);
				}
			} break;
			case op_add: arith_op(+); break;
			case op_sub: arith_op(-); break;
			case op_div: arith_op(/); break;
			case op_mul: arith_op(*); break;
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
			case op_set: {
				ctx->ip++;

				struct lsp_val old = ctx->stack[*ctx->ip];
				if (old.type == lsp_val_obj) {
					old.as.obj->ref--;
				}

				struct lsp_val new = lsp_peek(ctx);
				if (new.type == lsp_val_obj) {
					new.as.obj->ref++;
				}

				ctx->stack[*ctx->ip] = new;
			} break;
			case op_get:
				ctx->ip++;
				lsp_push(ctx, ctx->stack[*ctx->ip]);
				break;
			case op_jump_if_false: {
				ctx->ip++;
				u16 offset = *((u16*)ctx->ip);
				ctx->ip += 1;

				print_val(ctx->info, lsp_peek(ctx));

				if (is_falsey(lsp_peek(ctx))) { ctx->ip += offset; } 
			} break;
			case op_jump: {
				ctx->ip++;
				u16 offset = *((u16*)ctx->ip);
				ctx->ip += 2;

				ctx->ip += offset;
				continue;
			} break;
			case op_not:
				lsp_push(ctx, lsp_make_bool(is_falsey(lsp_pop(ctx))));
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
				struct lsp_val v = lsp_pop(ctx);

				u8* old_ip = ctx->ip;
				struct lsp_val* old_stack_top = ctx->stack_top;
				struct lsp_chunk* old_chunk = ctx->chunk;

				if (!(v.type == lsp_val_obj && v.as.obj->type == lsp_obj_fun)) {
					lsp_exception(ctx, "Value not callable.");
					return lsp_make_nil();
				}

				struct lsp_val ret = lsp_eval(ctx, v.as.obj->as.fun.chunk);

				ctx->stack_top = old_stack_top;

				lsp_push(ctx, ret);

				ctx->chunk = old_chunk;
				ctx->ip = old_ip;
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
	tok_number,
	tok_str,
	tok_iden,
	tok_print,
	tok_set,
	tok_nil,
	tok_true,
	tok_false,
	tok_cat,
	tok_if,
	tok_not,
	tok_neg,
	tok_fun,
	tok_ret,
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
};

struct parser {
	u32 line;
	const char* cur;
	const char* start;
	struct token token;

	struct lsp_chunk* chunk;

	u32 scope_depth;

	struct local locals[max_locals];
	u32 local_count;
};

static const char* keywords[] = {
	[tok_print] = "print",
	[tok_set]   = "set",
	[tok_nil]   = "nil",
	[tok_true]  = "true",
	[tok_false] = "false",
	[tok_cat]   = "cat",
	[tok_if]    = "if",
	[tok_not]   = "not",
	[tok_neg]   = "neg",
	[tok_fun]   = "fun",
	[tok_ret]   = "ret"
};

static struct token make_token(struct parser* parser, u32 type, u32 len) {
	return (struct token) {
		.type = type,
		.line = parser->line,
		.start = parser->cur,
		.len = len
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

static void skip_whitespace(struct parser* parser) {
	/* Skip over whitespace and comments */
	while (1) {
		switch (*parser->cur) {
			case '\n':
				parser->line++;
			case ' ':
			case '\r':
			case '\t':
				parser->cur++;
				break;
			case ';':
				while (*parser->cur && *parser->cur != '\n') parser->cur++;
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
	parser->cur++;

	skip_whitespace(parser);

	if (!*parser->cur) { return (struct token) { .type = tok_end }; }

	for (u32 i = tok_print; i < tok_keyword_count; i++) {
		u32 len = (u32)strlen(keywords[i]);
		if (memcmp(parser->cur, keywords[i], len) == 0) {
			struct token tok = make_token(parser, i, len);
			parser->cur += len - 1;
			return tok;
		}
	}

	if (is_alpha(*parser->cur)) {
		u32 len = 0;
		const char* c = parser->cur;
		while (is_alpha(*c)) {
			c++;
			len++;
		}

		struct token tok = make_token(parser, tok_iden, len);

		parser->cur += len - 1;

		return tok;
	}

	switch (*parser->cur) {
		case '(': return make_token(parser, tok_left_paren, 1);
		case ')': return make_token(parser, tok_right_paren, 1);
		case '+': return make_token(parser, tok_add, 1);
		case '*': return make_token(parser, tok_mul, 1);
		case '/': return make_token(parser, tok_div, 1);

		case '-': case '0': case '1': case '2':
		case '3': case '4': case '5': case '6':
		case '7': case '8': case '9': {
			if (*parser->cur == '-' && !is_digit(*(parser->cur + 1))) {
				return make_token(parser, tok_sub, 1);
			}

			/* Length is not required, number is parsed later
			 * using strtod. */
			struct token t = make_token(parser, tok_number, 0);

			if (*parser->cur == '-') {
				parser->cur++;
			}

			while (is_digit(*parser->cur)) {
				parser->cur++;
			}

			if (*parser->cur == '.') {
				parser->cur++;
				while (is_digit(*parser->cur)) {
					parser->cur++;
				}
			}

			parser->cur--;

			return t;
		} break;

		case '"': {
			parser->cur++;
			u32 len = 0;
			for (const char* c = parser->cur; *c && *c != '"'; c++) {
				len++;
			}

			struct token t = make_token(parser, tok_str, len);

			parser->cur += len;

			return t;
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
 * ctx->simple_errors is disabled, where it points to the position of the error on the line.
 *
 * Simple error printing is for when only a single line is allowed for the error message,
 * such as for a status bar. */
static void parse_error(struct lsp_state* ctx, struct parser* parser, const char* message, ...) {
	u32 col = 0;

	for (const char* c = parser->cur; *c != '\n' && c != parser->start; c--) {
		col++;
	}

	u32 line_len = col;
	for (const char* c = parser->cur; *c && *c != '\n'; c++) {
		line_len++;
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
		memcpy(to_print, parser->cur - col, 256 > line_len ? line_len : 256);

		u32 offset = 0;

		if (to_print[0] == '\n') {
			offset++;
		}

		if (to_print[line_len - 1] == '\n') {
			line_len--;
		}

		fprintf(ctx->error, "%10d| %.*s\n", parser->line, line_len - 1, to_print + offset);
		fprintf(ctx->error, "            ");
		fprintf(ctx->error, "\033[1;35m");
		for (u32 i = 0; i < col - 1; i++) {
			if ((to_print + offset)[i] == '\t') {
				fprintf(ctx->error, "\t");
			} else {
				fprintf(ctx->error, " ");
			}
		}
		fprintf(ctx->error, "^\033[0m\n");
	} else if (!ctx->simple_errors) {
		fprintf(ctx->error, "error [line %d:%d]: ", parser->line, col);

		va_list l;
		va_start(l, message);
		vfprintf(ctx->error, message, l);
		va_end(l);

		fprintf(ctx->error, "\n");
		
		char to_print[256];
		memcpy(to_print, parser->cur - col, 256 > line_len ? line_len : 256);

		u32 offset = 0;

		if (to_print[0] == '\n') {
			offset++;
		}

		if (to_print[line_len - 1] == '\n') {
			line_len--;
		}

		fprintf(ctx->error, "%10d| %.*s\n", parser->line, line_len - 1, to_print + offset);
		fprintf(ctx->error, "            ");
		for (u32 i = 0; i < col - 1; i++) {
			if ((to_print + offset)[i] == '\t') {
				fprintf(ctx->error, "\t");
			} else {
				fprintf(ctx->error, " ");
			}
		}
		fprintf(ctx->error, "^\n");
	} else {
		fprintf(ctx->error, "error [line %d:%d]: ", parser->line, col);

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

	u8 pop_count;

	while (parser->local_count > 0 &&
		parser->locals[parser->local_count - 1].depth > parser->scope_depth) {
		pop_count++;
		parser->local_count--;
	}

	lsp_chunk_add_op(ctx, parser->chunk, op_pop, parser->line);
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
		 } else if (tok.type == tok_cat) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_cat, parser->line);
		} else if (tok.type == tok_print) {
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_print, parser->line);
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
			}

			if (declare) {
				parser->locals[parser->local_count++] = l;
			}

			lsp_chunk_add_op(ctx, chunk, op_set, parser->line);
			lsp_chunk_add_op(ctx, chunk, (u8)l.pos, parser->line);

			parser_recurse();
		} else if (tok.type == tok_if) {
			parser_recurse(); /* Condition */

			i16 then_jump = emit_jump(ctx, parser, chunk, op_jump_if_false);
			lsp_chunk_add_op(ctx, chunk, op_pop, parser->line); /* Pop the condition */
			lsp_chunk_add_op(ctx, chunk, 1, parser->line);
			
			/* Then clause */
			parser_recurse();

			i16 else_jump = emit_jump(ctx, parser, chunk, op_jump);
			lsp_chunk_add_op(ctx, chunk, op_pop, parser->line); /* Pop the condition */
			lsp_chunk_add_op(ctx, chunk, 1, parser->line);

			patch_jump(ctx, parser, chunk, then_jump);

			/* Else clause */
			parser_recurse();

			tok = parser->token;

			patch_jump(ctx, parser, chunk, else_jump);
		} else if (tok.type == tok_fun) {
			advance();
			expect_tok(tok_left_paren, "Expected `(' after `fun'.");

			parser_begin_scope(ctx, parser);

			struct lsp_chunk* old_chunk = chunk;
			struct lsp_chunk* new_chunk = core_calloc(1, sizeof(struct lsp_chunk));

			chunk = new_chunk;

			while (1) {
				if (!parse(ctx, parser, chunk)) {
					return false;
				}
				struct token tok = parser->token;
				const char* cur = parser->cur;
				parser->token = next_tok(parser);

				bool found = false;

				if (parser->token.type == tok_right_paren) {
					found = true;
				}

				parser->cur = cur;
				parser->token = tok;

				if (found) {
					break;
				}
			}

			/* Default return value is nil. */
			lsp_chunk_add_op(ctx, chunk, op_push_nil, parser->line);
			lsp_chunk_add_op(ctx, chunk, op_halt, parser->line);

			parser_end_scope(ctx, parser);

			chunk = old_chunk;

			advance();
			expect_tok(tok_right_paren, "Expected `)'.");

			u8 a = lsp_chunk_add_const(ctx, chunk, lsp_make_fun(ctx, new_chunk));
			lsp_chunk_add_op(ctx, chunk, op_push, parser->line);
			lsp_chunk_add_op(ctx, chunk, a, parser->line);
		} else if (tok.type == tok_ret) {
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_halt, parser->line);
		} else if (tok.type == tok_iden) {
			bool resolved = false;

			for (u32 i = 0; i < parser->local_count; i++) {
				struct local* l = parser->locals + i;

				if (l->len == tok.len &&
					memcmp(l->start, tok.start, tok.len) == 0) {

					lsp_chunk_add_op(ctx, chunk, op_push, parser->line);
					lsp_chunk_add_op(ctx, chunk, (u8)l->pos, parser->line);
					lsp_chunk_add_op(ctx, chunk, op_call, parser->line);

					resolved = true;
					break;
				}
			}

			if (!resolved) {
				parse_error(ctx, parser, "Failed to resolve identifier: `%.*s'.", tok.len, tok.start);
				return false;
			}
		} else {
			parse_error(ctx, parser, "Unexpected token.");
		}

		advance();
		expect_tok(tok_right_paren, "Expected `)'.");
	} else if (tok.type == tok_iden) {
		bool resolved = false;

		for (u32 i = 0; i < parser->local_count; i++) {
			struct local* l = parser->locals + i;

			if (l->len == tok.len &&
				memcmp(l->start, tok.start, tok.len) == 0) {

				lsp_chunk_add_op(ctx, chunk, op_get, parser->line);
				lsp_chunk_add_op(ctx, chunk, (u8)l->pos, parser->line);

				resolved = true;
				break;
			}
		}

		if (!resolved) {
			parse_error(ctx, parser, "Failed to resolve identifier: `%.*s'.", tok.len, tok.start);
			return false;
		}
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
	parser.cur = str - 1;

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
