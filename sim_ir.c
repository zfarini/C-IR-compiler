

//int	*get_int_from_stack(uint8_t *stack, int offset)
//{
//	assert(offset >= 0 && offset % 4 == 0);
//	return (int *)(stack + offset);
//}

int get_rvalue_type_from_ctype(Type *t)
{
	int u = t->is_unsigned;

	if		(t->size == 8) return u ? U64 : I64;
	else if (t->size == 4) return u ? U32 : I32;
	else if (t->size == 2) return u ? U16 : I16;
	else if (t->size == 1) return u ? U8  : I8;
	assert(0);
	return 0;
}

RValue eval_op(int op, RValue r1, RValue r2)
{
	assert(r1.type == r2.type);
	int t = r1.type;

	RValue r;

	r.type = t;

    if (op == OP_ADD)
	{
		if		(t == U64) r.u64 = r1.u64 + r2.u64;
		else if (t == U32) r.u32 = r1.u32 + r2.u32;
		else if (t == U16) r.u16 = r1.u16 + r2.u16;
		else if (t == U8)  r.u8  = r1.u8  + r2.u8;
		else if (t == I64) r.i64 = r1.i64 + r2.i64;
		else if (t == I32) r.i32 = r1.i32 + r2.i32;
		else if (t == I16) r.i16 = r1.i16 + r2.i16;
		else if (t == I8)  r.i8  = r1.i8  + r2.i8;
	}
    else if (op == OP_SUB)
	{
		if		(t == U64) r.u64 = r1.u64 - r2.u64;
		else if (t == U32) r.u32 = r1.u32 - r2.u32;
		else if (t == U16) r.u16 = r1.u16 - r2.u16;
		else if (t == U8)  r.u8  = r1.u8  - r2.u8;
		else if (t == I64) r.i64 = r1.i64 - r2.i64;
		else if (t == I32) r.i32 = r1.i32 - r2.i32;
		else if (t == I16) r.i16 = r1.i16 - r2.i16;
		else if (t == I8)  r.i8  = r1.i8  - r2.i8;
	}
#if 0
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
#endif
    else
        assert(0);
    return r;
}

int	sim_ir_code(IR_Code *c)
{
    RValue regs[1024] = {0};
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

	regs[REG_SP].u64 = (uint64_t)stack;
	regs[REG_SP].type = U64;

    while (1)
    {
		printf("at %d\n", ip);
        IR_Instruction *e = &c->instructions[ip];

		printf("at %d %d %d %d\n", ip, e->r0.type->size,
				e->r1.type->size,
				e->r2.type->size);
        
		RValue r1_value = e->r1.imm ? e->r1.value : regs[e->r1.i];
		RValue r2_value = e->r2.imm ? e->r2.value : regs[e->r2.i];


        //int r1_value = (e->r1.imm ? e->r1.value : regs[e->r1]);
        //int r2_value = (e->r2.imm ? e->r2.value : regs[e->r2]);
	//	int addr = (e->r1.imm ? regs[REG_SP] - e->r1 : regs[e->r1]);

        if (e->op < OP_BINARY)
		{
			assert(e->r1.type == e->r2.type && e->r0.type == e->r1.type);
            regs[e->r0.i] = eval_op(e->op, r1_value, r2_value);
		}
        else if (e->op == OP_MOV)
		{
			assert(!e->r0.imm);
			assert(e->r0.type == e->r1.type);
            regs[e->r0.i] = r1_value;
		}
	//	else if (e->op == OP_NOT)
	//		regs[e->r0] = !r1_value;
        else if (e->op == OP_JMP)
        {
            ip = c->labels[e->label];
            continue ;
        }
        else if (e->op == OP_JMPZ)
        {
          //  if (!regs[e->r1])
          //  {
          //      ip = c->labels[e->label];
          //      continue ;
          //  }
        }
        else if (e->op == OP_PRINT)
        {
			printf("line:%d: 0x%"PRIx64"\n", e->node->token->line, r1_value.u64);
        }
		else if (e->op == OP_ASSERT)
		{
		//	if (!r1_value)
		//	{
		//		printf("SIM ERROR: line %d: assert failed (value = %d)\n", (!e->node || !e->node->token ? -1 : e->node->token->line), r1_value);
		//		err = 1;
		//		break ;
		//	}
		}
        else if (e->op == OP_CALL)
        {
		//	if (regs[REG_SP] == stack_max)
		//	{
		//		printf("SIM ERROR: stack overflow\n");
		//		err = 1;
		//		break ;
		//	}
		//	*get_int_from_stack(stack, regs[REG_SP]) = ip + 1;
		//	regs[REG_SP] += sizeof(int);
        //    ip = c->labels[e->r0];
        //    continue ;
        }
        else if (e->op == OP_RET)
        {
			break ;
           // if (!regs[REG_SP])
           //     break ;

		   // regs[REG_SP] -= sizeof(int);
		   // ip = *get_int_from_stack(stack, regs[REG_SP]);
           // continue ;
        }
        else if (e->op == OP_LOAD)
		{
			assert(!e->r2.imm);
			
			//SValue v;
			//if (e->r1.i == )
		//	*get_int_from_stack(stack, addr);

			//regs[e->r2.i] = 
		}
		else if (e->op == OP_STORE)
		{
			assert(e->r1.imm || r1_value.type == U64);
			void *s = (void *)((e->r1.imm ? regs[REG_SP].u64 - (uint64_t)e->r1.value.u64 : r1_value.u64));
	//		void *s = (void *)stack + addr;

			int t = get_rvalue_type_from_ctype(e->r2.type);
			if 		(t == U64) *(uint64_t *)s = r2_value.u64;
			else if (t == U32) *(uint32_t *)s = r2_value.u32;
			else if (t == U16) *(uint16_t *)s = r2_value.u16;
			else if (t == U8)  *(uint8_t  *)s = r2_value.u8;
			else if (t == I64) *(int64_t  *)s = r2_value.i64;
			else if (t == I32) *(int32_t  *)s = r2_value.i32;
			else if (t == I16) *(int16_t  *)s = r2_value.i16;
			else if (t == I8)  *(int8_t   *)s = r2_value.i8;
		//	int addr = (e->r1.imm ? regs[REG_SP] - e->r1 : regs[e->r1]);
			//*get_int_from_stack(stack, addr) = r2_value;
		}
		else if (e->op == OP_CAST)
		{
		}
		else
            assert(0);
        ip++;
    }
	free(stack);
	return err;
}
