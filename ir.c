#include "compiler.h"

char *get_ir_op_str(int type)
{
    local char s[256][2]; // ???
    struct {
        char *name;
        int type;
    } op_str[] = {
        {"==",      OP_EQUAL},
        {"!=",      OP_NOT_EQUAL},
        {"<=",      OP_LESS_OR_EQUAL},
        {">=",      OP_GREATER_OR_EQUAL},
    };

    if (type < 256)
    {
        s[type][0] = type;
        return s[type];
    }
    else
    {
        for (int i = 0; i < array_length(op_str); i++)
            if (op_str[i].type == type)
                return op_str[i].name;
    }

    assert(0);
    return "UNKOWN_OP_STR";
}

int get_var_register(IR_Code *c, Node *decl)
{
    int i = 0;

    while (c->vars_reg[i].decl && c->vars_reg[i].decl != decl)
        i++;

    if (!c->vars_reg[i].decl)
        return (-1);

    return c->vars_reg[i].reg;
}

void set_var_register(IR_Code *c, Node *decl, int reg)
{
    int i = 0;

    while (c->vars_reg[i].decl && c->vars_reg[i].decl != decl)
        i++;

    assert(i < array_length(c->vars_reg));
    c->vars_reg[i].decl = decl;
    c->vars_reg[i].reg = reg;
}

IR_Instruction *add_instruction(IR_Code *c, int op)
{
    IR_Instruction *e = &c->instructions[c->instruction_count];

    e->op = op;
    c->instruction_count++;

    return e;
}

int token_type_to_ir_op(int type)
{
    if (type < 256)
        return type;
    else if (type == TOKEN_EQUAL)
        return OP_EQUAL;
    else if (type == TOKEN_NOT_EQUAL)
        return OP_NOT_EQUAL;
    else if (type == TOKEN_LESS_OR_EQUAL)
        return OP_LESS_OR_EQUAL;
    else if (type == TOKEN_GREATER_OR_EQUAL)
        return OP_GREATER_OR_EQUAL;
    else
        assert(0);
}

Function *find_function(IR_Code *c, char *name)
{
	for (int i = 0; i < c->function_count; i++)
		if (!strcmp(name, c->functions[i].name))
			return &c->functions[i];

	return 0;
}


int gen_ir(IR_Code *c, Node *node)
{
    int reg = -1;

    if (node->type == NODE_NUMBER)
    {
        IR_Instruction  *e = add_instruction(c, OP_MOV);
        e->r0 = c->curr_reg;
        e->r1 = node->value;
        e->r1_imm = 1;
        reg = c->curr_reg;
        c->curr_reg++;
    }
    else if (node->type == NODE_VAR)
    {
        assert(node->decl);
        assert(node->decl->type == NODE_VAR_DECL);
        reg = get_var_register(c, node->decl);
        assert(reg >= 0);

    }
    else if (node->type == NODE_FUNC_DEF)
    {
		Function *f = find_function(c, node->token->name);

		assert(f);

		c->labels[f->label] = c->instruction_count;
		f->first_instruction = c->instruction_count;

        gen_ir(c, node->body);
        add_instruction(c, OP_RET);
		f->instruction_count = c->instruction_count - f->first_instruction;
    }
    else if (node->type == NODE_FUNC_CALL)
    {
        IR_Instruction *e = add_instruction(c, OP_CALL);

		Function *f = find_function(c, node->token->name);
		assert(f);

		e->r0 = f->label;
    }
    else if (node->type == NODE_PRINT)
    {
        int r = gen_ir(c, node->left);

        IR_Instruction *e = add_instruction(c, OP_PRINT);
        e->r1 = r;
    }
    else if (node->type == NODE_BINOP && node->op == '=')
    {
        int r1 = gen_ir(c, node->right);

        IR_Instruction *e = add_instruction(c, OP_MOV);
        e->r0 = gen_ir(c, node->left);
        e->r1 = r1;
        reg = e->r0;
    }
    else if (node->type == NODE_BINOP)
    {
        int r1 = gen_ir(c, node->left);
        int r2 = gen_ir(c, node->right);

        IR_Instruction *e = add_instruction(c, token_type_to_ir_op(node->op));
        e->r0 = c->curr_reg;
        e->r1 = r1;
        e->r2 = r2;

        reg = c->curr_reg;
        c->curr_reg++;
    }
    else if (node->type == NODE_WHILE || node->type == NODE_IF)
    {
        int start_label = c->label_count;
        if (node->type == NODE_WHILE)
            c->labels[c->label_count++] = c->instruction_count;

        int r0 = gen_ir(c, node->left);

        IR_Instruction *e = add_instruction(c, OP_JMPZ);
        e->r1 = r0;
        
        gen_ir(c, node->right);

        IR_Instruction *f = 0;
        int exit_label, else_label = -1;
        if (node->else_node)
        {
            f = add_instruction(c, OP_JMP);
            else_label = c->label_count++;
            c->labels[else_label] = c->instruction_count;
            gen_ir(c, node->else_node);
        }
        else if (node->type == NODE_WHILE)
            add_instruction(c, OP_JMP)->r0 = start_label;

        exit_label = c->label_count++;
        c->labels[exit_label] = c->instruction_count;

        e->r0 = (node->else_node ? else_label : exit_label);
        if (f)
            f->r0 = exit_label;
    }
    else if (node->type == NODE_BLOCK)
    {
        Node *curr = node->first_stmt;

        while (curr)
        {
            gen_ir(c, curr);
            curr = curr->next_stmt;
        }
    }
    else if (node->type == NODE_VAR_DECL)
    {
        reg = c->curr_reg++;
        set_var_register(c, node, reg);
    }
    else
        assert(0);
    return reg;
}

