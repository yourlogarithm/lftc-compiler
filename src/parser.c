#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "parser.h"
#include "type.h"
#include "gen.h"

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
        tkerr(domain, "symbol redefinition: `%s`", name);
    s = addSymbol(domain, name, kind);
    s->local = crtFn != NULL;
    return s;
}

bool base_type(Domain* domain, Symbol* s) {
    Token* tk = NULL;

    (tk = consume(domain, TYPE_INT, NULL)) || (tk = consume(domain, TYPE_REAL, NULL)) || (tk = consume(domain, TYPE_STR, NULL));

    if (tk) {
        ret.type = tk->code;
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
        s->type = ret.type;
        consume(domain, SEMICOLON, "expected `;` at the of variable declaration");
        Text_write(crtVar, "%s %s;\n", cType(ret.type), s->name);
        return true;
    }
    
    *domain->tkit = start_position;
    return false;
}

bool inf_consume(Domain* domain, consumer_func func) {
    while (func(domain));
    return true;
}

bool func_param(Domain* domain) {
    Symbol* arg = get_identifier(domain, KIND_ARG, NULL);
    if (!arg)
        return false;
    Symbol* sFnParam = addFnArg(crtFn, arg->name);
    consume(domain, COLON, "expected `:` followed by type after identifier");
    if (!base_type(domain, arg)) {
        tkerr(domain, "expected type after `:`, one of (int, real, str)");
    }
    arg->type = ret.type;
    sFnParam->type = ret.type;
    Text_write(&tFnHeader,"%s %s", cType(ret.type), sFnParam->name);
    return true;
}

bool func_params(Domain* domain) {
    bool first = true;
    do {
        if (!first) 
            Text_write(&tFnHeader, ",");
        if (!func_param(domain)) {
            if (!first)
                tkerr(domain, "expected parameter after `,`");
            return false;
        }
        first = false;
    } while (consume(domain, COMMA, NULL));
    return true;
}

bool complex_factor(Domain* domain) {
    Token *consumed = consume(domain, ID, NULL);
    if (!consumed)
        return false;

    Text_write(crtCode, "%s", consumed->text);

    Symbol *s = searchSymbol(domain, consumed->text);
    if (!s) 
        tkerr(domain, "undefined symbol: %s",consumed->text);

    if (consume(domain, LPAR, NULL)) {
        if (s->kind != KIND_FN)
            tkerr(domain, "%s cannot be called, because it is not a function", s->name);
        Text_write(crtCode,"(");
        Symbol* argDef = s->args;
        bool first = true;
        do {
            if (!first) {
                Text_write(crtCode, ",");
            }
            first = false;
            if (!expr(domain))
                tkerr(domain, "expected expression after `(`");
            if (!argDef)
                tkerr(domain, "the function %s is called with too many arguments", s->name);
            if (argDef->type != ret.type)
                tkerr(domain, "the argument type at function %s call is different from the one given at its definition",s->name);
            argDef = argDef->next;
        } while (consume(domain, COMMA, NULL));
        consume(domain, RPAR, "expected `)`");
        if (argDef)
            tkerr(domain, "the function %s is called with too few arguments",s->name);
        Text_write(crtCode, ")");
        setRet(s->type, false);
    } else {
        if (s->kind == KIND_FN)
            tkerr(domain, "the function %s can only be called",s->name);
        setRet(s->type, true);
    }
    return true;
}

bool factor(Domain* domain) {
    Token* consumed = NULL;
    if ((consumed = consume(domain, INT, NULL))) {
        setRet(TYPE_INT, false);
        Text_write(crtCode,"%d", consumed->i);
        return true;
    } else if ((consumed = consume(domain, REAL, NULL))) {
        setRet(TYPE_REAL, false);
        Text_write(crtCode,"%f", consumed->r);
        return true;
    } else if ((consumed = consume(domain, STR, NULL))) {
        setRet(TYPE_STR, false);
        Text_write(crtCode,"\"%s\"", consumed->text);
        return true;
    } else if (consume(domain, LPAR, NULL)) {
        Text_write(crtCode, "(");
        if (!expr(domain))
            tkerr(domain, "expected expression after `(`");
        consume(domain, RPAR, "expected `)` after expression");
        Text_write(crtCode, ")");
        return true;
    } else if (complex_factor(domain)) {
        setRet(TYPE_INT, true);
        return true;
    } else {
        return false;
    }
}

