#pragma once

#include "common.h"

struct lsp_state;

enum {
	lsp_val_nil = 0,
	lsp_val_num,
	lsp_val_bool
};

struct lsp_val {
	u8 type;
	union {
		float num;
		bool boolean;
	} as;
};

#define lsp_make_nil()           ((struct lsp_val) { .type = lsp_val_nil })
#define lsp_make_num(n_)         ((struct lsp_val) { .type = lsp_val_num, .as.num = n_ })
#define lsp_make_bool(b_)        ((struct lsp_val) { .type = lsp_val_num, .as.boolean = b_ })

struct lsp_state* new_lsp_state(void* error, void* info);
void free_lsp_state(struct lsp_state* state);

void lsp_set_simple_errors(struct lsp_state* state, bool simple);

void           lsp_push(struct lsp_state* ctx, struct lsp_val val);
struct lsp_val lsp_pop(struct lsp_state* ctx);

struct lsp_val lsp_do_string(struct lsp_state* ctx, const char* str);