IR_Code *gen_ir_code(Node *node)
{
    IR_Code *c = calloc(1, sizeof(*c));

    c->instructions = calloc(sizeof(*c->instructions), 16384);
    c->labels = calloc(sizeof(*c->labels), 256);
	c->functions = calloc(sizeof(*c->functions), 64);

	Node *curr = node;
	while (curr)
	{
		Function *f = &c->functions[c->function_count++];

		f->name = curr->token->name;
		f->label = c->label_count++;

		curr = curr->next_func;
	}

	curr = node;
    while (curr)
    {
        gen_ir(c, curr);
		curr = curr->next_func;
    }

    return c;
}

int eval_op(int op, int r1, int r2)
{
    int res = 0;

    if (op == OP_ADD)
        res = r1 + r2;
    else if (op == OP_SUB)
        res = r1 - r2;
    else if (op == OP_MUL)
        res = r1 * r2;
    else if (op == OP_DIV)
        res = (r2 == 0 ? 1 : r1 / r2);
    else if (op == OP_MOD)
        res = (r2 == 0 ? 1 : r1 % r2);
    else if (op == OP_LESS)
        res = r1 < r2;
    else if (op == OP_GREATER)
        res = r1 > r2;
    else if (op == OP_EQUAL)
        res = r1 == r2;
    else if (op == OP_LESS_OR_EQUAL)
        res = r1 <= r2;
    else if (op == OP_GREATER_OR_EQUAL)
        res = r1 >= r2;
    else if (op == OP_NOT_EQUAL)
        res = r1 != r2;
    else
        assert(0);
    return res;
}

