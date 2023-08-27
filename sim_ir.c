

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
    
	r.type = -1;
    
#define OP(kind, p) if (op == kind) {\
r.type = t; \
if	  (t == U64) r.u64 = r1.u64 p r2.u64;\
else if (t == U32) r.u32 = r1.u32 p r2.u32;\
else if (t == U16) r.u16 = r1.u16 p r2.u16;\
else if (t == U8)  r.u8  = r1.u8  p r2.u8;\
else if (t == I64) r.i64 = r1.i64 p r2.i64;\
else if (t == I32) r.i32 = r1.i32 p r2.i32;\
else if (t == I16) r.i16 = r1.i16 p r2.i16;\
else if (t == I8)  r.i8  = r1.i8  p r2.i8;\
}
    OP(OP_ADD, +);
    OP(OP_SUB, -);
    OP(OP_MUL, *);
    OP(OP_DIV, /);
    OP(OP_MOD, %);
    OP(OP_LESS, <);
    OP(OP_GREATER, >);
    OP(OP_EQUAL, ==);
    OP(OP_LESS_OR_EQUAL, <=);
    OP(OP_GREATER_OR_EQUAL, >=);
    OP(OP_NOT_EQUAL, !=);
    assert(r.type != -1);
    
    return r;
}

int	sim_ir_code(IR_Code *c)
{
    RValue regs[1024] = {0};
    uint64_t ip = 0;
    
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
		//printf("at %d\n", ip);
        IR_Instruction *e = &c->instructions[ip];
        
		
		RValue r1_value = e->r1.imm ? e->r1.value : regs[e->r1.i];
		RValue r2_value = e->r2.imm ? e->r2.value : regs[e->r2.i];
        
        
        //int r1_value = (e->r1.imm ? e->r1.value : regs[e->r1]);
        //int r2_value = (e->r2.imm ? e->r2.value : regs[e->r2]);
        //	int addr = (e->r1.imm ? regs[REG_SP] - e->r1 : regs[e->r1]);
        
        if (e->op < OP_BINARY)
		{
            if (e->r1.type->t == PTR)
            {
                assert(!"add pointer arithmetic");
            }
            //else
            //assert(e->r1.type == e->r2.type && e->r0.type == e->r1.type);
            regs[e->r0.i] = eval_op(e->op, r1_value, r2_value);
            regs[e->r0.i].type = get_rvalue_type_from_ctype(e->r0.type);
		}
        else if (e->op == OP_MOV)
		{
			assert(!e->r0.imm);
			//assert(e->r0.type == e->r1.type);
            regs[e->r0.i] = r1_value;
		}
        else if (e->op == OP_NOT)
        {
            regs[e->r0.i].type = I32;
            if      (r1_value.type == U64) regs[e->r0.i].i32 = !r1_value.u64;
            else if (r1_value.type == U16) regs[e->r0.i].i32 = !r1_value.u32;
            else if (r1_value.type == U32) regs[e->r0.i].i32 = !r1_value.u16;
            else if (r1_value.type == U8)  regs[e->r0.i].i32 = !r1_value.u8;
            else if (r1_value.type == I64) regs[e->r0.i].i32 = !r1_value.i64;
            else if (r1_value.type == I32) regs[e->r0.i].i32 = !r1_value.i32;
            else if (r1_value.type == I16) regs[e->r0.i].i32 = !r1_value.i16;
            else if (r1_value.type == I8)  regs[e->r0.i].i32 = !r1_value.i8;
        }
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
            if (r1_value.type == U64)
                printf("%"PRIu64, r1_value.u64);
            else if (r1_value.type == U32)
                printf("%"PRIu32, r1_value.u32);
            else if (r1_value.type == U16)
                printf("%"PRIu16, r1_value.u16);
            else if (r1_value.type == U8)
                printf("%"PRIu8, r1_value.u8);
            else if (r1_value.type == I64)
                printf("%"PRId64, r1_value.i64);
			else if (r1_value.type == I32)
                printf("%"PRId32, r1_value.i32);
            else if (r1_value.type == I16)
                printf("%"PRId16, r1_value.i16);
            else if (r1_value.type == I8)
                printf("%"PRId8, r1_value.i8);
            else
                assert(0);
            printf("\n");
            //printf("line:%d: 0x%"PRIx64"\n", e->node->token->, r1_value.u64);
            
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
            if ((void *)regs[REG_SP].u64 == stack)
                break ; // this is not quit right?
            
            regs[REG_SP].u64 -= sizeof(uint64_t);
            ip = *(uint64_t *)(regs[REG_SP].u64);
            continue ;
        }
        else if (e->op == OP_LOAD)
		{
            assert(!e->r2.imm);
			
			void *s = (void *)((e->r1.imm ? regs[REG_SP].u64 - (uint64_t)e->r1.value.u64 : r1_value.u64));
            
            int t = get_rvalue_type_from_ctype(e->r2.type);
			if 	 (t == U64) regs[e->r2.i].u64 = *(uint64_t *)s;
			else if (t == U32) regs[e->r2.i].u32 = *(uint32_t *)s;
			else if (t == U16) regs[e->r2.i].u16 = *(uint16_t *)s;
			else if (t == U8)  regs[e->r2.i].u8  = *(int8_t   *)s;
			else if (t == I64) regs[e->r2.i].i64 = *(int64_t  *)s;
			else if (t == I32) regs[e->r2.i].i32 = *(int32_t  *)s;
			else if (t == I16) regs[e->r2.i].i16 = *(int16_t  *)s;
			else if (t == I8)  regs[e->r2.i].i8  = *(int8_t   *)s;
            
            regs[e->r2.i].type = t;
        }
		else if (e->op == OP_STORE)
		{
			assert(e->r1.imm || r1_value.type == U64);
			void *s = (void *)((e->r1.imm ? regs[REG_SP].u64 - (uint64_t)e->r1.value.u64 : r1_value.u64));
            
			int t = get_rvalue_type_from_ctype(e->r2.type);
			if 		(t == U64) *(uint64_t *)s = r2_value.u64;
			else if (t == U32) *(uint32_t *)s = r2_value.u32;
			else if (t == U16) *(uint16_t *)s = r2_value.u16;
			else if (t == U8)  *(uint8_t  *)s = r2_value.u8;
			else if (t == I64) *(int64_t  *)s = r2_value.i64;
			else if (t == I32) *(int32_t  *)s = r2_value.i32;
			else if (t == I16) *(int16_t  *)s = r2_value.i16;
			else if (t == I8)  *(int8_t   *)s = r2_value.i8;
        }
		else if (e->op == OP_CAST)
		{
            int t0 = get_rvalue_type_from_ctype(e->r0.type);
            int t1 = get_rvalue_type_from_ctype(e->r1.type);
            // TODO: simplify this shit
#define U(d1, d2, U2, u2) [U##d1 * RVALUE_COUNT + U2##d2] = {.u##d1 = (uint##d1##_t)r1_value.##u2##d2}
#define I(d1, d2, U2, u2) [I##d1 * RVALUE_COUNT + U2##d2] = {.i##d1 = (int##d1##_t)r1_value.##u2##d2}
            
#define ALL_U(d) U(d, 64, U, u), U(d, 32, U, u), U(d, 16, U, u), U(d, 8, U, u), \
U(d, 64, I, i), U(d, 32, I, i), U(d, 16, I, i), U(d, 8, I, i)
#define ALL_I(d) I(d, 64, U, u), I(d, 32, U, u), I(d, 16, U, u), I(d, 8, U, u), \
I(d, 64, I, i), I(d, 32, I, i), I(d, 16, I, i), I(d, 8, I, i)
            
#define ALL(d) ALL_U(d), ALL_I(d)
            
            RValue cast_table[] = {
                ALL(64),
                ALL(32),
                ALL(16),
                ALL(8)
            };
            
            regs[e->r0.i] = cast_table[t0 * RVALUE_COUNT + t1];
            
            regs[e->r0.i].type = t0;
#undef U
#undef I
#undef ALL
#undef ALL_U
#undef ALL_I
        }
		else
            assert(0);
        ip++;
    }
	free(stack);
	return err;
}
