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

int	*get_int_from_stack(uint8_t *stack, int offset)
{
	assert(offset >= 0 && offset % 4 == 0);
	return (int *)(stack + offset);
}

int	sim_ir_code(IR_Code *c)
{
    int regs[1024] = {0};
    int ip = 0;

    printf("\033[1;32msim output:\033[0m\n");


	Function *main = 0;
	for (int j = 0; j < c->function_count; j++)
	{
		if (!strcmp(c->functions[j].name, "main"))
		{
			main = &c->functions[j];
			break ;
		}
	}

    if (!main)
    {
        printf("SIM ERROR: main is not defined\n");
        return 1;
    }

	int err = 0;

    ip = main->first_instruction;
	int stack_max = 4096;
	uint8_t *stack = malloc(stack_max * sizeof(int));
	memset(stack, 0xcc, stack_max * sizeof(int));

    while (1)
    {
        IR_Instruction *e = &c->instructions[ip];
        
        int r1_value = (e->r1_imm ? e->r1 : regs[e->r1]);
        int r2_value = (e->r2_imm ? e->r2 : regs[e->r2]);
		int addr = (e->r1_imm ? regs[REG_SP] - e->r1 : regs[e->r1]);

        if (e->op < OP_BINARY)
            regs[e->r0] = eval_op(e->op, r1_value, r2_value);
        else if (e->op == OP_MOV)
            regs[e->r0] = r1_value;
		else if (e->op == OP_NOT)
			regs[e->r0] = !r1_value;
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
		else if (e->op == OP_ASSERT)
		{
			if (!r1_value)
			{
				printf("SIM ERROR: line %d: assert failed (value = %d)\n", (!e->node || !e->node->token ? -1 : e->node->token->line), r1_value);
				err = 1;
				break ;
			}
		}
        else if (e->op == OP_CALL)
        {
			if (regs[REG_SP] == stack_max)
			{
				printf("SIM ERROR: stack overflow\n");
				err = 1;
				break ;
			}
			*get_int_from_stack(stack, regs[REG_SP]) = ip + 1;
			regs[REG_SP] += sizeof(int);
            ip = c->labels[e->r0];
            continue ;
        }
        else if (e->op == OP_RET)
        {
            if (!regs[REG_SP])
                break ;

			regs[REG_SP] -= sizeof(int);
			ip = *get_int_from_stack(stack, regs[REG_SP]);
            continue ;
        }
        else if (e->op == OP_LOAD)
		{
			assert(!e->r2_imm);
			regs[e->r2] = *get_int_from_stack(stack, addr);
		}
		else if (e->op == OP_STORE)
		{
			*get_int_from_stack(stack, addr) = r2_value;
		}
		else
            assert(0);
        ip++;
    }
	free(stack);
	return err;
}
