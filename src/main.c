#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "lexer.h"
#include "parser.h"

int main() {
    printf("Started...\n");
    char* buffer = loadFile("examples/1.q");
    TokenArray t = tokenize(buffer);
    for (Token *token = t.tokens; token < t.tokens + t.length; ++token)
        print_token(token);
    parse(&t);
    free(buffer);
    return 0;
}