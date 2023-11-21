#pragma once

#include "lexer.h"
#include "da.h"

typedef bool (*ExprDomainFunc)(Domain*);

_Noreturn void tkerr(Domain* domain, const char *fmt, ...);

Token* consume(Domain* domain, int expected_code, char* err_msg);

bool consume_ranged(Domain* domain, int min_code, int max_code);

bool base_type(Domain* domain, Symbol* s);

bool def_var(Domain* domain);

typedef bool (*consumer_func)(Domain* domain);

bool inf_consume(Domain* domain, consumer_func func);

bool func_param(Domain* domain, Symbol* s);

bool func_params(Domain* domain, Symbol* s);

bool complex_factor(Domain* domain);

bool factor(Domain* domain);

bool expr_prefix(Domain* domain);

bool asterisk_expr(Domain* domain, ExprDomainFunc func, int tk0, int tk1);

bool expr_add(Domain* domain);

bool expr_comp(Domain* domain);

bool expr_assign(Domain* domain);

bool expr(Domain* domain);

bool instr(Domain* domain);

bool block(Domain* domain);

bool def_func(Domain* domain);

void parse(TokenArray* token_array);
