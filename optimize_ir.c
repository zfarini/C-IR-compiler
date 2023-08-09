enum
{
    VALUE_VALUE,
    VALUE_CONST,
    VALUE_REGISTER,
    VALUE_OP,
	VALUE_FUNC_CALL,
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

	if (s->register_value[reg] == i)
		return ;
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

void local_value_numbering_for_basic_block(IR_Code *c, IR_Basic_Block *b)
{
    Local_Value_State *s = calloc(1, sizeof(*s)); // idk why not in stack, whatever
    
    s->max_value_count = 1024;
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

		if (e->op == OP_CALL)
		{
			//TODO: think about this more
			for (int j = 0; j < c->reserved_reg; j++) // use 1 + function_arg_count
			{
				Value v = {0};
				v.type = VALUE_FUNC_CALL;
				v.v1 = i; // just to avoid matching with another function call
				v.v2 = j;
				values[s->value_count++] = v;
				assign_register_to_value(s, j, s->value_count - 1);
			}

		}
		else if (e->op == OP_MOV || e->op == OP_PRINT || e->op == OP_ASSERT)
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

            if (e->op == OP_MOV)
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
		//else if (e->op == OP_LOAD)
		//{

		//	e->r2 = ;
		//}
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
		else if (v->type == VALUE_FUNC_CALL)
			printf("call ??");
        else
            assert(0);
        printf("\n");
    }
#endif
	free(s->values);
	free(s->value_owner);
	free(s->register_value);

}

void remove_instruction(IR_Basic_Block *b, int i)
{
	for (int j = i + 1; j < b->instruction_count; j++)
		b->instructions[j - 1] = b->instructions[j];
    b->instruction_count--;
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

		// remove jumps to next instruction
		//if (i != f->first_instruction && c->instructions[i - 1].op == OP_JMP
		//		&& c->labels[c->instructions[i - 1].r0] == i)
		//{
		//	curr_block->instruction_count--;
		//}
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
		else if (last_inst->op == OP_RET) // we could just leave it with no childs?
			block->childs[block->child_count++] = &g->blocks[g->block_count];
        else
            block->childs[block->child_count++] = &g->blocks[i + 1];
#endif
    }

#if 0
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
#endif

#if 1
    for (int i = 0; i < g->block_count; i++)
    {
        IR_Basic_Block *block = &g->blocks[i];

        local_value_numbering_for_basic_block(c, block);
	}
#if 0
    while (1)
    {

        int register_used[256] = {0};

		memset(register_used, 1, c->reserved_reg * sizeof(int)); // have a big value for the reserved regs


        int old_count = 0;

        for (int i = 0; i < g->block_count; i++)
        {
			old_count += g->blocks[i].instruction_count;

            for (int j = 0; j < g->blocks[i].instruction_count; j++)
            {
                IR_Instruction *e = &g->blocks[i].instructions[j];

                 if (!e->r1_imm && (e->op == OP_MOV || 
                     e->op == OP_PRINT || e->op == OP_JMPZ || e->op < OP_BINARY
					 || e->op == OP_LOAD || e->op == OP_STORE))
                     register_used[e->r1]++;
                 if (!e->r2_imm && (e->op < OP_BINARY || e->op == OP_LOAD || e->op == OP_STORE))
                     register_used[e->r2]++;
            }
        }



        for (int i = 0; i < g->block_count; i++)
        {
            for (int j = 0; j < g->blocks[i].instruction_count; j++)
            {
                IR_Instruction *e = &g->blocks[i].instructions[j];

                int remove = 0;
                // this is a block optimization for now, I'm not sure if can extended it to global
                // it happens when a register is used once to store an op and then moved to another
#if 1
				if ((e->op == OP_MOV || e->op < OP_BINARY) && register_used[e->r0] == 1)
                {
                    for (int k = j + 1; k < g->blocks[i].instruction_count; k++)
                    {
                        IR_Instruction *f = &g->blocks[i].instructions[k];

						if (f->op == OP_CALL)
							break ;
                        if (f->op == OP_MOV && !f->r1_imm && f->r1 == e->r0)
                        {
                            int r = f->r0;

                           	*f = *e;
                           	f->r0 = r;
                            remove = 1;
                        }
                            
                    }
                }
#endif
#if 1
                if (remove || (((e->op == OP_MOV || e->op < OP_BINARY) && !register_used[e->r0])
                    || (e->op == OP_MOV && !e->r1_imm && e->r0 == e->r1)))
                {
					remove_instruction(&g->blocks[i], j);
					j--;
                }
#endif
            }
        }
		int new_count = 0;
		for (int i = 0; i < g->block_count; i++)
			new_count += g->blocks[i].instruction_count;

        if (new_count == old_count)
            break ;
    }
#if 0
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
#endif


    return g;
}
