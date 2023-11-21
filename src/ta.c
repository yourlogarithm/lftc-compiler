#include <stddef.h>

#include "headers/lexer.h"
#include "headers/ta.h"

// adds in ST a function with an argument
// the argument has the type argType and the function returns the type retType
Symbol *addFn1Arg(Domain* domain, const char *fnName, int argType, int retType)
{
    Symbol *fn = addSymbol(domain, fnName, KIND_FN);
    fn->type = retType;
    fn->args = NULL;
    Symbol *arg = addFnArg(fn, "arg");
    arg->type = argType;
    return fn;
}

void addPredefinedFns(Domain* domain)
{
    addFn1Arg(domain, "puti", TYPE_INT, TYPE_INT);
    addFn1Arg(domain, "putr", TYPE_REAL, TYPE_REAL);
    addFn1Arg(domain, "puts", TYPE_STR, TYPE_STR);
}

void setRet(Ret* ret, int type, bool lval)
{
    ret->type = type;
    ret->lval = lval;
}
