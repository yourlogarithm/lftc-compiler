#pragma once

#include <stdbool.h>

typedef struct
{
    int type;  // TYPE_*
    bool lval; // if it is a left-value (required for types analysis)
} Ret;

enum
{
    KIND_VAR,
    KIND_ARG,
    KIND_FN
};

struct Symbol;
typedef struct Symbol Symbol;
struct Symbol 
{
    const char *name; // reference to a name stored in a token
    int kind;          // KIND_*
    int type;          // TYPE_* from tokens
    union
    {
        Symbol *args; // for functions: the list with the function args
        bool local;      // for vars: if it is local
    };
    Symbol *next; // link to the next Symbol in list
};

struct Domain;
typedef struct Domain Domain;
struct Domain
{
    Domain *parent;     // the parent of this domain or NULL for the global domain
    Symbol *symbols; // simple linked list of symbols
    Token **tkit;
};

Domain *addDomain(Domain* domain, Token** tkit);                               // adds a new domain to ST as the current domain
Domain *delDomain(Domain** domain);                                   // deletes the current domain from ST and returns the the last one
Symbol *searchInCurrentDomain(Domain* domain, const char *name);   // searches a symbol by name only in the current domain
Symbol *searchSymbol(Domain* domain, const char *name);               // searches in all domains
Symbol *addSymbol(Domain* domain, const char *name, int kind);       // adds a symbol to the current domain
Symbol *addFnArg(Symbol *fn, const char *argName); // adds an argument to the symbol fn
