#include <stdio.h>
#include <stdlib.h>
#include "utils.c"
#include "lexer.c"

int main() {
    printf("Started...\n");
    char* raw_code_buffer = loadFile("resources/1.q");
    TokenArray t = tokenize(raw_code_buffer);
    for (int i = 0; i < t.length; i++) {
        Token tk = t.tokens[i];
        char* str_repr = code_to_str(tk.code);
        if (tk.code == ID || tk.code == STR)
            printf("%s:%s\n", str_repr, tk.text);
        else if (tk.code == INT)
            printf("%s:%d\n",  str_repr, tk.i);
        else if (tk.code == REAL)
            printf("%s:%f\n", str_repr, tk.r);
        else
            printf("%s\n", str_repr);
    }
    free(raw_code_buffer);
    return 0;
}