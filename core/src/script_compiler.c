#include <stdio.h>
#include <string.h>

#include "core.h"
#include "script_engine.h"

enum {
	tt_fn = 0,
	tt_ret,
	tt_number,
	tt_plus,
	tt_dash,
	tt_star,
	tt_slash,
	tt_percent,
	tt_left_brace,
	tt_right_brace,
	tt_left_paren,
	tt_right_paren,
	tt_semi,
	tt_identifier,
	tt_print,
	tt_error,
	tt_eof
};

struct token {
	u32 type;

	const char* start;
	u32 length;
	u32 line;
};

struct lexer {
	const char* start;
	const char* current;

	u32 line;
};

static void init_lexer(struct lexer* lexer, const char* source) {
	lexer->start = source;
	lexer->current = source;
	lexer->line = 1;
}

static bool lexer_is_at_end(struct lexer* lexer) {
	return *lexer->current == '\0';
}

static bool lexer_is_digit(char c) {
	return c >= '0' && c <= '9';
}

static bool lexer_is_alpha(char c) {
	return 	(c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c == '_');
}

static char lexer_advance(struct lexer* lexer) {
	return *lexer->current++;
}

static char lexer_peek(struct lexer* lexer) {
	return *lexer->current;
}

static char lexer_peek_next(struct lexer* lexer) {
	if (lexer_is_at_end(lexer)) { return '\0'; }
	return lexer->current[1];
}

static struct token make_token(struct lexer* lexer, u32 type) {
	return (struct token) {
		.type = type,
		.start = lexer->start,
		.length = (u32)(lexer->current - lexer->start),
		.line = lexer->line
	};
}

static struct token make_error_token(struct lexer* lexer, const char* message) {
	return (struct token) {
		.type = tt_error,
		.start = message,
		.length = (u32)strlen(message),
		.line = lexer->line
	};
}

static struct token make_number_token(struct lexer* lexer) {
	while (lexer_is_digit(lexer_peek(lexer))) { lexer_advance(lexer); }

	if (lexer_peek(lexer) == '.') {
		lexer_advance(lexer);

		while (lexer_is_digit(lexer_peek(lexer))) { lexer_advance(lexer); }
	}

	return make_token(lexer, tt_number);
}

static u32 check_keyword(struct lexer* lexer, u32 start, u32 len, const char* rest, u32 type) {
	if (lexer->current - lexer->start == start + len &&
		memcmp(lexer->start + start, rest, len) == 0) {
		return type;
	}

	return tt_identifier;
}

static u32 get_identifier_type(struct lexer* lexer) {
	switch (*lexer->start) {
		case 'f': return check_keyword(lexer, 1, 1, "n", tt_fn);
		case 'r': return check_keyword(lexer, 1, 2, "et", tt_ret);
		case 'p': return check_keyword(lexer, 1, 4, "rint", tt_print);
	}

	return tt_identifier;
}

static struct token make_identifier_token(struct lexer* lexer) {
	while (lexer_is_alpha(lexer_peek(lexer)) || lexer_is_digit(lexer_peek(lexer))) {
		lexer_advance(lexer);
	}

	return make_token(lexer, get_identifier_type(lexer));
}

static void skip_whitespace(struct lexer* lexer) {
	for (;;) {
		char c = lexer_peek(lexer);
		switch (c) {
			case ' ':
			case '\r':
			case '\t':
				lexer_advance(lexer);
				break;
			case '\n':
				lexer->line++;
				lexer_advance(lexer);
				break;
			case '/':
				if (lexer_peek_next(lexer) == '/') {
					while (lexer_peek(lexer) != '\n' && !lexer_is_at_end(lexer)) {
						lexer_advance(lexer);
					}
				} else {
					return;
				}
				break;
			default: return;
		}
	}
}

static struct token next_token(struct lexer* lexer) {
	skip_whitespace(lexer);

