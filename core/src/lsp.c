#include <stdio.h>
#include <string.h>

#include "core.h"
#include "lsp.h"

#define chunk_size 1024
#define chunk_max_constants UINT8_MAX
#define stack_size 256

enum {
	op_halt = 0,
	op_push,
	op_add,
	op_sub,
	op_mul,
	op_div
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

	u8* ip;

	FILE* error;
	FILE* info;
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

struct lsp_state* new_lsp_state(void* error, void* info) {
	if (!error) { error = stderr; }
	if (!info)  { info = stdout; }

	struct lsp_state* state = core_alloc(sizeof(struct lsp_chunk));

	*state = (struct lsp_state) {
		.error = error, .info = info,
		.stack_top = state->stack
	};

	return state;
}

void free_lsp_state(struct lsp_state* state) {
	core_free(state);
}

void lsp_push(struct lsp_state* ctx, struct lsp_val val) {
	*(++ctx->stack_top) = val;
}

struct lsp_val lsp_pop(struct lsp_state* ctx) {
	return *(ctx->stack_top--);
}

#define arith_op(op_) do { \
		struct lsp_val b = lsp_pop(ctx); \
		struct lsp_val a = lsp_pop(ctx); \
		lsp_push(ctx, lsp_make_num(a.as.num op_ b.as.num)); \
	} while (0)

static struct lsp_val lsp_eval(struct lsp_state* ctx, struct lsp_chunk* chunk) {
	ctx->ip = chunk->code;

	while (1) {
		switch (*ctx->ip) {
			case op_halt:
				goto eval_halt;
			case op_push:
				ctx->ip++;
				lsp_push(ctx, chunk->consts[*ctx->ip]);
				break;
			case op_add: arith_op(+); break;
			case op_sub: arith_op(-); break;
			case op_div: arith_op(/); break;
			case op_mul: arith_op(*); break;
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
	tok_number,
	tok_end,
	tok_error
};

struct token {
	u32 type;
	u32 line;
	const char* start;
	u32 len;
};

struct parser {
	u32 line;
	const char* cur;
	const char* start;
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

static struct token next_tok(struct parser* parser) {
	parser->cur++;

	skip_whitespace(parser);

	if (!*parser->cur) { return (struct token) { .type = tok_end }; }

	switch (*parser->cur) {
		case '(': return make_token(parser, tok_left_paren, 1);
		case ')': return make_token(parser, tok_right_paren, 1);
		case '+': return make_token(parser, tok_add, 1);

		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		case '8': case '9': {
			/* Length is not required, number is parsed later
			 * using strtod. */
			struct token t = make_token(parser, tok_number, 0);

			while (is_digit(*parser->cur)) {
				parser->cur++;
			}

			if (*parser->cur == '.') {
				while (is_digit(*parser->cur)) {
					parser->cur++;
				}
			}

			parser->cur--;

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

static void parse_error(struct lsp_state* ctx, struct parser* parser, const char* message) {
	u32 col = ((u32)(parser->cur - parser->start) - parser->line) + 1;

	u32 line_len = 0;
	const char* c = parser->cur - col;
	while (*c && *c != '\n') {
		line_len++;
		c++;
	}

	fprintf(ctx->error, "\033[1;31merror \033[0m");
	fprintf(ctx->error, "[line %d:%d]: %s\n", parser->line, col + 1, message);
	fprintf(ctx->error, "%10d| %.*s\n", parser->line, line_len, parser->cur - col);
	fprintf(ctx->error, "            ");
	fprintf(ctx->error, "\033[1;35m");
	for (u32 i = 0; i < col; i++) {
		fprintf(ctx->error, "~");
	}
	fprintf(ctx->error, "^\033[0m\n");
}

static bool parse(struct lsp_state* ctx, struct parser* parser, struct lsp_chunk* chunk) {
	struct token tok;

	advance();

	if (tok.type == tok_left_paren) {
		advance();

		if (tok.type == tok_add) {
			parser_recurse();
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_add, parser->line);
		}
		
		advance();
		expect_tok(tok_right_paren, "Expected `)'.");
	} else if (tok.type == tok_number) {
		double n = strtod(tok.start, null);
		u8 a = lsp_chunk_add_const(ctx, chunk, lsp_make_num(n));
		lsp_chunk_add_op(ctx, chunk, op_push, parser->line);
		lsp_chunk_add_op(ctx, chunk, a, parser->line);
	} else {
		parse_error(ctx, parser, "Unexpected token.");
		return false;
	}

	return true;
}

struct lsp_val lsp_do_string(struct lsp_state* ctx, const char* str) {
	struct lsp_chunk chunk = { 0 };

	struct parser parser = { 0 };
	parser.line = 1;
	parser.start = str;
	parser.cur = str - 1;

	if (parse(ctx, &parser, &chunk)) {
		lsp_chunk_add_op(ctx, &chunk, op_halt, parser.line);
		return lsp_eval(ctx, &chunk);
	}

	return lsp_make_nil();
}