void sim_ir_code(IR_Code *c)
{
    int regs[128] = {};
    int ip = 0;

    fflush(stderr); // ??
    printf("\033[1;32msim output:\033[0m\n");


	Function *main = find_function(c, "main");

    if (!main)
    {
        printf("SIM ERROR: main is not defined\n");
        return ;
    }

    ip = main->first_instruction;
    int call_stack[256];
    int call_stack_depth = 0;

    while (1)
    {
        IR_Instruction *e = &c->instructions[ip];
        
        int r1_value = (e->r1_imm ? e->r1 : regs[e->r1]);
        int r2_value = (e->r2_imm ? e->r2 : regs[e->r2]);

        if (e->op < OP_BINARY)
            regs[e->r0] = eval_op(e->op, r1_value, r2_value);
        else if (e->op == OP_MOV)
            regs[e->r0] = r1_value;
        else if (e->op == OP_JMP)
        {
            ip = c->labels[e->r0];
            continue ;
        }
        else if (e->op == OP_JMPZ)
        {
            if (!regs[e->r1])
            {
                ip = c->labels[e->r0];
                continue ;
            }
        }
        else if (e->op == OP_PRINT)
        {
            printf("%d\n", r1_value);
        }
        else if (e->op == OP_CALL)
        {
            if (call_stack_depth >= array_length(call_stack))
            {
                printf("SIM ERROR: call stack is too deep (%d)\n", array_length(call_stack));
                return ;
            }
            call_stack[call_stack_depth++] = ip + 1;
            ip = e->r0;
            continue ;
        }
        else if (e->op == OP_RET)
        {
            if (call_stack_depth == 0)
                break ;
            ip = call_stack[call_stack_depth - 1];
            call_stack_depth--;
            continue ;
        }
        else
            assert(0);
        ip++;
    }
}

void print_instruction(IR_Code *c, IR_Instruction *e, int in_block)
{
    if (e->op == OP_JMP)
        printf("jmp %s%d", (in_block ? "B" : "L"), e->r0);
    else if (e->op == OP_JMPZ)
        printf("jmpz %s%d t%d", (in_block ? "B" : "L"), e->r0, e->r1);
    else if (e->op == OP_PRINT)
        printf("print %s%d", e->r1_imm ? "" : "t", e->r1);
    else if (e->op == OP_CALL)
        printf("call %s", c->functions[in_block ? -e->r0 : e->r0].name);
    else if (e->op == OP_RET)
        printf("ret");
    else
    {
        printf("t%d = ", e->r0);
        if (e->op == OP_MOV)
            printf("%s%d", e->r1_imm ? "" : "t", e->r1);
        else
        {
            printf("%s%d %s %s%d", e->r1_imm ? "" : "t", e->r1,
                    get_ir_op_str(e->op), e->r2_imm ? "" : "t", e->r2);
        }
    }
    printf("\n");
}

void print_ir_code(IR_Code *c)
{
    printf("\033[1;32mgenerated ir:\033[0m\n");

    for (int i = 0; i < c->instruction_count; i++)
    {
        IR_Instruction *e = &c->instructions[i];

        for (int j = 0; j < c->label_count; j++)
        {
            if (c->labels[j] == i)
            {
				if (j < c->function_count)
					printf("%s:\n", c->functions[j].name);
                else
                    printf("\tL%d:\n", j);
            }
        }

        printf("%-16s", "");
        print_instruction(c, e, 0);
    }

    for (int j = 0; j < c->label_count; j++)
    {
        if (c->labels[j] == c->instruction_count)
            printf("\tL%d:\n", j);
        assert(c->labels[j] <= c->instruction_count);
    }
}

enum
{
    VALUE_VALUE,
    VALUE_CONST,
    VALUE_REGISTER,
    VALUE_OP,
};

typedef struct
{
    int type;
    int op;
    int v1;
    int v2;
} Value;

typedef struct 
{
    Value   *values;
    int     *value_owner;
    int     *register_value;
    int     max_register_count;
    int     max_value_count;
    int     value_count;
} Local_Value_State;

int find_value(Local_Value_State *s, Value *to_find)
{
    for (int i = 0; i < s->value_count; i++)
        if (!memcmp(&s->values[i], to_find, sizeof(*to_find)))
            return i;

    return s->value_count;
}

void assign_register_to_value(Local_Value_State *s, int reg, int i)
{
    assert(i < s->value_count); 

    for (int j = 0; j < s->value_count; j++)
    {
        if (s->value_owner[j] == reg)
        {
            s->value_owner[j] = -1;
            for (int k = 0; k < s->max_register_count; k++)
            {
                if (k != reg && s->register_value[k] == j)
                {
                    s->value_owner[j] = k;
                    break ;
                }
            }
        }
    }

    if (s->value_owner[i] == -1)
        s->value_owner[i] = reg;
    s->register_value[reg] = i;
}

