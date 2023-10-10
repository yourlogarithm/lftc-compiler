#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "headers/lexer.h"
#include "headers/utils.h"
#include "djb2.c"

void addToken(TokenArray *arr, unsigned int position, unsigned int line, unsigned int code, char *text)
{
	if (arr->length == MAX_TOKENS) {
		err("too many tokens");
	}
	Token *tk = &arr->tokens[arr->length++];
	tk->code = code;
	tk->position = position;
	tk->line = line;
	if (code == ID || code == STR) {
		strcpy(tk->text, text);
	} else if (code == INT) {
		tk->i = atoi(text);
	} else if (code == REAL) {
		tk->r = atof(text);
	}
}

// copy in the dst buffer the string between [begin,end)
void copyn(char *dst, const char *begin, const char *end)
{
	char *p = dst;
	if (end - begin > MAX_STR)
		err("string too long");
	while (begin != end)
		*p++ = *begin++;
	*p = '\0';
}

void accumulate(
	char* text, 
	char** pch, 
	unsigned int* position, 
	bool (*must_continue)(char**),
	bool copy
) {
	char* start = *pch;
	while (must_continue(*pch)) {
		(*pch)++;
		(*position)++;
	}
	(*position)--;
	if (copy)
		copyn(text, start, *pch);
}

bool word_accumulator(char* pch) {
	return isalpha(*pch) || *pch == '_';
}

bool newline_accumulator(char* pch) {
	return *pch != '\n';
}

bool string_literal_accumulator(char* pch) {
	return *pch != '"';
}

void consume_or_raise(char* pch, char expected, unsigned int code) {
	if (*pch != expected) {
		err("expected `%c` but found `%c`", expected, *pch);
	}
	pch += 2;
}

void has_second_equal_check_add_token(
	TokenArray* arr, char** pch, unsigned int position, unsigned int line, 
	unsigned int default_code, unsigned int second_equal_code
) {
	if (*(pch+1) == '=') {
		addToken(arr, position, line, second_equal_code, NULL);
		(*pch)++;
	} else {
		addToken(arr, position, line, default_code, NULL);
	}
	(*pch)++;
}

