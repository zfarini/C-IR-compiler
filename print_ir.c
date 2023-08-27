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

void int_to_str(char *s, int x)
{
	assert(x >= 0);
	int n = x;
	int len = (x == 0);
	while (n)
	{
		len++;
		n /= 10;
	}
	int i = len - 1;
	while (i >= 0)
	{
		s[i] = x % 10 + '0';
		x /= 10;
		i--;
	}
}

void get_reg_str(Register r, char *s, int reserved)
{
	if (r.imm)
	{
		if (r.value.type == I32)
			sprintf(s, "%d", r.value.i32);
		else if (r.value.type == U64)
			sprintf(s, "%"PRIu64, r.value.u64);
		else
			assert(0);
	}
	else if (!r.i) memcpy(s, "rax", 3);
	else if (r.i == 1) memcpy(s, "rsp", 3);
	else if (r.i < REG_COUNT)
		assert(0);
	else if (r.i >= REG_COUNT && r.i < reserved)
	{
		s[0] = 'a';
		int_to_str(s + 1, r.i - REG_COUNT);
	}
	else
	{
		s[0] = 't';
		int_to_str(s + 1, r.i);
	}
}

void print_instruction(IR_Code *c, IR_Instruction *e, int in_block)
{
	char r0[64] = {0};
	char r1[64] = {0};
	char r2[64] = {0};
    
	get_reg_str(e->r0, r0, c->reserved_reg);
	get_reg_str(e->r1, r1, c->reserved_reg);
	get_reg_str(e->r2, r2, c->reserved_reg);
    
    if (e->op == OP_JMP)
        printf("jmp %s%d", (in_block ? "B" : "L"), e->label);
    else if (e->op == OP_JMPZ)
        printf("jmpz %s%d %s", (in_block ? "B" : "L"), e->label, r1);
    else if (e->op == OP_PRINT)
        printf("print %s", r1);
	else if (e->op == OP_ASSERT)
		printf("assert %s", r1);
    else if (e->op == OP_CALL)
        printf("call %s", c->functions[e->label].name);
    else if (e->op == OP_RET)
        printf("ret");
	else if (e->op == OP_NOT)
		printf("%s = !%s", r0, r1);
	else if (e->op == OP_STORE)
	{
		if (e->r1.imm)
			printf("[rsp - %"PRIu64"]", e->r1.value.u64);
		else
			printf("[%s]", r1);
		printf(" = %s", r2);
	}
	else if (e->op == OP_LOAD)
	{
		printf("%s = ", r2);
		if (e->r1.imm)
			printf("[rsp - %"PRIu64"]", e->r1.value.u64);
		else
			printf("[%s]", r1);
	}
	else if (e->op == OP_MOV)
		printf("%s = %s", r0, r1);
    else if (e->op < OP_BINARY)
        printf("%s = %s %s %s", r0, r1, get_ir_op_str(e->op), r2);
	else if (e->op == OP_CAST)
	{
		printf("%s = (%s)%s", r0, get_type_str(e->r0.type), r1);
	}
	else
		assert(0);
    printf("\n");
}

void print_ir_code(IR_Code *c)
{
    printf("\033[1;32mgenerated ir:\033[0m (%d instructions, %d reserved registers)\n", c->instruction_count, c->reserved_reg);
    
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
        
        printf("%-16d", i);
        print_instruction(c, e, 0);
    }
    
    for (int j = 0; j < c->label_count; j++)
    {
        if (c->labels[j] == c->instruction_count)
            printf("\tL%d:\n", j);
        assert(c->labels[j] <= c->instruction_count);
    }
}
