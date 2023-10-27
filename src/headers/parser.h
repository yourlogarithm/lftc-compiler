#pragma once

#include "lexer.h"

_Noreturn void tkerr(Token* token, const char *fmt, ...);

bool consume(Token** tkit, int expected_code, char* err_msg);

bool consume_ranged(Token** tkit, int min_code, int max_code);

bool base_type(Token** tkit);

bool def_var(Token** tkit);

typedef bool (*consumer_func)(Token** tkit);

bool inf_consume(Token** tkit, consumer_func func);

bool func_param(Token** tkit);

bool func_params(Token** tkit);

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

bool def_func(Token** tkit);

void parse(TokenArray* token_array);
