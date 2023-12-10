#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "parser.h"
#include "type.h"

_Noreturn void tkerr(Domain* domain, const char *fmt, ...)
{
    Token* token = *(domain->tkit);
    fprintf(stderr, "error in line %d, position %d: ", token->line, token->position);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

Token* consume(Domain* domain, int expected_code, char* err_msg) {
    Token* tk = *(domain->tkit);
    if (tk->code == expected_code) {
        (*domain->tkit)++;
        return tk;
    }
    if (err_msg)
        tkerr(domain, err_msg);
    return NULL;
}

Symbol* get_identifier(Domain* domain, int kind, char* err_msg) {
    Token* idtk = consume(domain, ID, err_msg);
    if (!idtk)
        return NULL;
    const char* name = idtk->text;
    Symbol* s = searchSymbol(domain, name);
    if (s)
        tkerr(domain, "symbol redefinition `%s`", name);
    s = addSymbol(domain, name, kind);
    s->local = domain->parent ? true : false;
    return s;
}

bool base_type(Domain* domain, Symbol* s) {
    Token* tk = NULL;

    (tk = consume(domain, TYPE_INT, NULL)) || (tk = consume(domain, TYPE_REAL, NULL)) || (tk = consume(domain, TYPE_STR, NULL));

    if (tk) {
        s->type = tk->code;
        return true;
    }
    return false;
}

bool def_var(Domain* domain) {
    Token* start_position = *domain->tkit;

    if (consume(domain, VAR, NULL)) {
        Symbol* s = get_identifier(domain, KIND_VAR, "expected identifier after `var` keyword");
        consume(domain, COLON, "expected `:` followed by type after identifier");
        if (!base_type(domain, s))
            tkerr(domain, "expected type after `:`, one of (int, real, str)");
        consume(domain, SEMICOLON, "expected `;` at the of variable declaration");
        return true;
    }
    
    *domain->tkit = start_position;
    return false;
}

bool inf_consume(Domain* domain, consumer_func func) {
    while (func(domain));
    return true;
}

bool func_param(Domain* domain, Symbol* fn) {
    Symbol* arg = get_identifier(domain, KIND_ARG, NULL);
    if (!arg)
        return false;
    consume(domain, COLON, "expected `:` followed by type after identifier");
    if (!base_type(domain, arg)) {
        tkerr(domain, "expected type after `:`, one of (int, real, str)");
    }
    addFnArg(fn, arg->name);
    return true;
}

bool func_params(Domain* domain, Symbol* fn) {
    bool first = true;
    do {
        if (!func_param(domain, fn)) {
            if (!first)
                tkerr(domain, "expected parameter after `,`");
            return false;
        }
        first = false;
    } while (consume(domain, COMMA, NULL));
    return true;
}

bool complex_factor(Domain* domain) {
    if (!consume(domain, ID, NULL))
        return false;
    
    if (consume(domain, LPAR, NULL)) {
        do {
            expr(domain);
        } while (consume(domain, COMMA, NULL));
        consume(domain, RPAR, "expected `)`");
    }
    return true;
}

bool factor(Domain* domain) {
    return (
        consume(domain, INT, NULL) || 
        consume(domain, REAL, NULL) || 
        consume(domain, STR, NULL) || 
        (consume(domain, LPAR, NULL) && expr(domain) && consume(domain, RPAR, "expected `)`")) ||
        complex_factor(domain)
    );
}

bool expr_prefix(Domain* domain) {
    consume(domain, SUB, NULL) || consume(domain, NOT, NULL);
    return factor(domain);
}

bool asterisk_expr(Domain* domain, ExprDomainFunc func, int tk0, int tk1) {
    bool first = true;
    do {
        if (!func(domain)) {
            if (!first)
                tkerr(domain, "expected expression after operator");
            return false;
        }
        first = false;
    } while (consume(domain, tk0, NULL) || consume(domain, tk1, NULL));
    return true;
}

bool expr_add(Domain* domain) {
    return asterisk_expr(domain, expr_prefix, MUL, DIV);
}

bool expr_comp(Domain* domain) {
    if (!asterisk_expr(domain, expr_add, ADD, SUB))
        return false;
    if ((consume(domain, LT, NULL) || consume(domain, GT, NULL) || consume(domain, EQ, NULL) || consume(domain, LE, NULL) || consume(domain, GE, NULL) || consume(domain, NE, NULL)) && !expr_add(domain))
        tkerr(domain, "expected expression after operator `");
    return true;
}

bool expr_assign(Domain* domain) {
    if (consume(domain, ID, NULL) && !consume(domain, ASSIGN, NULL))
        (*domain->tkit)--;
    return expr_comp(domain);
}

bool expr(Domain* domain) {
    // do {
    //     if (!expr_assign(tkit))
    //         return false;
    // } while (consume(tkit, AND, NULL) || consume(tkit, OR, NULL));
    // return true;
    return asterisk_expr(domain, expr_assign, AND, OR);
}

bool instr(Domain* domain) {
    if (expr(domain)) {
        consume(domain, SEMICOLON, "expected `;` after expression");
        return true;
    }

    if (consume(domain, IF, NULL)) {
        consume(domain, LPAR, "expected `(` after `if` keyword");
        if (!expr(domain))
            tkerr(domain, "expected expression after `(`");
        if (domain->symbols->type == TYPE_STR)
            tkerr(domain, "expected expression of type `int` or `real`");
        consume(domain, RPAR, "expected `)` after expression");
        if (!block(domain))
            tkerr(domain, "expected block after `if` statement");
        while (consume(domain, ELIF, NULL)) {
            consume(domain, LPAR, "expected `(` after `elif` keyword");
            if (!expr(domain))
                tkerr(domain, "expected expression after `(`");
            consume(domain, RPAR, "expected `)` after expression");
            if (!block(domain))
                tkerr(domain, "expected block after `elif` statement");
        }
        if (consume(domain, ELSE, NULL)) {
            if (!block(domain))
                tkerr(domain, "expected block after `else` keyword");
        }
        consume(domain, END, "expected `end` keyword");
        return true;
    }

    if (consume(domain, RETURN, NULL)) {
        if (!expr(domain))
            tkerr(domain, "expected expression after `return` keyword");
        if (!domain->parent->symbols || domain->parent->symbols->kind != KIND_FN)
            tkerr(domain, "`return` statement can only be used in a function");
        if (domain->symbols->type != domain->parent->symbols->type)
            tkerr(domain, "the return type must be the same as the function return type");
        consume(domain, SEMICOLON, "expected `;` after return statement");
        return true;
    }

    if (consume(domain, WHILE, NULL)) {
        consume(domain, LPAR, "expected `(` after `while` keyword");
        if (!expr(domain))
            tkerr(domain, "expected expression after `(`");
        if (domain->symbols->type == TYPE_STR)
            tkerr(domain, "expected expression of type `int` or `real`");
        consume(domain, RPAR, "expected `)` after expression");
        if (!block(domain))
            tkerr(domain, "expected block after `while` statement");
        consume(domain, END, "expected `end` keyword");
        return true;
    }
        
    return false;
}

bool block(Domain* domain) {
    Token* start_position = *domain->tkit;

    if (instr(domain) && inf_consume(domain, instr))
        return true;

    *domain->tkit = start_position;
    return false;
}

bool def_func(Domain* parent_domain) {
    Token* start_position = *parent_domain->tkit;

    if (consume(parent_domain, FUNCTION, NULL)) {
        Symbol* fn_symbol = get_identifier(parent_domain, KIND_FN, "expected identifier after `function` keyword");
        fn_symbol->args = NULL;
        Domain* domain = addDomain(parent_domain, parent_domain->tkit);
        consume(domain, LPAR, "expected `(` after function identifier");
        if (!func_params(domain, fn_symbol))
            tkerr(domain, "expected function signature");
        consume(domain, RPAR, "expected `)` after function parameters");
        consume(domain, COLON, "expected `:` followed by return type after function signature");
        if (!base_type(domain, fn_symbol))
            tkerr(domain, "expected function type `int`, `real` or `str`");
        inf_consume(domain, def_var);
        block(domain);
        delDomain(&domain);
        consume(parent_domain, END, "expected `end` keyword after function definition");
        return true;
    }

    *parent_domain->tkit = start_position;
    return false;
}

void parse(TokenArray* token_array)
{
    Token* tkit = token_array->tokens;
    Domain* domain = addDomain(NULL, &tkit);
    addPredefinedFns(domain);
    while (def_var(domain) || def_func(domain) || block(domain));
    consume(domain, FINISH, "unexpected character");
    delDomain(&domain);
}