	lexer->start = lexer->current;

	if (lexer_is_at_end(lexer)) { return make_token(lexer, tt_eof); }

	char c = lexer_advance(lexer);

	if (lexer_is_alpha(c)) {
		return make_identifier_token(lexer);
	}

	if (lexer_is_digit(c)) {
		return make_number_token(lexer);
	}

	switch (c) {
		case '(': return make_token(lexer, tt_left_paren);
		case ')': return make_token(lexer, tt_right_paren);
		case '{': return make_token(lexer, tt_left_brace);
		case '}': return make_token(lexer, tt_right_brace);
		case '+': return make_token(lexer, tt_plus);
		case '-': return make_token(lexer, tt_dash);
		case '*': return make_token(lexer, tt_star);
		case '/': return make_token(lexer, tt_slash);
		case ';': return make_token(lexer, tt_semi);
		case '%': return make_token(lexer, tt_percent);
		default: break;
	}

	return make_error_token(lexer, "Unexpected character.");
}

struct compiler {
	struct token token;
	struct token previous;

	struct script_chunk* chunk;

	struct lexer lexer;

	struct script_engine* engine;

	bool had_error;
};

static void compile_error(struct compiler* compiler, const char* fmt, ...) {
	compiler->had_error = true;

	fprintf(stderr, "Compile Error (line %u): ", compiler->token.line);

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fprintf(stderr, "\n");
}

static void compiler_advance(struct compiler* compiler) {
	compiler->previous = compiler->token;
	compiler->token = next_token(&compiler->lexer);
}

static void compiler_expect(struct compiler* compiler, const char* name, u32 tt) {
	compiler_advance(compiler);
	if (compiler->token.type != tt) {
		compile_error(compiler, "Expected %s, but got `%.*s'.",
			name, compiler->token.length, compiler->token.start);
	}
}

static void compiler_consume(struct compiler* compiler, u32 tt, const char* error) {
	if (compiler->token.type == tt) {
		compiler_advance(compiler);
		return;
	}
	compile_error(compiler, error);
}

static void compile_expression(struct compiler* compiler);

enum {
	prec_none = 0,
	prec_assignment,
	prec_term,
	prec_factor
};

typedef void (*parse_func)(struct compiler* compiler);

struct parse_rule {
	parse_func prefix;
	parse_func infix;
	u32 prec;
};

static struct parse_rule* get_parse_rule(struct compiler* compiler, u32 type);
static void parse_precedance(struct compiler* compiler, u32 prec);

static void number_parser(struct compiler* compiler) {
	double value = strtod(compiler->previous.start, null);
	chunk_add_instruction(compiler->chunk, op_push);
	chunk_add_address(compiler->chunk, new_constant(compiler->engine, script_number_value(value)));
}

static void grouping_parser(struct compiler* compiler) {
	compile_expression(compiler);
	compiler_consume(compiler, tt_right_paren, "Expected `)' after expression.");
}

static void binary_parser(struct compiler* compiler) {
	u32 op_type = compiler->previous.type;
	struct parse_rule* rule = get_parse_rule(compiler, op_type);
	parse_precedance(compiler, rule->prec + 1);

	switch (op_type) {
		case tt_plus:		chunk_add_instruction(compiler->chunk, op_add); break;
		case tt_dash:		chunk_add_instruction(compiler->chunk, op_sub); break;
		case tt_star:		chunk_add_instruction(compiler->chunk, op_mul); break;
		case tt_slash:		chunk_add_instruction(compiler->chunk, op_div); break;
		case tt_percent:	chunk_add_instruction(compiler->chunk, op_mod); break;
		default: return;
	}
}

