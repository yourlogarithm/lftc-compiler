#include <stdio.h>
#include <stdlib.h>
#include "utils.c"
#include "lexer.c"

int main() {
    printf("Started...\n");
    char* raw_code_buffer = loadFile("../resources/1.q");
    tokenize(raw_code_buffer);
    showTokens();
    free(raw_code_buffer);
    return 0;
}