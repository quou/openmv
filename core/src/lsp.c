#include <stdio.h>
#include <string.h>

#include "core.h"
#include "lsp.h"
#include "res.h"

#define chunk_size 1024
#define chunk_max_constants UINT8_MAX
#define stack_size 256

enum {
	op_halt = 0,
	op_push,
	op_push_nil,
	op_push_true,
	op_push_false,
	op_add,
	op_sub,
	op_mul,
	op_div,
	op_print,
	op_set,
	op_get
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

#define arith_op(op_) do { \
		struct lsp_val b = lsp_pop(ctx); \
		struct lsp_val a = lsp_pop(ctx); \
		lsp_push(ctx, lsp_make_num(a.as.num op_ b.as.num)); \
	} while (0)

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
		default: break;
	}
}

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
			case op_push_nil:   lsp_push(ctx, lsp_make_nil()); break;
			case op_push_true:  lsp_push(ctx, lsp_make_bool(true)); break;
			case op_push_false: lsp_push(ctx, lsp_make_bool(false)); break;
			case op_add: arith_op(+); break;
			case op_sub: arith_op(-); break;
			case op_div: arith_op(/); break;
			case op_mul: arith_op(*); break;
			case op_print:
				print_val(ctx->info, lsp_pop(ctx));
				fprintf(ctx->info, "\n");
				break;
			case op_set:
				ctx->ip++;
				ctx->stack[*ctx->ip] = lsp_peek(ctx);
				break;
			case op_get:
				ctx->ip++;
				lsp_push(ctx, ctx->stack[*ctx->ip]);
				break;
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
	tok_iden,
	tok_print,
	tok_set,
	tok_nil,
	tok_true,
	tok_false,
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

	u32 pos;
};

struct parser {
	u32 line;
	const char* cur;
	const char* start;
	struct token token;

	struct local locals[max_locals];
	u32 local_count;
};

static const char* keywords[] = {
	[tok_print] = "print",
	[tok_set]   = "set",
	[tok_nil]   = "nil",
	[tok_true]  = "true",
	[tok_false] = "false"
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
		case '-': return make_token(parser, tok_sub, 1);
		case '*': return make_token(parser, tok_mul, 1);
		case '/': return make_token(parser, tok_div, 1);

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
			fprintf(ctx->error, "~");
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
			fprintf(ctx->error, "~");
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

static bool parse(struct lsp_state* ctx, struct parser* parser, struct lsp_chunk* chunk) {
	struct token tok;

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
		} else if (tok.type == tok_print) {
			parser_recurse();
			lsp_chunk_add_op(ctx, chunk, op_print, parser->line);
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

			parser_recurse();

			if (declare) {
				l.pos = parser->local_count;
			}

			lsp_chunk_add_op(ctx, chunk, op_set, parser->line);
			lsp_chunk_add_op(ctx, chunk, (u8)l.pos, parser->line);

			if (declare) {
				parser->locals[parser->local_count++] = l;
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

	while (parser.token.type != tok_end) {
		if (!parse(ctx, &parser, &chunk)) {
			return lsp_make_nil();
		}
	}

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
