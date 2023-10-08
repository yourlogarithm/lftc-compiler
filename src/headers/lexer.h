#pragma once

#define MAX_STR 127
#define MAX_TOKENS 4096

enum Atom {
	ID,
	
	VAR,
	FUNCTION,
	IF,
	ELSE,
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
	int code; // ID, TYPE_INT, ...
	int position; // the position from the input file
	int line; // the line from the input file
	union
	{
		char text[MAX_STR + 1]; // the chars for ID, STR
		int i;					// the value for INT
		double r;				// the value for REAL
	};
} Token;

typedef struct {
	Token tokens[MAX_TOKENS];
	unsigned int length;
} TokenArray;


void addToken(TokenArray *arr, unsigned int position, unsigned int line, unsigned int code, char *text);
TokenArray tokenize(const char *pch);
