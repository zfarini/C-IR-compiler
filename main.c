#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "compiler.h"

char *find_char_in_str(char *s, char c)
{
    while (*s) {
        if (*s == c)
            return s;
        s++;
    }
    return 0;
}

#include "tokenizer.c"
#include "parser.c"
#include "ir.c"

char *load_entire_file(char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("failed to load file: %s\n", filename);
        assert(f);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *result = malloc(length + 1);
    assert(result);
    fread(result, 1, length, f);
    result[length] = 0;
    fclose(f);

    return result;
}

int main()
{
    char *s = load_entire_file("code.txt");
    Token *tokens = tokenize(s);

#if 0
    for (int i = 0; tokens[i].type; i++)
    {
        printf("'");
        for (int j = tokens[i].c0; j < tokens[i].c1; j++)
            printf("%c", s[j]);
        printf("' ");
    }
    printf("\n");
#endif

    Node *node = parse(tokens);
    while (node)
    {
        gen_ir(node);
        node = node->next_expr;
    }

    printf("variables:\n");
    for (int i = 0; vars_reg[i].name; i++)
        printf("%s -> t%d\n", vars_reg[i].name, vars_reg[i].reg);

    printf("IR:\n");
    print_ir();

    pthread_t thread;
    pthread_create(&thread, 0, sim_ir, 0);
    pthread_join(thread, 0);

    optimize_ir();
    printf("Optimized:\n");
    print_ir();

    pthread_create(&thread, 0, sim_ir, 0);
    pthread_join(thread, 0);

}
