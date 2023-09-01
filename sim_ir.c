int get_rvalue_type_from_ctype(Type *t)
{
	int u = t->is_unsigned;
    
    //if      (t->t == PTR) return RV_PTR;
    if (t->t == FLOAT) return RV_F32;
    else if (t->t == DOUBLE) return RV_F64;
	else if (t->size == 8) return u ? RV_U64 : RV_I64;
	else if (t->size == 4) return u ? RV_U32 : RV_I32;
	else if (t->size == 2) return u ? RV_U16 : RV_I16;
	else if (t->size == 1) return u ? RV_U8  : RV_I8;
	printf("type: %s\n", get_type_str(t));
	assert(0);
	return 0;
}

RValue convert_rvalue_type(RValue r, int new_type)
{
    RValue result;
    result.type = new_type;

    switch (new_type)
    {
        case RV_U64:
            result.u64 = (uint64_t)r.i64;
            break;
        case RV_U32:
            result.u32 = (uint32_t)r.i64;
            break;
        case RV_U16:
            result.u16 = (uint16_t)r.i64;
            break;
        case RV_U8:
            result.u8 = (uint8_t)r.i64;
            break;
        case RV_I64:
            result.i64 = r.i64;
            break;
        case RV_I32:
            result.i32 = r.i32;
            break;
        case RV_I16:
            result.i16 = r.i16;
            break;
        case RV_I8:
            result.i8 = r.i8;
            break;
        case RV_PTR:
            // Handle pointer conversion
            break;
        case RV_F32:
            switch (r.type) {
                case RV_I64:
                case RV_I32:
                case RV_I16:
                case RV_I8:
                    result.f32 = (float)r.i64;
                    break;
                case RV_U64:
                case RV_U32:
                case RV_U16:
                case RV_U8:
                    result.f32 = (float)r.u64;
                    break;
                case RV_F64:
                    result.f32 = (float)r.f64;
                    break;
                default:
                    // Handle other cases
                    break;
            }
            break;
        case RV_F64:
            switch (r.type) {
                case RV_I64:
                case RV_I32:
                case RV_I16:
                case RV_I8:
                    result.f64 = (double)r.i64;
                    break;
                case RV_U64:
                case RV_U32:
                case RV_U16:
                case RV_U8:
                    result.f64 = (double)r.u64;
                    break;
                case RV_F32:
                    result.f64 = (double)r.f32;
                    break;
                default:
                    // Handle other cases
                    break;
            }
            break;
    }

    return result;
}



RValue eval_op(int op, RValue r1, RValue r2)
{
	assert(r1.type == r2.type);
	int t = r1.type;
    
	RValue r;
    
    #if 0
	r.type = r1.type;
    
    if (op == OP_EQUAL)
    {
        r.type = RV_I32;
        r.i32 = r1.u64 == r2.u64;
    }
    else if (op == OP_NOT_EQUAL)
    {
        r.type = RV_I32;
        r.i32 = r1.u64 == r2.u64;
    }
    else if (1) // / % <= >= < > (just do every one)
    {
    }

    else
    {
        
        if (r.type < RV_F32)
        {
            if (op == OP_ADD) r.u64 = r1.u64 + r2.u64;
            else if (op == OP_SUB) r.u64 = r1.u64 - r2.u64;
            else if (op == OP_MUL) r.u64 = r1.u64 * r2.u64;
            else
            {
                
            }
        }
        else if (r1.type == RV_F32)
        {

        }
        else if (r1.type == RV_F64)
        {

        }
    }
    #else
    r.type = -1;
#define OP_INT(kind, p) if (op == kind) {\
r.type = t; \
if	  (t == RV_U64) r.u64 = r1.u64 p r2.u64;\
else if (t == RV_U32) r.u32 = r1.u32 p r2.u32;\
else if (t == RV_U16) r.u16 = r1.u16 p r2.u16;\
else if (t == RV_U8)  r.u8  = r1.u8  p r2.u8;\
else if (t == RV_I64) r.i64 = r1.i64 p r2.i64;\
else if (t == RV_I32) r.i32 = r1.i32 p r2.i32;\
else if (t == RV_I16) r.i16 = r1.i16 p r2.i16;\
else if (t == RV_I8)  r.i8  = r1.i8  p r2.i8;
    
#define OP_FLOAT(kind, p) \
else if (t == RV_F32) r.f32 = r1.f32 p r2.f32;\
else if (t == RV_F64) r.f64 = r1.f64 p r2.f64;\
else assert(0); 
    
    
#define OP(kind, p) OP_INT(kind, p) OP_FLOAT(kind, p) }
#define OP_NO_FLOAT(kind, p) OP_INT(kind, p) }
#define OPI(kind, p)  if (op == kind) {\
r.type = RV_I32; \
if	  (t == RV_U64) r.i32 = r1.u64 p r2.u64;\
else if (t == RV_U32) r.i32 = r1.u32 p r2.u32;\
else if (t == RV_U16) r.i32 = r1.u16 p r2.u16;\
else if (t == RV_U8)  r.i32  = r1.u8  p r2.u8;\
else if (t == RV_I64) r.i32 = r1.i64 p r2.i64;\
else if (t == RV_I32) r.i32 = r1.i32 p r2.i32;\
else if (t == RV_I16) r.i32 = r1.i16 p r2.i16;\
else if (t == RV_I8)  r.i32  = r1.i8  p r2.i8; \
else if (t == RV_F32) r.i32 = r1.f32 p r2.f32;\
else if (t == RV_F64) r.i32 = r1.f64 p r2.f64; \
else assert(0); }

    OP(OP_ADD, +);
    OP(OP_SUB, -);
    OP(OP_MUL, *);
    OP(OP_DIV, /);
    OP_NO_FLOAT(OP_MOD, %);

    OPI(OP_LESS, <);
    OPI(OP_GREATER, >);
    OPI(OP_EQUAL, ==);
    OPI(OP_LESS_OR_EQUAL, <=);
    OPI(OP_GREATER_OR_EQUAL, >=);
    OPI(OP_NOT_EQUAL, !=);
    assert(r.type != -1);
    
