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
	if (text != NULL) {
		// TODO: process and insert
	}
}

// copy in the dst buffer the string between [begin,end)
char *copyn(char *dst, const char *begin, const char *end)
{
	char *p = dst;
	if (end - begin > MAX_STR)
		err("string too long");
	while (begin != end)
		*p++ = *begin++;
	*p = '\0';
	return dst;
}

char* accumulate(
	char* accumulator_buffer, 
	char* pch, 
	unsigned int* position, 
	bool (*must_continue)(char*)
) {
	char* start = pch;
	while (must_continue(pch)) {
		pch++;
		(*position)++;
	}
	pch--;
	(*position)--;
	copyn(accumulator_buffer, start, pch);
}

bool is_from_word(char* pch) {
	return isalpha(*pch) || *pch == '_';
}

bool newline_accumulator(char* pch) {
	return *pch != '\n';
}

TokenArray tokenize(const char *pch)
{
	TokenArray arr = {0};
	unsigned int position = 1;
	unsigned int line = 1;
	char accumulator_buffer[MAX_STR+1];
	while (1) {
		printf("%c", *pch);
		switch (*pch) {
			case '\0':
				addToken(&arr, position, line, FINISH, NULL);
				return arr;
			case '#':
				accumulate(accumulator_buffer, pch, &position, newline_accumulator);
				break;
			case '\n':
				line++;
				position = 0;
				break;
			case '\r':
				break;
			case ' ':
				break;
			default:
				if (is_from_word(pch)) {
					char* text = accumulate(accumulator_buffer, pch, &position, is_from_word);
					switch (hash(text)) {
						case 7572387384277067:
							addToken(&arr, position, line, FUNCTION, text);
							break;
					}
				}
				err("unexpected character `%c` at position %d line %d", *pch, position, line);
		}
		position++;
	}
}

