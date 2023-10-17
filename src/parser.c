#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.c"

bool def_var(Token* token_iterator);
bool def_func(Token* token_iterator);
bool block(Token* token_iterator);

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

bool consume(int code, Token* token_iterator)
{
    if (token_iterator->code == code)
    {
        token_iterator++;
        return true;
    }
    return false;
}

bool program(Token* token_iterator)
{
    while (true) {
        if (def_var(token_iterator) || def_func(token_iterator) || block(token_iterator))
            continue;
        break;
    }

    if (consume(FINISH, token_iterator))
        return true;
    
    tkerr("syntax error", token_iterator);
}

void parse(TokenArray* token_array)
{
    program(token_array->tokens);
}
