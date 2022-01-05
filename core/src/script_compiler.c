#include <stdio.h>
#include <string.h>

#include "core.h"
#include "script_engine.h"

enum {
	tt_num = 0,
	tt_ret,
	tt_number,
	tt_plus,
	tt_dash,
	tt_star,
	tt_slash,
	tt_left_brace,
	tt_right_brace,
	tt_left_paren,
	tt_right_paren,
	tt_semi,
	tt_identifier,
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
		case 'n': return check_keyword(lexer, 1, 2, "um", tt_num);
		case 'r': return check_keyword(lexer, 1, 2, "et", tt_ret);
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

struct token next_token(struct lexer* lexer) {
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
		default: break;
	}

	return make_error_token(lexer, "Unexpected character.");
}

void compile_script(struct script_engine* engine, const char* source) {
	struct lexer lexer;
	init_lexer(&lexer, source);

	struct token token;
	while ((token = next_token(&lexer)).type != tt_eof) {
		printf("%d\t%.*s\n", token.type, token.length, token.start);
	}
}
