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
	e->node = c->curr_node;

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

	c->curr_node = node;
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

		IR_Instruction *e = add_instruction(c, OP_LOAD);
		e->r1 = node->decl->stack_offset;
		e->r1_imm = 1;
		e->r2 = reg;

        assert(reg >= 0);
    }
	else if (node->type == NODE_DEREF)
	{
		int r;

		if (node->left->type == NODE_VAR)
			r = get_var_register(c, node->left->decl);
		else
			r = gen_ir(c, node->left);
		IR_Instruction *e = add_instruction(c, OP_LOAD);
		e->r1 = r;//
		e->r2 = c->curr_reg++;

		reg = e->r2;
	}
	else if (node->type == NODE_ADDR)
	{
		assert(node->left->type == NODE_VAR ||
				node->left->type == NODE_DEREF);

		if (node->left->type == NODE_VAR)
		{
			// this later get updated after we know the stack size
			IR_Instruction *e = add_instruction(c, OP_SUB);
			e->r0 = c->curr_reg++;
			e->r1 = REG_SP;
			e->r2 = node->left->decl->stack_offset;;
			e->r2_imm = 1;

			reg = e->r0;
		}
		else
			reg = gen_ir(c, node->left->left);	
	}
	else if (node->type == NODE_RETURN)
	{
		int r = gen_ir(c, node->left);
		IR_Instruction *e = add_instruction(c, OP_MOV);
		e->r0 = REG_RT;
		e->r1 = r;
		add_instruction(c, OP_RET);
	}
    else if (node->type == NODE_FUNC_DEF)
    {
		c->vars_reg[0].decl = 0; // check

		Function *f = find_function(c, node->token->name);

		f->stack_size = node->arg_count * sizeof(int);
		

		c->curr_func = f;
		assert(f);

		c->labels[f->label] = c->instruction_count;
		f->first_instruction = c->instruction_count;

		{
			IR_Instruction *e = add_instruction(c, OP_ADD);
			e->r0 = REG_SP;
			e->r1 = REG_SP;
			e->r2_imm = 1;
			// e->r2 will be updated later
		}

		Node *arg = node->first_arg;
		int	i = 0;
		while (arg)
		{
			set_var_register(c, arg, REG_ARG0 + i);
			arg->stack_offset = i * sizeof(int);
			IR_Instruction *e = add_instruction(c, OP_STORE);
			e->r1 = arg->stack_offset;
			e->r1_imm = 1;
			e->r2 = REG_ARG0 + i;

			arg = arg->next_arg;
			i++;
		}


        gen_ir(c, node->body);
		// add return label
		{
			int ret_label = c->label_count++;
			c->labels[ret_label] = c->instruction_count;
			for (int j = f->first_instruction; j < c->instruction_count; j++)
				if (c->instructions[j].op == OP_RET)
				{
					c->instructions[j].op = OP_JMP;
					c->instructions[j].r0 = ret_label;
				}
		}

		{
			IR_Instruction *e = add_instruction(c, OP_SUB);
			e->r0 = REG_SP;
			e->r1 = REG_SP;
			e->r2_imm = 1;
		}
        add_instruction(c, OP_RET);
		f->instruction_count = c->instruction_count - f->first_instruction;

    }
    else if (node->type == NODE_FUNC_CALL)
    {
		Node	*arg = node->first_arg;
		int		i = 0;	
		int		regs[16];

		// execute args
		// [arg1, arg2 ... argn]
		//

		// TODO: check if this correct always
		// also you don't need to save the result unless you will be called in the arguments
		int call_arg_offset = c->curr_func->stack_size;
		int my_arg_offset = c->curr_func->stack_size + node->arg_count * sizeof(int);

		c->curr_func->stack_size += (node->arg_count + c->curr_func->decl->arg_count) * sizeof(int);

		// gen function arguments and store them
		i = 0;
		while (arg)
		{
			regs[i] = gen_ir(c, arg);
			assert(regs[i] != -1);

			IR_Instruction *e = add_instruction(c, OP_STORE);
			e->r1 = call_arg_offset + i * sizeof(int);
			e->r1_imm = 1;
			e->r2 = regs[i];

			i++;
			arg = arg->next_arg;
		}

		// save my arguments before call
		i = 0;
		while (i < c->curr_func->decl->arg_count)
		{
			IR_Instruction *e = add_instruction(c, OP_STORE);
			e->r1 = my_arg_offset + i * sizeof(int);
			e->r1_imm = 1;
			e->r2 = REG_ARG0 + i;
			i++;
		}

		// load function arguments
		i = 0;
		while (i < node->arg_count)
		{
			IR_Instruction *e = add_instruction(c, OP_LOAD);
			e->r1 = call_arg_offset + i * sizeof(int);
			e->r1_imm = 1;
			e->r2 = REG_ARG0 + i;
			i++;
		}

        IR_Instruction *e = add_instruction(c, OP_CALL);

	
		Function *f = find_function(c, node->token->name);

		// reload my arguments
		i = 0;
		while (i < c->curr_func->decl->arg_count)
		{
			IR_Instruction *e = add_instruction(c, OP_LOAD);
			e->r1 = my_arg_offset + i * sizeof(int);
			e->r1_imm = 1;
			e->r2 = REG_ARG0 + i;
			i++;
		}

		if (!f) // shouldn't be here
			error_token(node->token, "undeclared function");

		e->r0 = f->label;

		e = add_instruction(c, OP_MOV);
		e->r0 = c->curr_reg++;
		e->r1 = REG_RT;

		reg = e->r0;
    }
    else if (node->type == NODE_PRINT)
    {
        int r = gen_ir(c, node->left);

        IR_Instruction *e = add_instruction(c, OP_PRINT);
        e->r1 = r;
    }
	else if (node->type == NODE_ASSERT)
	{
        int r = gen_ir(c, node->left);

        IR_Instruction *e = add_instruction(c, OP_ASSERT);
        e->r1 = r;
	}
    else if (node->type == NODE_BINOP && node->op == '=')
    {
        int r1 = gen_ir(c, node->right);

		if (node->left->type == NODE_DEREF)
		{
			int r;

			if (node->left->left->type == NODE_VAR)
				r = get_var_register(c, node->left->left->decl);
			else
				r = gen_ir(c, node->left->left);
			//assert(node->left->left->type == NODE_VAR);

			IR_Instruction *e = add_instruction(c, OP_STORE);
			e->r1 = r;
			e->r2 = r1;

			reg = r1;
		}
		else
		{
			assert(node->left->type == NODE_VAR);
			int vr = get_var_register(c, node->left->decl);

        	IR_Instruction *e = add_instruction(c, OP_MOV);
        	e->r0 = vr;
        	e->r1 = r1;

			e = add_instruction(c, OP_STORE);
			e->r1 = node->left->decl->stack_offset;;
			e->r1_imm = 1;
			e->r2 = vr;

        	reg = vr;
		}

    }
    else if (node->type == NODE_BINOP)
    {
        int r1 = gen_ir(c, node->left);

		int offset = c->curr_func->stack_size;
		c->curr_func->stack_size += sizeof(int);

		IR_Instruction *e = add_instruction(c, OP_STORE);
		e->r1 = offset;
		e->r1_imm = 1;
		e->r2 = r1;

        int r2 = gen_ir(c, node->right);

		e = add_instruction(c, OP_LOAD);
		e->r1 = offset;
		e->r1_imm = 1;
		e->r2 = r1;

        e = add_instruction(c, token_type_to_ir_op(node->op));
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
	else if (node->type == '!')
	{
		int r = gen_ir(c, node->left);

		IR_Instruction *e = add_instruction(c, OP_NOT);
		e->r0 = c->curr_reg++;
		e->r1 = r;
		reg = e->r0;
	}
    else if (node->type == NODE_VAR_DECL)
    {
		node->stack_offset = c->curr_func->stack_size;
		c->curr_func->stack_size += sizeof(int);
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
	
	c->reserved_reg = 0;
	{
		Node *curr = node;
		while (curr)
		{
			if (curr->arg_count > c->reserved_reg)
				c->reserved_reg = curr->arg_count;
			curr = curr->next_func;
		}
	}
	c->reserved_reg += REG_COUNT;

	c->curr_reg = c->reserved_reg;
    c->instructions = calloc(sizeof(*c->instructions), 16384);
    c->labels = calloc(sizeof(*c->labels), 256);
	c->functions = calloc(sizeof(*c->functions), 64);

	Node *curr = node;
	while (curr)
	{
		Function *f = &c->functions[c->function_count++];

		f->name = curr->token->name;
		f->label = c->label_count++;
		f->decl = curr;

		curr = curr->next_func;
	}

	curr = node;
	int	i = 0;
    while (curr)
    {
        gen_ir(c, curr);

		// update stack size after computing intermediate stuff


		for (int j = c->functions[i].first_instruction; j < 
					 c->functions[i].first_instruction + c->functions[i].instruction_count;
					 j++)
		{
			IR_Instruction *e = &c->instructions[j];

			if (e->op == OP_SUB && !e->r1_imm && e->r1 == REG_SP)
				e->r2 = c->functions[i].stack_size - e->r2;
			else if ((e->op == OP_LOAD || e->op == OP_STORE) && e->r1_imm)
				e->r1 = c->functions[i].stack_size - e->r1;
		}

		assert(c->instructions[c->labels[i]].r0 == REG_SP &&
				c->instructions[c->labels[i] + c->functions[i].instruction_count - 2].r0 == REG_SP);

		c->instructions[c->labels[i]].r2 = c->functions[i].stack_size;
		c->instructions[c->labels[i]
			+ c->functions[i].instruction_count - 2].r2 = c->functions[i].stack_size;

		curr = curr->next_func;
		i++;
    }

    return c;
}


