#pragma once

#define MAX_STR 127

enum Atom {
	ID,
	
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
	LTE,
	GT,
	GTE
};

typedef struct
{
	int code; // ID, TYPE_INT, ...
	int line; // the line from the input file
	union
	{
		char text[MAX_STR + 1]; // the chars for ID, STR
		int i;					// the value for INT
		double r;				// the value for REAL
	};
} Token;

#define MAX_TOKENS 4096
extern Token tokens[];
extern int nTokens;

void tokenize(const char *pch);
void showTokens();
