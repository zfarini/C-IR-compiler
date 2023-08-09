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

#if 1
IR_Code *cfg_to_ir_code(IR_Code *c)
{
    IR_Code *res = calloc(1, sizeof(*res));


    int total_instructions = 0;
	int	total_blocks = 0;
    for (int i = 0; i < c->function_count; i++)
	{
		Control_Flow_Graph *g = c->functions[i].cfg;
		
		for (int j = 0; j < g->block_count; j++)
        	total_instructions += g->blocks[j].instruction_count;
		total_blocks += g->block_count;
	}

	// why *res = *c crashes??
    res->instructions = calloc(sizeof(*c->instructions), total_instructions);
    res->labels = calloc(sizeof(*c->labels), total_blocks + c->function_count);
	res->functions = calloc(sizeof(*res->functions), c->function_count);
	res->function_count = c->function_count;
	res->label_count = res->function_count;
	res->reserved_reg = c->reserved_reg;
	memcpy(res->vars_reg, c->vars_reg, sizeof(c->vars_reg));

	for (int i = 0; i < c->function_count; i++)
	{
		Control_Flow_Graph *g = c->functions[i].cfg;
		

		res->functions[i] = c->functions[i];
		res->functions[i].first_instruction = res->instruction_count;
		res->labels[i] = res->instruction_count;

		int first = res->label_count;

		for (int j = 0; j < g->block_count; j++)
		{
			//if (j)
			res->labels[res->label_count++] = res->instruction_count;
			for (int k = 0; k < g->blocks[j].instruction_count; k++)
			{
				IR_Instruction *e = &res->instructions[res->instruction_count++];

				*e = g->blocks[j].instructions[k];
				if (e->op == OP_CALL)
					e->r0 *= -1;
				else if (e->op == OP_JMP || e->op == OP_JMPZ)
					e->r0 = first + e->r0;
			}
		}

		res->functions[i].instruction_count = res->instruction_count - res->functions[i].first_instruction;
		//res->labels[res->label_count++] = res->instruction_count;
	}
    return res;
}
#endif

int main(void)
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
	
	//for (int i = 0; i < c->function_count; i++)
	//	c->functions[i].cfg = gen_function_control_flow_graph(c, &c->functions[i]);

	//IR_Code *final = cfg_to_ir_code(c);

	//print_ir_code(final);

	//sim_ir_code(final);

}
