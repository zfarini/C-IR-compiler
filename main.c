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

IR_Code *cfg_to_ir_code(Control_Flow_Graph *g)
{
    IR_Code *c = calloc(1, sizeof(*c));

    int total = 0;
    for (int i = 0; i < g->block_count; i++)
        total += g->blocks[i].instruction_count;

    c->instructions = calloc(sizeof(*c->instructions), total);
    c->labels = calloc(sizeof(*c->labels), g->block_count + 1);

    for (int i = 0; i < g->block_count; i++)
    {
        c->labels[c->label_count++] = c->instruction_count;
        for (int j = 0; j < g->blocks[i].instruction_count; j++)
            c->instructions[c->instruction_count++] = g->blocks[i].instructions[j];
    }
    c->labels[c->label_count++] = c->instruction_count;

    return c;
}

int main(void)
{
    char *s = load_entire_file("/Users/zakariafarini/Desktop/Compiler/Compiler/code.txt");
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

 //   Control_Flow_Graph *g = gen_control_flow_graph(c);

 //   IR_Code *final = cfg_to_ir_code(g);
 //   print_ir_code(final);
 //   sim_ir_code(final);
}
