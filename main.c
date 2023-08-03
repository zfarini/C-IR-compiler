#include "compiler.h"

char *find_char_in_str(char *s, char c)
{
    while (*s)
    {
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
    if (!f)
    {
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

    IR_Code *c = gen_ir_code(node);

    print_ir_code(c);
    sim_ir_code(c);

    gen_control_flow_graph(c);
}
