#include <stdio.h>
#include <stdlib.h>
#include "utils.c"

#include "lexer.c"
#include "parser.c"

int main() {
    printf("Started...\n");
    char* raw_code_buffer = loadFile("resources/1.q");
    TokenArray t = tokenize(raw_code_buffer);
    for (int i = 0; i < t.length; i++) {
        print_token(&t.tokens[i]);
    }
    parse(&t);
    free(raw_code_buffer);
    return 0;
}