bool expr_prefix(Domain* domain) {
    if (consume(domain, SUB, NULL)) {
        if (ret.type == TYPE_STR)
            tkerr(domain, "the expression of unary - must be of type int or real");
        ret.lval = false;
    } else if (consume(domain, NOT, NULL)) {
        if (ret.type == TYPE_STR)
            tkerr(domain, "the expression of ! must be of type int or real");
        setRet(TYPE_INT, false);
    }
    return factor(domain);
}

bool asterisk_expr(Domain* domain, ExprDomainFunc func, int tk0, int tk1) {
    bool first = true;
    Token* consumed = NULL;
    do {
        if (first) {
            if (!func(domain)) {
                if (!first)
                    tkerr(domain, "expected expression after operator");
                return false;
            }
            first = false;
        } else {
            Text_write(crtCode, op_to_str(consumed->code));
            int ltype = ret.type;
            if (ltype == TYPE_STR)
                tkerr(domain, "the operands of `%s` or `%s` cannot be of type str", op_to_str(tk0), op_to_str(tk1));
            if (!func(domain)) {
                if (!first)
                    tkerr(domain, "expected expression after operator");
                return false;
            }
            if (ltype != ret.type)
                tkerr(domain, "different types for the operands of `%s` or `%s`", op_to_str(tk0), op_to_str(tk1));
            ret.lval = false;
        }
    } while ((consumed = consume(domain, tk0, NULL)) || (consumed = consume(domain, tk1, NULL)));
    return true;
}

bool expr_add(Domain* domain) {
    return asterisk_expr(domain, expr_prefix, MUL, DIV);
}

bool expr_comp(Domain* domain) {
    if (!asterisk_expr(domain, expr_add, ADD, SUB))
        return false;
    Token* consumed = NULL;
    if ((
        (consumed = consume(domain, LT, NULL)) || 
        (consumed = consume(domain, GT, NULL)) || 
        (consumed = consume(domain, EQ, NULL)) || 
        (consumed = consume(domain, LE, NULL)) || 
        (consumed = consume(domain, GE, NULL)) || 
        (consumed = consume(domain, NE, NULL)))
    ) {
        Text_write(crtCode, "%s", op_to_str(consumed->code));
        int ltype = ret.type;
        if (!expr_add(domain))
            tkerr(domain, "expected expression after operator");
        if (ltype != ret.type)
            tkerr(domain, "different types for the operands of < or ==");
        setRet(TYPE_INT, false);
    }
    return true;
}

