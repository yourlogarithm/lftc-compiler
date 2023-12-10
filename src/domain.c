#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "domain.h"
#include "utils.h"

Domain *addDomain(Domain* domain, Token** tkit)
{
    puts("creates a new domain");
    Domain *d = (Domain *) safeAlloc(sizeof(Domain));
    d->parent = domain;
    d->symbols = NULL;
    d->tkit = tkit;
    return d;
}

void delSymbols(Symbol *list);

void delSymbol(Symbol *s)
{
    printf("\tdeletes the symbol %s\n", s->name);
    if (s->kind == KIND_FN)
        delSymbols(s->args);
    free(s);
}

void delSymbols(Symbol *list)
{
    for (Symbol *s1 = list, *s2; s1; s1 = s2)
    {
        s2 = s1->next;
        delSymbol(s1);
    }
}

Domain* delDomain(Domain** domain)
{
    puts("deletes the current domain");
    Domain *parent = (*domain)->parent;
    delSymbols((*domain)->symbols);
    free(*domain);
    puts("returns to the parent domain");
    return parent;
}

Symbol *searchInList(Symbol *list, const char *name)
{
    for (Symbol *s = list; s; s = s->next)
        if (!strcmp(s->name, name))
            return s;
    return NULL;
}

Symbol *searchInCurrentDomain(Domain *domain, const char *name)
{
    return searchInList(domain->symbols, name);
}

Symbol *searchSymbol(Domain *domain, const char *name)
{
    for (Domain *d = domain; d; d = d->parent)
    {
        Symbol *s = searchInList(d->symbols, name);
        if (s) 
			return s;
    }
    return NULL;
}

Symbol *createSymbol(const char *name, int kind)
{
    Symbol *s = (Symbol *)safeAlloc(sizeof(Symbol));
    s->name = name;
    s->kind = kind;
    return s;
}

Symbol *addSymbol(Domain *domain, const char *name, int kind)
{
    printf("\tadds symbol %s\n", name);
    Symbol *s = createSymbol(name, kind);
    s->next = domain->symbols;
    domain->symbols = s;
    return s;
}

Symbol *addFnArg(Symbol *fn, const char *argName)
{
    printf("\tadds symbol %s as argument\n", argName);
    Symbol *s = createSymbol(argName, KIND_ARG);
    s->next = NULL;
    if (fn->args)
    {
        Symbol *p = fn->args;
		while (p->next) 
			p = p->next;
        p->next = s;
    }
    else
        fn->args = s;
    return s;
}
