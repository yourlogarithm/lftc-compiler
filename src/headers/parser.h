#pragma once

#include "lexer.h"
#include "da.h"

_Noreturn void tkerr(Token* token, const char *fmt, ...);

Token* consume(Token** tkit, int expected_code, char* err_msg);

bool consume_ranged(Token** tkit, int min_code, int max_code);

bool base_type(Token** tkit, Symbol* s);

bool def_var(Token** tkit, Domain* domain);

typedef bool (*consumer_func)(Token** tkit);

bool inf_consume(Token** tkit, consumer_func func);

bool func_param(Token** tkit, Domain* domain, Symbol* s);

bool func_params(Token** tkit, Domain* domain, Symbol* s);

bool complex_factor(Token** tkit);

bool factor(Token** tkit);

bool expr_prefix(Token** tkit);

bool expr_mul(Token** tkit);

bool expr_add(Token** tkit);

bool expr_comp(Token** tkit);

bool expr_assign(Token** tkit);

bool expr(Token** tkit);

bool instr(Token** tkit);

bool block(Token** tkit);

bool def_func(Token** tkit, Domain* domain);

void parse(TokenArray* token_array);
