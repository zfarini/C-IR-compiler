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
	return -1;
}

Register get_var_register(IR_Code *c, Node *var)
{
	//if (var->type != NODE_VAR_DECL)
	//	var = var->decl;
	assert(var && var->type == NODE_VAR_DECL);
	return c->vars_reg[var->var_index];
}

void set_var_register(IR_Code *c, Node *decl, Register r)
{
	assert(decl->type == NODE_VAR_DECL);
    
	decl->var_index = c->var_count++;
	c->vars_reg[decl->var_index] = r;
}

IR_Instruction *add_instruction(IR_Code *c, int op)
{
    IR_Instruction *e = &c->instructions[c->instruction_count];
    
    e->op = op;
    c->instruction_count++;
	e->node = c->curr_node;
    
    return e;
}

Function *find_function(IR_Code *c, Node *func)
{
	if (func->type == NODE_FUNC_CALL)
		func = func->decl;
	assert(func && func->type == NODE_FUNC_DEF);
	return &c->functions[func->func_index];
}

Register alloc_register(IR_Code *c, Type *t)
{
	Register r;
    
	r.type = t;
	r.i = c->curr_reg++;
	r.imm = 0;
	return r;
}

Register alloc_register_value(IR_Code *c, RValue value)
{
	(void)c;
    
	Register r;
    
	r.type = type_int;
	r.imm = 1;
	r.value = value;
	return r;
}

int alloc_size_aligned(IR_Code *c, int size)
{
	assert(size == 1 || size == 2 || size == 4 || size == 8);
    
	c->curr_func->stack_size = ((c->curr_func->stack_size + size - 1) / size) * size;
	int res = c->curr_func->stack_size;
	assert(res % size == 0);
	c->curr_func->stack_size += size;
	return res;
}

