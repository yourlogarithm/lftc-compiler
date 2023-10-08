#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "../include/lexer.h"
#include "../include/utils.h"


Token tokens[MAX_TOKENS];
int nTokens;

int line = 1; // the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code)
{
	if (nTokens == MAX_TOKENS)
		err("too many tokens");
	Token *tk = &tokens[nTokens];
	tk->code = code;
	tk->line = line;
	nTokens++;
	return tk;
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

void tokenize(const char *pch)
{
	const char *start;
	Token *tk;
	char buf[MAX_STR + 1];
	for (;;)
	{
		switch (*pch)
		{
		case ' ':
		case '\t':
			pch++;
			break;
		case '\r': // handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
			if (pch[1] == '\n')
				pch++;
			// fallthrough to \n
		case '\n':
			line++;
			pch++;
			break;
		case '\0':
			addTk(FINISH);
			return;
		case ',':
			addTk(COMMA);
			pch++;
			break;
		case '=':
			if (pch[1] == '=') {
				addTk(EQ);
				pch += 2;
			}
			else {
				addTk(ASSIGN);
				pch++;
			}
			break;
		case '(':
			addTk(LPAR);
			pch++;
			break;
		case ')':
			addTk(RPAR);
			pch++;
			break;
		case ':':
			addTk(COLON);
			pch++;
			break;
		case '<':
			if (pch[1] == '=') {
				addTk(LTE);
				pch += 2;
			} else {
				addTk(LT);
				pch++;
			}
			break;
		case '>':
			if (pch[1] == '=') {
				addTk(GTE);
				pch += 2;
			} else {
				addTk(GT);
				pch++;
			}
			break;
		case ';':
			addTk(SEMICOLON);
			pch++;
			break;
		case '#':
			while (*pch != '\n')
				pch++;
			break;
		default:
			if (isalpha(*pch) || *pch == '_') {
				for (start = pch++; isalnum(*pch) || *pch == '_'; pch++)
				{
				}
				char *text = copyn(buf, start, pch);
				if (strcmp(text, "int") == 0)
					addTk(TYPE_INT);
				else if (strcmp(text, "str") == 0)
					addTk(TYPE_STR);
				else if (strcmp(text, "real") == 0)
					addTk(TYPE_REAL);
				else if (strcmp(text, "function") == 0) {
					addTk(FUNCTION);
				}
				else if (strcmp(text, "return") == 0) {
					addTk(RETURN);
				}
				else if (strcmp(text, "if") == 0) {
					addTk(IF);
				}
				else if (strcmp(text, "else") == 0) {
					addTk(ELSE);
				}
				else if (strcmp(text, "end") == 0) {
					addTk(END);
				}
				else
				{
					tk = addTk(ID);
					strcpy(tk->text, text);
				}
			} else if (isnumber(*pch)) {
				short isReal = 0;
				for (start = pch++; isnumber(*pch) || *pch == '.'; pch++)
				{
					if (*pch == '.') {
						if (isReal) {
							err("invalid real number");
						}
						isReal = 1;
					}
				}
				if (isReal)
					addTk(REAL);
					
			}
			else
				err("invalid char: %c (%d)", *pch, *pch);
		}
	}
}

void showTokens()
{
	for (int i = 0; i < nTokens; i++)
	{
		Token *tk = &tokens[i];
		printf("%d\n", tk->code);
	}
}