int is_binop_commutative(int op)
{
    assert(op < OP_BINARY);
    return (op == '+' || op == '*' || op == OP_EQUAL ||
            op == OP_NOT_EQUAL);
}

void add_value_register(Local_Value_State *s, int r)
{
    s->register_value[r] = s->value_count;
    s->values[s->value_count].type = VALUE_REGISTER;
    s->values[s->value_count].v1 = r;
    s->value_owner[s->value_count] = r;
    s->value_count++;
}

void local_value_numbering_for_basic_block(IR_Basic_Block *b)
{
    Local_Value_State *s = calloc(1, sizeof(*s)); // idk why not in stack, whatever
    
    s->max_value_count = b->instruction_count * 2;
    s->max_register_count = 1024;

    s->values           = calloc(s->max_value_count,     sizeof(*s->values));
    s->value_owner      = calloc(s->max_value_count,     sizeof(*s->value_owner));
    s->register_value   = calloc(s->max_register_count,  sizeof(*s->register_value));
    s->value_count = 0;
    memset(s->value_owner, -1, s->max_value_count * sizeof(*s->value_owner));
    memset(s->register_value, -1, s->max_register_count * sizeof(*s->register_value));

    int *register_value = s->register_value;
    int *value_owner = s->value_owner;
    Value *values = s->values;

    for (int i = 0; i < b->instruction_count; i++)
    {
        IR_Instruction *e = &b->instructions[i];

        if (e->op == OP_MOV || e->op == OP_PRINT)
        {
            Value v = {0};
            v.type = (e->r1_imm ? VALUE_CONST : VALUE_REGISTER);
            v.v1 = e->r1;
            
            int j = find_value(s, &v);

            if (!e->r1_imm && register_value[e->r1] == -1)
                add_value_register(s, e->r1);
            if (!e->r1_imm)
                j = register_value[e->r1];
            else if (j == s->value_count)
                values[s->value_count++] = v;

            if (!e->r1_imm)
                assert(value_owner[j] != -1);

            if (e->op != OP_PRINT)
                assign_register_to_value(s, e->r0, j);

            if (values[j].type == VALUE_CONST)
            {
                e->r1 = values[j].v1;
                e->r1_imm = 1;
            }
            else
                e->r1 = value_owner[j];
        }
        else if (e->op < OP_BINARY)
        {
            Value v = {0};
            v.type = VALUE_OP;
            v.op = e->op;

            //assert(!e->r1_imm && !e->r2_imm);

            if (!e->r1_imm && register_value[e->r1] == -1)
                add_value_register(s, e->r1);
            if (!e->r2_imm && register_value[e->r2] == -1)
                add_value_register(s, e->r2);

			if (e->r1_imm)
			{
				Value v1 = {.type = VALUE_CONST, .v1 = e->r1};	
				int j = find_value(s, &v1);
				if (j == s->value_count)
					values[s->value_count++] = v1;
				v.v1 = j;
			}
			else
            	v.v1 = register_value[e->r1];

			if (e->r2_imm)
			{
				Value v2 = {.type = VALUE_CONST, .v1 = e->r2};	
				int j = find_value(s, &v2);
				if (j == s->value_count)
					values[s->value_count++] = v2;
				v.v2 = j;
			}
			else
            	v.v2 = register_value[e->r2];

            if (v.v1 > v.v2 && is_binop_commutative(e->op))
            {
                int temp = v.v1;
                v.v1 = v.v2;
                v.v2 = temp;
            }

            int j = find_value(s, &v);

            if (j == s->value_count)
                values[s->value_count++] = v;

            if (value_owner[j] != -1)
            {
                e->op = OP_MOV;
                e->r1 = value_owner[j];
            }
            else
            {
				if (!e->r1_imm)
				{
               		if (values[register_value[e->r1]].type == VALUE_CONST)
               		    e->r1_imm = 1, e->r1 = values[register_value[e->r1]].v1;
               		else
               		    e->r1 = value_owner[register_value[e->r1]];
				}

				if (!e->r2_imm)
				{
               	 	if (values[register_value[e->r2]].type == VALUE_CONST)
               	 	    e->r2_imm = 1, e->r2 = values[register_value[e->r2]].v1;
               	 	else
               	 	    e->r2 = value_owner[register_value[e->r2]];
				}

                if (e->r1_imm && e->r2_imm)
                {
                    e->r1 = eval_op(e->op, e->r1, e->r2);
                    e->op = OP_MOV;
                    e->r1_imm = 1;

                    Value v3 = {.type = VALUE_CONST, .v1 = e->r1};

                    j = find_value(s, &v3);
                    if (j == s->value_count)
                        values[s->value_count++] = v3;
                }
            }

            assign_register_to_value(s, e->r0, j);
        }
    }
#if 0
    printf("values:\n");
    for (int j = 0; j < s->value_count; j++)
    {
        Value *v = &values[j];

        printf("\t#%d: ", j);
        if (v->type == VALUE_CONST)
            printf("const(%d)", v->v1);
        else if (v->type == VALUE_REGISTER)
            printf("t%d", v->v1);
        else if (v->type == VALUE_OP)
            printf("#%d %s #%d", v->v1, get_ir_op_str(v->op), v->v2);
        else
            assert(0);
        printf("\n");
    }
#endif

}