Register gen_ir(IR_Code *c, Node *node)
{
    Register reg = {0};
    
	reg.i = -1;
    
	c->curr_node = node;
    if (node->type == NODE_NUMBER)
    {
        IR_Instruction *e = add_instruction(c, OP_MOV);
        
		e->r0 = alloc_register(c, node->t);
		e->r1 = alloc_register_value(c, (RValue){.type = I32, .i32 = node->value});
		reg = e->r0;
    }
    else if (node->type == NODE_VAR)
    {
        reg = get_var_register(c, node->decl);
        
		IR_Instruction *e = add_instruction(c, OP_LOAD);
        
		e->r1 = alloc_register_value(c, (RValue){.type = I32, .i32 = node->decl->stack_offset});
		e->r2 = reg;
    }
	else if (node->type == NODE_DEREF)
	{
		Register r;
        
		if (node->left->type == NODE_VAR)
			r = get_var_register(c, node->left->decl);
		else
			r = gen_ir(c, node->left);
        
		IR_Instruction *e = add_instruction(c, OP_LOAD);
		e->r1 = r;
		e->r2 = alloc_register(c, node->t);
		reg = e->r2;
	}
	else if (node->type == NODE_ADDR)
	{
		if (node->left->type == NODE_VAR)
		{
			// this later get updated after we know the stack size
			//assert(0);
            
			IR_Instruction *e = add_instruction(c, OP_SUB);
			e->r0 = alloc_register(c, type_ulong);
			e->r1.type = type_ulong;
			e->r1.i = REG_SP;
			e->r2 = alloc_register_value(c, (RValue){.type = U64, .u64 = node->left->decl->stack_offset});
			e->r2.type = type_ulong;
            
			IR_Instruction *f = add_instruction(c, OP_CAST);
			f->r0 = alloc_register(c, node->t);
			f->r1 = e->r0;
            
			reg = f->r0;
		}
		else
		{
			assert(node->left->type == NODE_DEREF);
			reg = gen_ir(c, node->left->left);	
		}
	}
	else if (node->type == NODE_RETURN)
	{
		Register r = gen_ir(c, node->left);
        
		IR_Instruction *e = add_instruction(c, OP_MOV);
		e->r0.type = r.type;
		e->r0.i = REG_RT;
		e->r1 = r;
        
		add_instruction(c, OP_JMP)->label = c->curr_func->exit_label;
	}
	else if (node->type == '!')
	{
		assert(0);
        
		//int r = gen_ir(c, node->left);
        
		//IR_Instruction *e = add_instruction(c, OP_NOT);
		//e->r0 = c->curr_reg++;
		//e->r1 = r;
		//reg = e->r0;
	}
    else if (node->type == NODE_VARS_DECL)
    {
		Node *curr = node->next_decl;
        
		while (curr)
		{
			Register var_reg = alloc_register(c, curr->t);
            
			curr->stack_offset = alloc_size_aligned(c, curr->t->size);
            
        	set_var_register(c, curr, var_reg);
			
			if (curr->assign)
			{
				Register r1 = gen_ir(c, curr->assign);
                
				IR_Instruction *e = add_instruction(c, OP_MOV);
				e->r0 = var_reg;
				e->r1 = r1;
                
				e = add_instruction(c, OP_STORE);
				e->r1 = alloc_register_value(c, (RValue){.type = I32, .i32 = curr->stack_offset});
				e->r2 = var_reg;
			}
            
			curr = curr->next_decl;
		}
    }
    else if (node->type == NODE_BINOP && node->op == '=')
    {
        Register r1 = gen_ir(c, node->right);
        
		if (node->left->type == NODE_DEREF)
		{
			Register r;
            
			if (node->left->left->type == NODE_VAR)
				r = get_var_register(c, node->left->left->decl);
			else
				r = gen_ir(c, node->left->left);
            
			IR_Instruction *e = add_instruction(c, OP_STORE);
			e->r1 = r;
			e->r2 = r1;
			reg = r1;
		}
		else
		{
			assert(node->left->type == NODE_VAR);
			Register vr = get_var_register(c, node->left->decl);
            
        	IR_Instruction *e = add_instruction(c, OP_MOV);
        	e->r0 = vr;
        	e->r1 = r1;
            
			e = add_instruction(c, OP_STORE);
			e->r1 = alloc_register_value(c, (RValue){.type = I32, .i32 = node->left->decl->stack_offset});
			e->r2 = vr;
            
        	reg = vr;
		}
    }
    else if (node->type == NODE_BINOP)
    {
        Register r1 = gen_ir(c, node->left);
        
		int offset = alloc_size_aligned(c, r1.type->size);
		
		// if we have f(..) + f(..)
		// then we generate something like
		// t1 = f(...)
		// t2 = f(...)
		// the problem is that the second call will overwrite t1
		// so we have to save it in the stack
		IR_Instruction *e = add_instruction(c, OP_STORE);
		e->r1 = alloc_register_value(c, (RValue){.type = I32, .i32 = offset});
		e->r2 = r1;
        
        Register r2 = gen_ir(c, node->right);
        
		e = add_instruction(c, OP_LOAD);
		e->r1 = alloc_register_value(c, (RValue){.type = I32, .i32 = offset});
		e->r2 = r1;
        
        e = add_instruction(c, token_type_to_ir_op(node->op));
        e->r0 = alloc_register(c, node->t);
        e->r1 = r1;
        e->r2 = r2;
        
        reg = e->r0;
    }
    else if (node->type == NODE_FUNC_DEF)
    {
		Function *f = find_function(c, node);
        
        //	f->stack_size = node->arg_count * sizeof(int);
		f->first_instruction = c->instruction_count;
		c->curr_func = f;
		c->labels[f->label] = c->instruction_count;
		f->exit_label = c->label_count++;
		// allocate the stack
		// we don't save the rsp in something like rbp, we just fix all uses of the stack 
		// after we finish
		{
			IR_Instruction *e = add_instruction(c, OP_ADD);
			e->r0.i = REG_SP;
			e->r0.type = type_ulong;
			e->r1.i = REG_SP;
			e->r1.type = type_ulong;
			e->r2.imm = 1;
			e->r2.type = type_ulong;
			e->r2.value.type = U64;
            e->r2.type = type_ulong;
			// r2 will be updated later after we know the total stack size
		}
		Node *arg = node->first_arg;
		int	i = 0;
		while (arg)
		{
			Register var_reg = {.i = REG_ARG0 + i};
            
			set_var_register(c, arg, var_reg);
			arg->stack_offset = alloc_size_aligned(c, arg->t->size);
            //		arg->stack_offset = i * sizeof(int);
			IR_Instruction *e = add_instruction(c, OP_STORE);
			e->r1 = alloc_register_value(c, (RValue){.type = I32, .i32 = arg->stack_offset});
			e->r2 = var_reg;
            
			arg = arg->next_arg;
			i++;
		}
        
        gen_ir(c, node->body);
		// exit stuff
		c->labels[f->exit_label] = c->instruction_count;
		{
			IR_Instruction *e = add_instruction(c, OP_SUB);
			e->r0.i = REG_SP;
			e->r0.type = type_ulong;
			e->r1.i = REG_SP;
			e->r1.type = type_ulong;
			e->r2.imm = 1;
			e->r2.value.type = U64;
            e->r2.type = type_ulong;
			// r2 will be updated later
		}
        add_instruction(c, OP_RET);
		f->instruction_count = c->instruction_count - f->first_instruction;
    }
    else if (node->type == NODE_FUNC_CALL)
    {
		assert(0);
#if 0
		int	i;
		int call_arg_offset = c->curr_func->stack_size;
		int my_arg_offset = 0; // they are the first thing we alloc
        
		c->curr_func->stack_size += node->arg_count * sizeof(int);
		// gen function arguments and store them
		/*
			1 - generate each arguments and push it to the stack
				because we could have a recursive function that will overwrite that register
			2 - save my params a0 .. an
			3 - copy the args we saved in step 1 to a0, .. am (m = called function arg count)
			4 - make the call
			5 - reload my params from the stack
			6 - we save the return value for cases like f1() + f2()
				if we don't both of them will use the second return value
			I don't think we need step 2, because we know our params are in the stack
			but I will do it anyways because it might help us in optimizing
		*/
		// 1
		i = 0;
		Node	*arg = node->first_arg;
		while (arg)
		{
			int r = gen_ir(c, arg);
            
			IR_Instruction *e = add_instruction(c, OP_STORE);
			e->r1 = call_arg_offset + i * sizeof(int);
			e->r1_imm = 1;
			e->r2 = r;
            
			i++;
			arg = arg->next_arg;
		}
		// 2
		i = 0;
		while (i < c->curr_func->decl->arg_count)
		{
			IR_Instruction *e = add_instruction(c, OP_STORE);
			e->r1 = my_arg_offset + i * sizeof(int);
			e->r1_imm = 1;
			e->r2 = REG_ARG0 + i;
			i++;
		}
		// 3
		i = 0;
		while (i < node->arg_count)
		{
			IR_Instruction *e = add_instruction(c, OP_LOAD);
			e->r1 = call_arg_offset + i * sizeof(int);
			e->r1_imm = 1;
			e->r2 = REG_ARG0 + i;
			i++;
		}
		// 4
		Function *f = find_function(c, node);
        IR_Instruction *e = add_instruction(c, OP_CALL);
		e->label = f->label;
		// 5
		i = 0;
		while (i < c->curr_func->decl->arg_count)
		{
			IR_Instruction *e = add_instruction(c, OP_LOAD);
			e->r1 = my_arg_offset + i * sizeof(int);
			e->r1_imm = 1;
			e->r2 = REG_ARG0 + i;
			i++;
		}
		// 6
		e = add_instruction(c, OP_MOV);
		e->r0 = c->curr_reg++;
		e->r1 = REG_RT;
		reg = e->r0;
#endif
    }
#if 0
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
        int exit_label;
		int else_label = -1;
        
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
    
#endif
    else if (node->type == NODE_BLOCK)
    {
        Node *curr = node->first_stmt;
        
        while (curr)
        {
            gen_ir(c, curr);
            curr = curr->next_stmt;
        }
    }
	else if (node->type == NODE_PRINT)
    {
        Register r = gen_ir(c, node->left);
        
        IR_Instruction *e = add_instruction(c, OP_PRINT);
        e->r1 = r;
    }
	else if (node->type == NODE_ASSERT)
	{
        Register r = gen_ir(c, node->left);
        
        IR_Instruction *e = add_instruction(c, OP_ASSERT);
        e->r1 = r;
	}
    else if (node->type == NODE_CAST)
	{
        Register r = gen_ir(c, node->left);
        
		IR_Instruction *e = add_instruction(c, OP_CAST);
		e->r0 = alloc_register(c, node->t);
		e->r1 = r;
		reg = e->r0;
	}
    else if (node->type == NODE_NOT)
    {
        Register r = gen_ir(c, node->left);
        
        IR_Instruction *e = add_instruction(c, OP_NOT);
        e->r0 = alloc_register(c, node->t);
        e->r1 = r;
        reg = e->r0;
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
    c->labels = calloc(sizeof(*c->labels), 1024);
	c->functions = calloc(sizeof(*c->functions), 128);
	c->vars_reg = calloc(sizeof(*c->vars_reg), 1024);
    
	Node *curr = node;
	// we give the first n labels to functions so that
	// if a function is used before its definition we just point to the label
	while (curr)
	{
		int index = c->function_count++;
        
		Function *f = &c->functions[index];
        
		f->name = curr->token->name;
		f->label = c->label_count++;
		f->decl = curr;
		f->decl->func_index = index;
        
		curr = curr->next_func;
	}
    
	curr = node;
	int	i = 0;
    while (curr)
    {
        gen_ir(c, curr);
		// All Stores / Loads assume that they use the base pointer (rbp) instead of rsp
		// so we fix them after we know the total stack size
		for (int j = c->functions[i].first_instruction; j < 
             c->functions[i].first_instruction + c->functions[i].instruction_count;
             j++)
		{
			IR_Instruction *e = &c->instructions[j];
            
			if (e->op == OP_SUB && !e->r1.imm && e->r1.i == REG_SP)
			{
				assert(e->r2.value.type == U64);
				e->r2.value.u64 = c->functions[i].stack_size - (int)e->r2.value.u64;
			}
			else if ((e->op == OP_LOAD || e->op == OP_STORE) && e->r1.imm)
			{
				assert(e->r1.value.type == I32);
				e->r1.value.i32 = c->functions[i].stack_size - e->r1.value.i32;
			}
		}
		// update the sub / add instructions for allocating the stack
		// which currently are the first instruction and the one before the last one (before "ret")
		assert(c->instructions[c->labels[i]].r0.i == REG_SP &&
               c->instructions[c->labels[i] + c->functions[i].instruction_count - 2].r0.i == REG_SP);
		c->instructions[c->labels[i]].r2.value.i32 = c->functions[i].stack_size;
		c->instructions[c->labels[i]
                        + c->functions[i].instruction_count - 2].r2.value.i32 = c->functions[i].stack_size;
        
		curr = curr->next_func;
		i++;
    }
    
    return c;
}