TokenArray tokenize(const char *str)
{
	char *pch = (char*) str;
	TokenArray arr = {0};
	unsigned int position = 1;
	unsigned int line = 1;
	char text[MAX_STR+1];
	while (1) {
		switch (*pch) {
			case '\0':
				addToken(&arr, position, line, FINISH, NULL);
				return arr;
			case '#':
				accumulate(text, &pch, &position, newline_accumulator, false);
				break;
			case '\n':
				line++;
				position = 0;
				pch++;
				break;
			case '\r':
				pch++;
				break;
			case ' ':
				pch++;
				break;
			case '\t':
				pch++;
				break;
			case '(':
				addToken(&arr, position, line, LPAR, NULL);
				pch++;
				break;
			case ')':
				addToken(&arr, position, line, RPAR, NULL);
				pch++;
				break;
			case ':':
				addToken(&arr, position, line, COLON, NULL);
				pch++;
				break;
			case ';':
				addToken(&arr, position, line, SEMICOLON, NULL);
				pch++;
				break;
			case ',':
				addToken(&arr, position, line, COMMA, NULL);
				pch++;
				break;
			case '<':
				has_second_equal_check_add_token(&arr, &pch, position, line, LT, LE);
				break;
			case '>':
				has_second_equal_check_add_token(&arr, &pch, position, line, GT, GE);
				break;
			case '=':
				has_second_equal_check_add_token(&arr, &pch, position, line, ASSIGN, EQ);
				break;
			case '+':
				addToken(&arr, position, line, ADD, NULL);
				pch++;
				break;
			case '-':
				addToken(&arr, position, line, SUB, NULL);
				pch++;
				break;
			case '*':
				addToken(&arr, position, line, MUL, NULL);
				pch++;
				break;
			case '/':
				addToken(&arr, position, line, DIV, NULL);
				pch++;
				break;
			case '"':
				pch++;
				position++;
				accumulate(text, &pch, &position, string_literal_accumulator, true);
				pch++;
				addToken(&arr, position, line, STR, text);
				break;
			case '&':
				consume_or_raise(pch, '&', AND);
				addToken(&arr, position, line, AND, NULL);
				break;
			case '|':
				consume_or_raise(pch, '|', OR);
				addToken(&arr, position, line, OR, NULL);
				break;
			case '!':
				has_second_equal_check_add_token(&arr, &pch, position, line, NOT, NE);
				break;
			default:
				if (word_accumulator(pch)) {
					accumulate(text, &pch, &position, word_accumulator, true);
					switch (hash(text)) {
						case HASH_VAR:
							addToken(&arr, position, line, VAR, NULL);
							break;
						case HASH_FUNCTION:
							addToken(&arr, position, line, FUNCTION, NULL);
							break;
						case HASH_IF:
							addToken(&arr, position, line, IF, NULL);
							break;
						case HASH_ELSE:
							addToken(&arr, position, line, ELSE, NULL);
							break;
						case HASH_WHILE:
							addToken(&arr, position, line, WHILE, NULL);
							break;
						case HASH_END:
							addToken(&arr, position, line, END, NULL);
							break;
						case HASH_RETURN:
							addToken(&arr, position, line, RETURN, NULL);
							break;
						case HASH_TYPE_INT:
							addToken(&arr, position, line, TYPE_INT, NULL);
							break;
						case HASH_TYPE_REAL:
							addToken(&arr, position, line, TYPE_REAL, NULL);
							break;
						case HASH_TYPE_STR:
							addToken(&arr, position, line, TYPE_STR, NULL);
							break;
						default:
							addToken(&arr, position, line, ID, text);
					}
				} else if (isdigit(*pch)) {
					char* start = pch;
					bool is_real = false;
					while (isdigit(*pch) || *pch == '.') {
						if (*pch == '.') {
							if (is_real)
								err("unexpected character `%c` at line %d position %d\n", *pch, line, position);
							is_real = true;
						}
						pch++;
						position++;
					}
					position -= 1;
					copyn(text, start, pch);
					if (is_real) {
						addToken(&arr, position, line, REAL, text);
					} else {
						addToken(&arr, position, line, INT, text);
					}
				} else {
					err("unexpected character `%c` at line %d position %d\n", *pch, line, position);
				}
		}
		position++;
	}
}

char* code_to_str(unsigned int code) {
	switch (code) {
		case ID:
			return "ID";
		case VAR:
			return "VAR";
		case FUNCTION:
			return "FUNCTION";
		case IF:
			return "IF";
		case ELSE:
			return "ELSE";
		case WHILE:
			return "WHILE";
		case END:
			return "END";
		case RETURN:
			return "RETURN";
		case TYPE_INT:
			return "TYPE_INT";
		case TYPE_REAL:
			return "TYPE_REAL";
		case TYPE_STR:
			return "TYPE_STR";
		case INT:
			return "INT";
		case REAL:
			return "REAL";
		case STR:
			return "STR";
		case COMMA:
			return "COMMA";
		case COLON:
			return "COLON";
		case SEMICOLON:
			return "SEMICOLON";
		case LPAR:
			return "LPAR";
		case RPAR:
			return "RPAR";
		case FINISH:
			return "FINISH";
		case ADD:
			return "ADD";
		case SUB:
			return "SUB";	
		case MUL:
			return "MUL";
		case DIV:
			return "DIV";
		case AND:
			return "AND";
		case OR:
			return "OR";
		case NOT:
			return "NOT";
		case ASSIGN:
			return "ASSIGN";
		case EQ:
			return "EQ";
		case NE:
			return "NE";
		case LT:
			return "LT";
		case LE:
			return "LE";
		case GT:
			return "GT";
		case GE:
			return "GE";
		default:
			err("unknown code %d", code);
	}
}