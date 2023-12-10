#pragma once

#define MAX_STR 127
#define MAX_TOKENS 4096

enum Atom {
    ID,
    
    VAR,
    FUNCTION,
    IF,
    ELSE,
    ELIF,
    WHILE,
    END,
    RETURN,
    TYPE_INT,
    TYPE_REAL,
    TYPE_STR,

    INT,
    REAL,
    STR,

    COMMA,
    COLON,
    SEMICOLON,
    LPAR,
    RPAR,
    FINISH,

    ADD,
    SUB,
    MUL,
    DIV,
    AND,
    OR,
    NOT,
    ASSIGN,
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE
};

typedef struct
{
    int code;
    int position;
    int line;
    union
    {
        char text[MAX_STR + 1];
        int i;
        double r;
    };
} Token;

typedef struct {
    Token tokens[MAX_TOKENS];
    unsigned int length;
} TokenArray;


void add_token(TokenArray *arr, unsigned int position, unsigned int line, unsigned int code, char *text);
TokenArray tokenize(const char *pch);
char* code_to_str(unsigned int code);
char* op_to_str(unsigned int code);
void print_token(Token *tk);