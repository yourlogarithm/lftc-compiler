#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "headers/lexer.h"
#include "headers/parser.h"
#include "da.c"

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

Token* consume(Token** tkit, int expected_code, char* err_msg) {
    if ((*tkit)->code == expected_code) {
        (*tkit)++;
        return (*tkit) - 1;
    }
    if (err_msg)
        tkerr(*tkit, err_msg);
    return NULL;
}

Symbol* get_identifier(Token** tkit, Domain* domain, int kind, char* err_msg) {
    Token* idtk = consume(tkit, ID, err_msg);
    if (!idtk)
        return NULL;
    const char* name = idtk->text;
    Symbol* s = searchSymbol(domain, name);
    if (s)
        tkerr(*tkit, "symbol redefinition `%s`", name);
    s = addSymbol(domain, name, kind);
    s->local = domain->parent ? true : false;
    return s;
}

bool base_type(Token** tkit, Symbol* s) {
    Token* tk = NULL;
    (tk = consume(tkit, TYPE_INT, NULL)) || (tk = consume(tkit, TYPE_REAL, NULL)) || (tk = consume(tkit, TYPE_STR, NULL));
    if (tk) {
        s->type = tk->code;
        return true;
    }
    return false;
}

bool def_var(Token** tkit, Domain* domain) {
    Token* start_position = *tkit;

    if (consume(tkit, VAR, NULL)) {
        Symbol* s = get_identifier(tkit, domain, KIND_VAR, "expected identifier after `var` keyword");
        consume(tkit, COLON, "expected `:` after identifier");
        if (!base_type(tkit, s))
            tkerr(*tkit, "expected type after `:`, one of (int, real, str)");
        consume(tkit, SEMICOLON, "expected `;` at the of variable declaration");
        return true;
    }
    
    *tkit = start_position;
    return false;
}

bool inf_consume(Token** tkit, consumer_func func) {
    while (func(tkit));
    return true;
}

bool func_param(Token** tkit, Domain* domain, Symbol* fn) {
    Symbol* arg = get_identifier(tkit, domain, KIND_ARG, NULL);
    if (!arg)
        return false;
    if (!consume(tkit, COLON, NULL))
        return false; 
    if (!base_type(tkit, arg))
        return false;
    addFnArg(fn, arg->name);
    return true;
}

bool func_params(Token** tkit, Domain* domain, Symbol* fn) {
    do {
        if (!func_param(tkit, domain, fn))
            return false;
    } while (consume(tkit, COMMA, NULL));
    return true;
}

bool complex_factor(Token** tkit) {
    if (!consume(tkit, ID, NULL))
        return false;
    
    if (consume(tkit, LPAR, NULL)) {
        do {
            expr(tkit);
        } while (consume(tkit, COMMA, NULL));
        consume(tkit, RPAR, "expected `)`");
    }
    return true;
}

bool factor(Token** tkit) {
    return (
        consume(tkit, INT, NULL) || 
        consume(tkit, REAL, NULL) || 
        consume(tkit, STR, NULL) || 
        (consume(tkit, LPAR, NULL) && expr(tkit) && consume(tkit, RPAR, "expected `)`")) ||
        complex_factor(tkit)
    );
}

bool expr_prefix(Token** tkit) {
    consume(tkit, SUB, NULL) || consume(tkit, NOT, NULL);
    return factor(tkit);
}

bool expr_mul(Token** tkit) {
    do {
        if (!expr_prefix(tkit))
            return false;
    } while (consume(tkit, MUL, NULL) || consume(tkit, DIV, NULL));
    return true;
}

bool expr_add(Token** tkit) {
    do {
        if (!expr_mul(tkit))
            return false;
    } while (consume(tkit, ADD, NULL) || consume(tkit, SUB, NULL));
    return true;
}