Control_Flow_Graph *gen_function_control_flow_graph(IR_Code *c, Function *f)
{
    Control_Flow_Graph *g = calloc(1, sizeof(*g));

    IR_Basic_Block *curr_block = &g->blocks[g->block_count++];

    int label_to_block_index[256];
    memset(label_to_block_index, -1, sizeof(label_to_block_index));

	int last_instruction = f->first_instruction + f->instruction_count;

    for (int i = f->first_instruction; i < last_instruction; i++)
    {
        int label_here = 0;

        for (int j = 0; j < c->label_count; j++)
        {
            if (c->labels[j] == i)
            {
                label_to_block_index[j] = (i != f->first_instruction ? g->block_count : 0);
                label_here = 1;
            }
        }

        if (i != f->first_instruction && (label_here || c->instructions[i - 1].op == OP_JMP ||
            c->instructions[i - 1].op == OP_JMPZ ||
            c->instructions[i - 1].op == OP_RET))
        {
            assert(g->block_count < array_length(g->blocks));
            curr_block = &g->blocks[g->block_count];
            curr_block->index = g->block_count;
            g->block_count++;
        }

        assert(curr_block->instruction_count < array_length(curr_block->instructions));
        IR_Instruction *e = &curr_block->instructions[curr_block->instruction_count];
        *e = c->instructions[i];

        curr_block->instruction_count++;
    }
    
    // should we increment block_count here or not?
    // because this is more like a virtual block in the end of function
    g->blocks[g->block_count].index = g->block_count;

    for (int j = 0; j < c->label_count; j++)
    {
        if (c->labels[j] == last_instruction)
            label_to_block_index[j] = g->block_count;
    }

    for (int i = 0; i < g->block_count; i++)
    {
        IR_Basic_Block *block = &g->blocks[i];

        // change jumps to point to block indices instead of labels
        for (int j = 0; j < block->instruction_count; j++)
        {
            if (block->instructions[j].op == OP_JMP || block->instructions[j].op == OP_JMPZ 
				|| block->instructions[j].op == OP_CALL)
			{
				if (block->instructions[j].r0 < c->function_count)
				{
					// later we may want to jmp directly to a function
					assert(block->instructions[j].op == OP_CALL);
					block->instructions[j].r0 *= -1;
				}
				else
                	block->instructions[j].r0 = label_to_block_index[block->instructions[j].r0];
			}
        }

#if 1
        IR_Instruction *last_inst = &block->instructions[block->instruction_count - 1];

        if (last_inst->op == OP_JMP)
        {
            block->childs[block->child_count++] = &g->blocks[last_inst->r0];
        }
        else if (last_inst->op == OP_JMPZ)
        {
            if (last_inst->op == OP_JMPZ)
                block->childs[block->child_count++] = &g->blocks[i + 1];
            block->childs[block->child_count++] = &g->blocks[last_inst->r1];
        }
        else
            block->childs[block->child_count++] = &g->blocks[i + 1];
#endif
    }

	printf("\033[4;33mfunction %s:\033[0m\n", f->name);
    printf("\033[1;34mbasic blocks:\033[0m\n");
    for (int i = 0; i < g->block_count; i++)
    {
        IR_Basic_Block *block = &g->blocks[i];

        printf("block %d (%d instructions, childs: [", i, block->instruction_count);
        for (int j = 0; j < block->child_count; j++)
        {
            printf("%d", block->childs[j]->index);
            if (j + 1 < block->child_count)
                printf(", ");
        }
        printf("]):\n");

        for (int j = 0; j < block->instruction_count; j++)
        {
            IR_Instruction *e = &block->instructions[j];

            printf("\t\t");
            print_instruction(c, e, 1);
        }
    }

#if 1
    for (int i = 0; i < g->block_count; i++)
    {
        IR_Basic_Block *block = &g->blocks[i];

        local_value_numbering_for_basic_block(block);
	}
#if 1
    while (1)
    {

        int register_used[256] = {0};
        for (int i = 0; i < g->block_count; i++)
        {
            for (int j = 0; j < g->blocks[i].instruction_count; j++)
            {
                IR_Instruction *e = &g->blocks[i].instructions[j];

                 if (!e->r1_imm && (e->op == OP_MOV || 
                     e->op == OP_PRINT || e->op == OP_JMPZ || e->op < OP_BINARY))
                     register_used[e->r1]++;
                 if (e->op < OP_BINARY && !e->r2_imm)
                     register_used[e->r2]++;
            }
        }

        int end = 1;

        for (int i = 0; i < g->block_count; i++)
        {
            for (int j = 0; j < g->blocks[i].instruction_count; j++)
            {
                IR_Instruction *e = &g->blocks[i].instructions[j];

                int remove = 0;
                // this is a block optimization for now, I'm not sure if can extended it to global
                // it happens when a register is used once to store an op and then moved to another
                // one
                if ((e->op == OP_MOV || e->op < OP_BINARY) && register_used[e->r0] == 1)
                {
                    for (int k = j + 1; k < g->blocks[i].instruction_count; k++)
                    {
                        IR_Instruction *f = &g->blocks[i].instructions[k];

                        if (f->op == OP_MOV && !f->r1_imm && f->r1 == e->r0)
                        {
                            int r = f->r0;

                            *f = *e;
                            f->r0 = r;
                            remove = 1;
                        }
                            
                    }
                }
                if (remove || (((e->op == OP_MOV || e->op < OP_BINARY) && !register_used[e->r0])
                    || (e->op == OP_MOV && !e->r1_imm && e->r0 == e->r1)))
                {
                    for (int k = j + 1; k < g->blocks[i].instruction_count; k++)
                        g->blocks[i].instructions[k - 1] = g->blocks[i].instructions[k];
                    g->blocks[i].instruction_count--;
                    end = 0;
                }

            }
        }
        if (end)
            break ;
    }

    printf("\033[1;32moptimized blocks:\033[0m\n");

    for (int i = 0; i < g->block_count; i++)
    {
        IR_Basic_Block *block = &g->blocks[i];

        printf("block %d (%d instructions, childs: [", i, block->instruction_count);
        for (int j = 0; j < block->child_count; j++)
        {
            printf("%d", block->childs[j]->index);
            if (j + 1 < block->child_count)
                printf(", ");
        }
        printf("]):\n");

        for (int j = 0; j < block->instruction_count; j++)
        {
            IR_Instruction *e = &block->instructions[j];

            printf("\t\t");
            print_instruction(c, e, 1);
        }
    }
#endif
#endif


    return g;
}