#undef OP_INT
#undef OP_FLOAT
#undef OP
#undef OP_NO_FLOAT
#undef OPI

    #endif
    return r;
}


int is_reg_value_zero(RValue v)
{
    if      (v.type == RV_U64) return v.u64 == 0;
    else if (v.type == RV_U32) return v.u32 == 0;
    else if (v.type == RV_U16) return v.u16 == 0;
    else if (v.type == RV_U8)  return v.u8 == 0;
    else if (v.type == RV_I64) return v.i64 == 0;
    else if (v.type == RV_I32) return v.i32 == 0;
    else if (v.type == RV_I16) return v.i16 == 0;
    else if (v.type == RV_I8)  return v.i8 == 0;
    else if (v.type == RV_F32) return v.f32 == 0;
    else if (v.type == RV_F64) return v.f64 == 0;
    else
        assert(0);
    return 0;
}

void sprintf_reg_value(RValue r1_value, char *s)
{
    if (r1_value.type == RV_U64)
        sprintf(s, "%"PRIu64, r1_value.u64);
    else if (r1_value.type == RV_U32)
        sprintf(s, "%"PRIu32, r1_value.u32);
    else if (r1_value.type == RV_U16)
        sprintf(s, "%"PRIu16, r1_value.u16);
    else if (r1_value.type == RV_U8)
        sprintf(s, "%"PRIu8, r1_value.u8);
    else if (r1_value.type == RV_I64)
        sprintf(s, "%"PRId64, r1_value.i64);
    else if (r1_value.type == RV_I32)
        sprintf(s, "%"PRId32, r1_value.i32);
    else if (r1_value.type == RV_I16)
        sprintf(s, "%"PRId16, r1_value.i16);
    else if (r1_value.type == RV_I8)
        sprintf(s, "%c", r1_value.i8); // @Temporary
    else if (r1_value.type == RV_F64)
        sprintf(s, "%lf", r1_value.f64);
    else if (r1_value.type == RV_F32)
        sprintf(s, "%f", r1_value.f32);
    else
        assert(0);
}