bool expr_comp(Token** tkit) {
    if (!expr_add(tkit))
        return false;
    if ((consume(tkit, LT, NULL) || consume(tkit, GT, NULL) || consume(tkit, EQ, NULL) || consume(tkit, LE, NULL) || consume(tkit, GE, NULL) || consume(tkit, NE, NULL)) && !expr_add(tkit))
        tkerr(*tkit, "expected expression after operator");
    return true;
}

bool expr_assign(Token** tkit) {
    if (consume(tkit, ID, NULL) && !consume(tkit, ASSIGN, NULL))
        (*tkit)--;
    return expr_comp(tkit);
}

bool expr(Token** tkit) {
    do {
        if (!expr_assign(tkit))
            return false;
    } while (consume(tkit, AND, NULL) || consume(tkit, OR, NULL));
    return true;
}

bool instr(Token** tkit) {
    expr(tkit);

    if (consume(tkit, SEMICOLON, NULL)) 
        return true;

    if (consume(tkit, IF, NULL)) {
        consume(tkit, LPAR, "expected `(` after `if` keyword");
        if (!expr(tkit))
            tkerr(*tkit, "expected expression after `(`");
        consume(tkit, RPAR, "expected `)` after expression");
        if (!block(tkit))
            tkerr(*tkit, "expected block after `if` statement");
        while (consume(tkit, ELIF, NULL)) {
            consume(tkit, LPAR, "expected `(` after `elif` keyword");
            if (!expr(tkit))
                tkerr(*tkit, "expected expression after `(`");
            consume(tkit, RPAR, "expected `)` after expression");
            if (!block(tkit))
                tkerr(*tkit, "expected block after `elif` statement");
        }
        if (consume(tkit, ELSE, NULL)) {
            if (!block(tkit))
                tkerr(*tkit, "expected block after `else` keyword");
        }
        consume(tkit, END, "expected `end` keyword");
        return true;
    }

    if (consume(tkit, RETURN, NULL)) {
        if (!expr(tkit))
            tkerr(*tkit, "expected expression after `return` keyword");
        consume(tkit, SEMICOLON, "expected `;` after return statement");
        return true;
    }

    if (consume(tkit, WHILE, NULL)) {
        consume(tkit, LPAR, "expected `(` after `while` keyword");
        if (!expr(tkit))
            tkerr(*tkit, "expected expression after `(`");
        consume(tkit, RPAR, "expected `)` after expression");
        if (!block(tkit))
            tkerr(*tkit, "expected block after `while` statement");
        consume(tkit, END, "expected `end` keyword");
        return true;
    }
        
    return false;
}

bool block(Token** tkit) {
    Token* start_position = *tkit;

    if (instr(tkit) && inf_consume(tkit, instr))
        return true;

    *tkit = start_position;
    return false;
}

bool def_func(Token** tkit, Domain* domain) {
    Token* start_position = *tkit;

    if (consume(tkit, FUNCTION, NULL)) {
        Symbol* fn_symbol = get_identifier(tkit, domain, KIND_FN, "expected identifier after `function` keyword");
        fn_symbol->args = NULL;
        domain = addDomain(domain);
        consume(tkit, LPAR, "expected `(` after function identifier");
        if (!func_params(tkit, domain, fn_symbol))
            tkerr(*tkit, "expected function signature");
        consume(tkit, RPAR, "expected `)` after function parameters");
        consume(tkit, COLON, "expected `:` followed by return type after function signature");
        if (!base_type(tkit, fn_symbol))
            tkerr(*tkit, "expected type `int`, `real` or `str`");
        inf_consume(tkit, def_var);
        block(tkit);
        delDomain(&domain);
        consume(tkit, END, "expected `end` keyword after function definition");
        return true;
    }

    *tkit = start_position;
    return false;
}

void parse(TokenArray* token_array)
{
    Token* tkit = token_array->tokens;

    Domain* domain = addDomain(NULL);
    while (def_var(&tkit, domain) || def_func(&tkit, domain) || block(&tkit));
    delDomain(&domain);
    consume(&tkit, FINISH, "unexpected character");
}