struct parse_rule parse_rules[] = {
	[tt_fn]				= { null,				null,				prec_none },
	[tt_ret]			= { null,				null,				prec_none },
	[tt_number]			= { number_parser,		null,				prec_none },
	[tt_plus]			= { null,				binary_parser,		prec_term },
	[tt_dash]			= { null,				binary_parser,		prec_term },
	[tt_star]			= { null,				binary_parser,		prec_factor },
	[tt_slash]			= { null,				binary_parser,		prec_factor },
	[tt_percent]		= { null,				binary_parser,		prec_factor },
	[tt_left_brace]		= { null,				null,				prec_none },
	[tt_right_brace]	= { null,				null,				prec_none },
	[tt_left_paren]		= { grouping_parser,	null,				prec_none },
	[tt_right_paren]	= { null,				null,				prec_none },
	[tt_semi]			= { null,				null,				prec_none },
	[tt_identifier]		= { null,				null,				prec_none },
	[tt_print]			= { null,				null,				prec_none },
	[tt_error]			= { null,				null,				prec_none },
	[tt_eof]			= { null,				null,				prec_none }
};

static struct parse_rule* get_parse_rule(struct compiler* compiler, u32 type) {
	return parse_rules + type;
}

static void parse_precedance(struct compiler* compiler, u32 prec) {
	compiler_advance(compiler);
	parse_func prefix_rule = get_parse_rule(compiler, compiler->previous.type)->prefix;
	if (!prefix_rule) {
		compile_error(compiler, "Expected an expression.");
		return;
	}

	prefix_rule(compiler);

	while (prec <= get_parse_rule(compiler, compiler->token.type)->prec) {
		compiler_advance(compiler);
		parse_func infix_rule = get_parse_rule(compiler, compiler->previous.type)->infix;
		infix_rule(compiler);
	}
}

static void compile_expression(struct compiler* compiler) {
	parse_precedance(compiler, prec_assignment);
}

static bool compiler_match_token(struct compiler* compiler, u32 tt) {
	if (compiler->token.type != tt) {
		return false;
	}

	compiler_advance(compiler);

	return true;
}

static void compile_statement(struct compiler* compiler) {
	if (compiler_match_token(compiler, tt_print)) {
		compile_expression(compiler);
		compiler_consume(compiler, tt_semi, "Expected `;' after expression.");
		chunk_add_instruction(compiler->chunk, op_print);
	} else {
		compile_expression(compiler);
		compiler_consume(compiler, tt_semi, "Expected `;' after expression.");
		chunk_add_instruction(compiler->chunk, op_pop);
	}
}

static void compile_declaration(struct compiler* compiler) {
	compile_statement(compiler);
}

static void compile_function(struct compiler* compiler) {
	if (compiler->chunk) {
		compile_error(compiler, "Nested functions are not supported.");
		return;
	}

	compiler_expect(compiler, "identifier", tt_identifier);

	struct token fn_name = compiler->token;

	compiler_expect(compiler, "{", tt_left_brace);

	u64 fn_addr = new_chunk(compiler->engine);
	compiler->chunk = compiler->engine->chunks + fn_addr;

	compiler_advance(compiler);

	/* Compile the function */
	while (compiler->token.type != tt_right_brace) {
		compile_declaration(compiler);

		if (compiler->token.type == tt_eof) {
			compile_error(compiler, "Expected `}' after function.");
			return;
		}
	}

	chunk_add_instruction(compiler->chunk, op_halt);

	script_value_table_set(&compiler->engine->globals, elf_hash((const u8*)fn_name.start, fn_name.length), script_function_value(fn_addr));

	compiler->chunk = null;
}

void compile_script(struct script_engine* engine, const char* source) {
	struct compiler compiler = { 0 };
	compiler.engine = engine;

	init_lexer(&compiler.lexer, source);

	compiler_advance(&compiler);

	while (compiler.token.type != tt_eof) {
		switch (compiler.token.type) {
			case tt_fn:
				compile_function(&compiler);
				break;
			default: break;
		}
		
		if (compiler.had_error) {
			return;
		}

		compiler_advance(&compiler);
	}
}
