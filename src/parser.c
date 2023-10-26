#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "headers/lexer.h"
#include "headers/parser.h"

_Noreturn void tkerr(Token* token, const char *fmt, ...)
{
    fprintf(stderr, "error in line %d, position %d: ", token->line, token->position);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

bool consume(Token** tkit, int expected_code) {
    if ((*tkit)->code == expected_code)
    {
        (*tkit)++;
        return true;
    }
    return false;
}

bool consume_ranged(Token** tkit, int min_code, int max_code) {
    if ((*tkit)->code >= min_code && (*tkit)->code <= max_code)
    {
        (*tkit)++;
        return true;
    }
    return false;
}

bool base_type(Token** tkit) {
    return consume(tkit, TYPE_INT) || consume(tkit, TYPE_REAL) || consume(tkit, TYPE_STR);
}

bool def_var(Token** tkit) {
    Token* start_position = *tkit;

    if (
        consume(tkit, VAR) && 
        consume(tkit, ID) &&
        consume(tkit, COLON) &&
        base_type(tkit) &&
        consume(tkit, SEMICOLON)
    )
        return true;
    
    *tkit = start_position;
    return false;
}

bool inf_consume(Token** tkit, consumer_func func) {
    while (func(tkit));
    return true;
}

bool func_param(Token** tkit) {
    return consume(tkit, ID) && consume(tkit, COLON) && base_type(tkit);
}

bool func_params(Token** tkit) {
    do {
        if (!func_param(tkit))
            return false;
    } while (consume(tkit, COMMA));
    return true;
}

bool complex_factor(Token** tkit) {
    // ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?

    if (!consume(tkit, ID))
        return false;
    
    if (consume(tkit, LPAR)) {
        do {
            expr(tkit);
        } while (consume(tkit, COMMA));
        if (!consume(tkit, RPAR))
            tkerr(*tkit, "expected right parenthesis");
    }

    return true;
}

bool factor(Token** tkit) {
    return (
        consume(tkit, INT) || 
        consume(tkit, REAL) || 
        consume(tkit, STR) || 
        (consume(tkit, LPAR) && expr(tkit) && consume(tkit, RPAR)) ||
        complex_factor(tkit)
    );
}

bool expr_prefix(Token** tkit) {
    consume(tkit, ADD) || consume(tkit, SUB);
    return factor(tkit);
}

bool expr_mul(Token** tkit) {
    do {
        if (!expr_prefix(tkit))
            return false;
    } while (consume(tkit, MUL) || consume(tkit, DIV));
    return true;
}

bool expr_add(Token** tkit) {
    do {
        if (!expr_mul(tkit))
            return false;
    } while (consume(tkit, ADD) || consume(tkit, SUB));
    return true;
}

bool expr_comp(Token** tkit) {
    if (!expr_add(tkit))
        return false;
    if (consume(tkit, LE) || consume(tkit, EQ))
        if (!expr_add(tkit))
            tkerr(*tkit, "expected expression");
    return true;
}

bool expr_assign(Token** tkit) {
    if (consume(tkit, ID))
        if (!consume(tkit, ASSIGN))
            tkerr(*tkit, "expected assignment operator");
    return expr_comp(tkit);
}

bool expr(Token** tkit) {
    do {
        if (!expr_assign(tkit))
            return false;
    } while (consume(tkit, AND) || consume(tkit, OR));
}

bool instr(Token** tkit) {
    expr(tkit);
    return (
        consume(tkit, SEMICOLON) ||
        (consume(tkit, IF) && consume(tkit, LPAR) && expr(tkit) && consume(tkit, RPAR) && block(tkit) && (consume(tkit, ELSE) && block(tkit) || true) && consume(tkit, END)) ||
        (consume(tkit, RETURN) && expr(tkit) && consume(tkit, SEMICOLON)) ||
        (consume(tkit, WHILE) && consume(tkit, LPAR) && expr(tkit) && consume(tkit, RPAR) && block(tkit) && consume(tkit, END))
    );
}

bool block(Token** tkit) {
    Token* start_position = *tkit;

    if (instr(tkit) && inf_consume(tkit, instr))
        return true;

    *tkit = start_position;
    return false;
}

bool def_func(Token** tkit) {
    Token* start_position = *tkit;

    if (
        consume(tkit, FUNCTION) &&
        consume(tkit, ID) &&
        consume(tkit, LPAR) &&
        func_params(tkit) &&
        consume(tkit, RPAR) &&
        consume(tkit, COLON) &&
        base_type(tkit) &&
        inf_consume(tkit, def_var) &&
        block(tkit) &&
        consume(tkit, END)
    )
        return true;

    *tkit = start_position;
    return false;
}

bool program(Token* tkit)
{
    while (true) {
        if (def_var(&tkit) || def_func(&tkit) || block(&tkit))
            continue;
        break;
    }

    if (consume(&tkit, FINISH))
        return true;
    
    tkerr(tkit, "expected end of file");
}

void parse(TokenArray* token_array)
{
    program(token_array->tokens);
}