bool expr_assign(Domain* domain) {
    Token* consumed = consume(domain, ID, NULL);
    if (!consumed) {
        return expr_comp(domain);
    }
    const char* name = consumed->text;
    if (!consume(domain, ASSIGN, NULL)) {
        (*domain->tkit)--;
        return expr_comp(domain);
    }
    Text_write(crtCode, "%s=", name);
    if (!expr_comp(domain)) 
        return false;
    Symbol *s = searchSymbol(domain, name);
    if (!s) 
        tkerr(domain, "undefined symbol: %s", name);
    if (s->kind == KIND_FN)
        tkerr(domain, "a function (%s) cannot be used as a destination for assignment", name);
    if (s->type != ret.type)
        tkerr(domain, "the source and destination for assignment must have the same type");
    ret.lval = false;
    return true;
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
        Text_write(crtCode, ";\n");
        return true;
    }

    if (consume(domain, IF, NULL)) {
        consume(domain, LPAR, "expected `(` after `if` keyword");
        Text_write(crtCode, "if(");
        if (!expr(domain))
            tkerr(domain, "expected expression after `(`");
        if (ret.type == TYPE_STR) 
            tkerr(domain, "the if condition must have type int or real");
        consume(domain, RPAR, "expected `)` after expression");
        Text_write(crtCode, "){\n");
        if (!block(domain))
            tkerr(domain, "expected block after `if` statement");
        Text_write(crtCode,"}\n");
        while (consume(domain, ELIF, NULL)) {
            Text_write(crtCode, "else if(");
            consume(domain, LPAR, "expected `(` after `elif` keyword");
            if (!expr(domain))
                tkerr(domain, "expected expression after `(`");
            consume(domain, RPAR, "expected `)` after expression");
            Text_write(crtCode, "){\n");
            if (!block(domain))
                tkerr(domain, "expected block after `elif` statement");
            Text_write(crtCode,"}\n");
        }
        if (consume(domain, ELSE, NULL)) {
            Text_write(crtCode, "else{\n");
            if (!block(domain))
                tkerr(domain, "expected block after `else` keyword");
            Text_write(crtCode, "}\n");
        }
        consume(domain, END, "expected `end` keyword");
        return true;
    }

    if (consume(domain, RETURN, NULL)) {
        Text_write(crtCode, "return ");
        if (!expr(domain))
            tkerr(domain, "expected expression after `return` keyword");
        if (domain->parent->symbols->kind != KIND_FN)
            tkerr(domain, "return can be used only in a function");
        if (!crtFn) 
            tkerr(domain, "return can be used only in a function");
        if (ret.type != crtFn->type)
            tkerr(domain, "the return type must be the same as the function return type");
        if (domain->symbols->type != domain->parent->symbols->type)
            tkerr(domain, "the return type must be the same as the function return type");
        consume(domain, SEMICOLON, "expected `;` after return statement");
        Text_write(crtCode, ";\n");
        return true;
    }

    if (consume(domain, WHILE, NULL)) {
        Text_write(crtCode,"while(");
        consume(domain, LPAR, "expected `(` after `while` keyword");
        if (!expr(domain))
            tkerr(domain, "expected expression after `(`");
        if (ret.type == TYPE_STR)
            tkerr(domain, "expected expression of type `int` or `real`");
        consume(domain, RPAR, "expected `)` after expression");
        Text_write(crtCode,"){\n");
        if (!block(domain))
            tkerr(domain, "expected block after `while` statement");
        consume(domain, END, "expected `end` keyword");
        Text_write(crtCode, "}\n");
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
        crtFn = get_identifier(parent_domain, KIND_FN, "expected identifier after `function` keyword");
        crtFn->args = NULL;
        crtCode = &tFunctions;
        crtVar = &tFunctions;
        Text_clear(&tFnHeader);
        Text_write(&tFnHeader,"%s(", crtFn->name);
        Domain* domain = addDomain(parent_domain, parent_domain->tkit);
        consume(domain, LPAR, "expected `(` after function identifier");
        if (!func_params(domain))
            tkerr(domain, "expected function signature");
        consume(domain, RPAR, "expected `)` after function parameters");
        consume(domain, COLON, "expected `:` followed by return type after function signature");
        if (!base_type(domain, crtFn))
            tkerr(domain, "expected function type `int`, `real` or `str`");
        crtFn->type = ret.type;
        Text_write(&tFunctions, "\n%s %s){\n", cType(ret.type), tFnHeader.buf);
        inf_consume(domain, def_var);
        block(domain);
        delDomain(&domain);
        consume(parent_domain, END, "expected `end` keyword after function definition");
        Text_write(&tFunctions, "}\n");
        crtCode=&tMain;
        crtVar=&tBegin;
        crtFn = NULL;
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
    crtCode = &tMain;
    crtVar = &tBegin;
    Text_write(&tBegin,"#include \"quick.h\"\n\n");
    Text_write(&tMain,"\nint main(){\n");
    while (def_var(domain) || def_func(domain) || block(domain));
    consume(domain, FINISH, "unexpected character");
    delDomain(&domain);
    Text_write(&tMain, "return 0;\n}\n");
    FILE *fis = fopen("examples/result.c", "w");
    if (!fis) {
        printf("cannot write to file 1.c\n");
        exit(EXIT_FAILURE);
    }
    fwrite(tBegin.buf,sizeof(char),tBegin.n,fis);
    fwrite(tFunctions.buf,sizeof(char),tFunctions.n,fis);
    fwrite(tMain.buf,sizeof(char),tMain.n,fis);
    fclose(fis);
}