void print_reg_value(RValue r1_value)
{
    char s[128];
    sprintf_reg_value(r1_value, s);
    printf("%s", s);
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

	uint64_t data_size = align_to_size(c->p->strings_size, 16);
	uint64_t stack_size = 16 * 1024 * 1024; // 16mb
	uint64_t memory_size = data_size + stack_size; 
	uint8_t *memory = malloc(memory_size);

	uint8_t *stack = memory + data_size;

	memset(memory, 0xcc, memory_size);
    
	for (int i = 0; i < c->p->string_count; i++)
	{
		memcpy(memory + c->p->strings_offset[i], c->p->strings[i],
					strlen(c->p->strings[i]) + 1);
	}

	if (main->decl->ret_type->t != VOID)
		regs[REG_RT].type = get_rvalue_type_from_ctype(main->decl->ret_type);
	regs[REG_SP].u64 = (uint64_t)stack;
	regs[REG_SP].type = RV_U64;
    int cast = 0, op = 0, load = 0, store = 0;
    while (1)
    {
		//printf("at %d\n", ip);
        IR_Instruction *e = &c->instructions[ip];
        
		if (e->node->type == NODE_STRING)
		{
			assert(e->op == OP_MOV && e->r1.value.type == RV_U64);
			e->r1.value.u64 += (uint64_t)memory;
		}
		
		RValue r1_value = e->r1.imm ? e->r1.value : regs[e->r1.i];
		RValue r2_value = e->r2.imm ? e->r2.value : regs[e->r2.i];
        
        
        //int r1_value = (e->r1.imm ? e->r1.value : regs[e->r1]);
        //int r2_value = (e->r2.imm ? e->r2.value : regs[e->r2]);
        //	int addr = (e->r1.imm ? regs[REG_SP] - e->r1 : regs[e->r1]);
        
        if (e->op < OP_BINARY)
		{
            op++;
            int s = 0;
            
            if (e->r1.type->t == PTR)
                s = e->r1.type->ptr_to->size;
            else if (e->r2.type->t == PTR)
                s = e->r2.type->ptr_to->size;
#define OP(op, v1, v2) do{\
if (v2.type == RV_U64) regs[e->r0.i].u64 = v1.u64 op v2.u64 * s; \
else if (v2.type == RV_U32) regs[e->r0.i].u64 = v1.u64 op v2.u32 * s; \
else if (v2.type == RV_U16) regs[e->r0.i].u64 = v1.u64 op v2.u16 * s; \
else if (v2.type == RV_U8) regs[e->r0.i].u64 = v1.u64 op v2.u8 * s; \
else if (v2.type == RV_I64) regs[e->r0.i].u64 = v1.u64 op v2.i64 * s; \
else if (v2.type == RV_I32) regs[e->r0.i].u64 = v1.u64 op v2.i32 * s; \
else if (v2.type == RV_I16) regs[e->r0.i].u64 = v1.u64 op v2.i16 * s; \
else if (v2.type == RV_I8) regs[e->r0.i].u64 = v1.u64 op v2.i8 * s; \
else assert(0);\
regs[e->r0.i].type = RV_U64; \
} while(0)
            
            if (e->r1.type->t == PTR && e->r2.type->t == PTR)
            {
                assert(e->op == OP_SUB);
                assert(e->r0.type == type_int);
                assert(e->r1.type->ptr_to->size == e->r2.type->ptr_to->size);
                regs[e->r0.i].type = RV_I32;
                regs[e->r0.i].i32 = (r1_value.u64 - r2_value.u64) / e->r1.type->ptr_to->size;
            }
            else if (e->r1.type->t == PTR && (e->op == OP_SUB || e->op == OP_ADD))
            {
                if (e->op == OP_ADD)
                    OP(+, r1_value, r2_value);
                else
                    OP(-, r1_value, r2_value);
            }
            else if (e->r2.type->t == PTR && e->op == OP_ADD)
                OP(+, r2_value, r1_value);
            else
            {
                regs[e->r0.i] = eval_op(e->op, r1_value, r2_value);
                regs[e->r0.i].type = get_rvalue_type_from_ctype(e->r0.type);
            }
#undef OP
        }
        else if (e->op == OP_MOV)
        {
            assert(!e->r0.imm);
            //assert(e->r0.type == e->r1.type);
            regs[e->r0.i] = r1_value;
        }
        else if (e->op == OP_NOT)
        {
            regs[e->r0.i].type = RV_I32;
            regs[e->r0.i].i32 = is_reg_value_zero(r1_value);
        }
        else if (e->op == OP_JMP)
        {
            ip = c->labels[e->label];
            continue ;
        }
        else if (e->op == OP_JMPZ)
        {
            if (is_reg_value_zero(r1_value))
            {
                ip = c->labels[e->label];
                continue ;
            }
        }
		else if (e->op == OP_JMPNZ)
		{
            if (!is_reg_value_zero(r1_value))
            {
                ip = c->labels[e->label];
                continue ;
            }
		}
        else if (e->op == OP_WRITE)
        {
			char *buf = (char *)r1_value.u64;
			assert(r2_value.type == RV_U32);
			unsigned int len = r2_value.u32;

			write(1, buf, len);
            //print_reg_value(r1_value);
            
 //           printf("\n");
            //printf("line:%d: 0x%"PRIx64"\n", e->node->token->, r1_value.u64);
        }
        else if (e->op == OP_ASSERT)
        {
            if (is_reg_value_zero(r1_value))
            {
                printf("SIM ERROR: line %d: assert failed (value = ",
                       (!e->node || !e->node->token ? -1 : e->node->token->line));
                print_reg_value(r1_value);
                printf(")\n");
                err = 1;
                break ;
            }
        }
        else if (e->op == OP_CALL)
        {
            *(uint64_t *)(regs[REG_SP].u64) = ip + 1;
            regs[REG_SP].u64 += sizeof(uint64_t);
            
			if (c->functions[e->label].decl->ret_type->t != VOID)
				regs[REG_RT].type = get_rvalue_type_from_ctype(c->functions[e->label].decl->ret_type);
			regs[REG_RT].u64 = 0;

            ip = c->labels[e->label];
            continue ;
        }
        else if (e->op == OP_RET)
        {
            if ((void *)regs[REG_SP].u64 == stack)
                break ; // this is not quite right?
            
            regs[REG_SP].u64 -= sizeof(uint64_t);
            ip = *(uint64_t *)(regs[REG_SP].u64);
            continue ;
        }
        else if (e->op == OP_LOAD)
        {
            assert(!e->r2.imm);
            
            void *s = (void *)((e->r1.imm ? regs[REG_SP].u64 - (uint64_t)e->r1.value.u64 : r1_value.u64));
            
            int t = get_rvalue_type_from_ctype(e->r2.type);
            if 	 (t == RV_U64) regs[e->r2.i].u64 = *(uint64_t *)s;
            else if (t == RV_U32) regs[e->r2.i].u32 = *(uint32_t *)s;
            else if (t == RV_U16) regs[e->r2.i].u16 = *(uint16_t *)s;
            else if (t == RV_U8)  regs[e->r2.i].u8  = *(uint8_t   *)s;
            else if (t == RV_I64) regs[e->r2.i].i64 = *(int64_t  *)s;
            else if (t == RV_I32) regs[e->r2.i].i32 = *(int32_t  *)s;
            else if (t == RV_I16) regs[e->r2.i].i16 = *(int16_t  *)s;
            else if (t == RV_I8)  regs[e->r2.i].i8  = *(int8_t   *)s;
            else if (t == RV_F32) regs[e->r2.i].f32 = *(float  *)s;
            else if (t == RV_F64) regs[e->r2.i].f64  = *(double   *)s;
            else assert(0);
            regs[e->r2.i].type = t;
			load++;
        }
        else if (e->op == OP_STORE)
        {
            assert(e->r1.imm || r1_value.type == RV_U64);
            void *s = (void *)((e->r1.imm ? regs[REG_SP].u64 - (uint64_t)e->r1.value.u64 : r1_value.u64));
            
            int t = get_rvalue_type_from_ctype(e->r2.type);
            if 	 (t == RV_U64) *(uint64_t *)s = r2_value.u64;
            else if (t == RV_U32) *(uint32_t *)s = r2_value.u32;
            else if (t == RV_U16) *(uint16_t *)s = r2_value.u16;
            else if (t == RV_U8)  *(uint8_t  *)s = r2_value.u8;
            else if (t == RV_I64) *(int64_t  *)s = r2_value.i64;
            else if (t == RV_I32) *(int32_t  *)s = r2_value.i32;
            else if (t == RV_I16) *(int16_t  *)s = r2_value.i16;
            else if (t == RV_I8)  *(int8_t   *)s = r2_value.i8;
            else if (t == RV_F32) *(float    *)s = r2_value.f32;
            else if (t == RV_F64) *(double   *)s = r2_value.f64;
            else assert(0);
			store++;
        }
        else if (e->op == OP_CAST)
        {
            regs[e->r0.i] = convert_rvalue_type(r1_value, get_rvalue_type_from_ctype(e->r0.type));
           #if 1
            int t0 = get_rvalue_type_from_ctype(e->r0.type);
            int t1 = get_rvalue_type_from_ctype(e->r1.type);
            
            
            // TODO: simplify this shit
			#define Join(r1) r1
#define U(d1, d2, U2, u2) [RV_##U##d1 * RVALUE_COUNT + RV_##U2##d2] = {.u##d1 = (uint##d1##_t)Join(r1_value.)u2##d2, .type = t0}
#define I(d1, d2, U2, u2) [RV_##I##d1 * RVALUE_COUNT + RV_##U2##d2] = {.i##d1 = (int##d1##_t)Join(r1_value.)u2##d2, .type = t0}
            
#define ALL_U(d) U(d, 64, U, u), U(d, 32, U, u), U(d, 16, U, u), U(d, 8, U, u), \
U(d, 64, I, i), U(d, 32, I, i), U(d, 16, I, i), U(d, 8, I, i)
#define ALL_I(d) I(d, 64, U, u), I(d, 32, U, u), I(d, 16, U, u), I(d, 8, U, u), \
I(d, 64, I, i), I(d, 32, I, i), I(d, 16, I, i), I(d, 8, I, i)
            
#define ALL(d) ALL_U(d), ALL_I(d)
            
#define F(d1, d2, U2, u2, ex) [RV_F##d1 * RVALUE_COUNT + RV_##U2##d2] = {.f##d1 = (float##d1)Join(r1_value.)u2##d2, .type = t0}, \
[RV_##U2##d2 * RVALUE_COUNT + RV_F##d1] = {.u##d2 = (ex##int##d2##_t)r1_value.f##d1, .type = t0}
#define ALL_F(d) F(d, 64, I, i,), F(d, 32, I, i,), F(d, 16, I, i,), F(d, 8, I, i,), \
F(d, 64, U, u, u), F(d, 32, U, u, u), F(d, 16, U, u, u), F(d, 8, U, u, u)
            cast++;
            if (t0 == t1)
            {
                regs[e->r0.i] = r1_value;
            }
            else
            {
                #if 1
            RValue cast_table[] = {
                #if 1
                 ALL(64),
                 ALL(32),
                 ALL(16),
                 ALL(8),
                 ALL_F(64),
                 ALL_F(32),

                 #else
                
                [RV_F32 * RVALUE_COUNT + RV_I32] = {.f32 = (float32)r1_value.i32, .type = t0},
                [RV_I32 * RVALUE_COUNT + RV_F32] = {.i32 = (int32_t)r1_value.f32, .type = t0},
                [RV_F64 * RVALUE_COUNT + RV_I32] = {.f64 = (float64)r1_value.i32, .type = t0},
                [RV_I8 * RVALUE_COUNT + RV_I32] = {.i8 = (int8_t)r1_value.i32, .type = t0},
                #endif
                [RV_F64 * RVALUE_COUNT + RV_F32] = {.f64 = (float64)r1_value.f32, .type = t0},
                [RV_F32 * RVALUE_COUNT + RV_F64] = {.f32 = (float32)r1_value.f64, .type = t0},
            };
            
        
            int idx = t0 * RVALUE_COUNT + t1;
            if (idx >= array_length(cast_table) || cast_table[idx].type != t0)
            {
                printf("can't convert to %s from %s\n", get_type_str(e->r0.type), get_type_str(e->r1.type));
                assert(0);
            }
            regs[e->r0.i] = cast_table[idx];
            #else
            if (t0 == RV_F64 && t1 == RV_F32) regs[e->r0.i].f64 = r1_value.f32;
            else if (t0 == RV_F32 && t1 == RV_F64) regs[e->r0.i].f32 = r1_value.f64;
            else if (t0 == RV_F64 && t1 == RV_I32) regs[e->r0.i].f64 = r1_value.i32;
            else if (t0 == RV_F64 && t1 == RV_F32) regs[e->r0.i].f64 = r1_value.f32;
            else if (t0 == RV_F32 && t1 == RV_I32) regs[e->r0.i].f32 = r1_value.i32;
            else if (t0 == RV_I32 && t1 == RV_F32) regs[e->r0.i].i32 = r1_value.f32;
            else if (t0 == RV_I8 && t1 == RV_I32) regs[e->r0.i].i8 = r1_value.i32;
            else assert(0);

            regs[e->r0.i].type = t0;
            #endif
            }
#undef U
#undef I
#undef ALL
#undef ALL_U
#undef ALL_I
#undef Join
#endif
        }
        else
            assert(0);
        ip++;
    }
    free(memory);

    printf("\n\033[1;32mstats:\033[0m %d binops, %d loads, %d stores, %d cast\n", op, load, store, cast);
    return err;
}
