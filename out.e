# 0 "sim_ir.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "sim_ir.c"
# 9 "sim_ir.c"
int get_rvalue_type_from_ctype(Type *t)
{
 int u = t->is_unsigned;


    if (t->t == FLOAT) return RV_F32;
    else if (t->t == DOUBLE) return RV_F64;
 else if (t->size == 8) return u ? RV_U64 : RV_I64;
 else if (t->size == 4) return u ? RV_U32 : RV_I32;
 else if (t->size == 2) return u ? RV_U16 : RV_I16;
 else if (t->size == 1) return u ? RV_U8 : RV_I8;
 assert(0);
 return 0;
}

RValue eval_op(int op, RValue r1, RValue r2)
{
 assert(r1.type == r2.type);
 int t = r1.type;

 RValue r;

 r.type = -1;
# 53 "sim_ir.c"
    if (op == OP_ADD) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 + r2.u64;else if (t == RV_U32) r.u32 = r1.u32 + r2.u32;else if (t == RV_U16) r.u16 = r1.u16 + r2.u16;else if (t == RV_U8) r.u8 = r1.u8 + r2.u8;else if (t == RV_I64) r.i64 = r1.i64 + r2.i64;else if (t == RV_I32) r.i32 = r1.i32 + r2.i32;else if (t == RV_I16) r.i16 = r1.i16 + r2.i16;else if (t == RV_I8) r.i8 = r1.i8 + r2.i8; else if (t == RV_F32) r.f32 = r1.f32 + r2.f32;else if (t == RV_F64) r.f64 = r1.f64 + r2.f64;else assert(0); };
    if (op == OP_SUB) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 - r2.u64;else if (t == RV_U32) r.u32 = r1.u32 - r2.u32;else if (t == RV_U16) r.u16 = r1.u16 - r2.u16;else if (t == RV_U8) r.u8 = r1.u8 - r2.u8;else if (t == RV_I64) r.i64 = r1.i64 - r2.i64;else if (t == RV_I32) r.i32 = r1.i32 - r2.i32;else if (t == RV_I16) r.i16 = r1.i16 - r2.i16;else if (t == RV_I8) r.i8 = r1.i8 - r2.i8; else if (t == RV_F32) r.f32 = r1.f32 - r2.f32;else if (t == RV_F64) r.f64 = r1.f64 - r2.f64;else assert(0); };
    if (op == OP_MUL) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 * r2.u64;else if (t == RV_U32) r.u32 = r1.u32 * r2.u32;else if (t == RV_U16) r.u16 = r1.u16 * r2.u16;else if (t == RV_U8) r.u8 = r1.u8 * r2.u8;else if (t == RV_I64) r.i64 = r1.i64 * r2.i64;else if (t == RV_I32) r.i32 = r1.i32 * r2.i32;else if (t == RV_I16) r.i16 = r1.i16 * r2.i16;else if (t == RV_I8) r.i8 = r1.i8 * r2.i8; else if (t == RV_F32) r.f32 = r1.f32 * r2.f32;else if (t == RV_F64) r.f64 = r1.f64 * r2.f64;else assert(0); };
    if (op == OP_DIV) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 / r2.u64;else if (t == RV_U32) r.u32 = r1.u32 / r2.u32;else if (t == RV_U16) r.u16 = r1.u16 / r2.u16;else if (t == RV_U8) r.u8 = r1.u8 / r2.u8;else if (t == RV_I64) r.i64 = r1.i64 / r2.i64;else if (t == RV_I32) r.i32 = r1.i32 / r2.i32;else if (t == RV_I16) r.i16 = r1.i16 / r2.i16;else if (t == RV_I8) r.i8 = r1.i8 / r2.i8; else if (t == RV_F32) r.f32 = r1.f32 / r2.f32;else if (t == RV_F64) r.f64 = r1.f64 / r2.f64;else assert(0); };
    if (op == OP_MOD) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 % r2.u64;else if (t == RV_U32) r.u32 = r1.u32 % r2.u32;else if (t == RV_U16) r.u16 = r1.u16 % r2.u16;else if (t == RV_U8) r.u8 = r1.u8 % r2.u8;else if (t == RV_I64) r.i64 = r1.i64 % r2.i64;else if (t == RV_I32) r.i32 = r1.i32 % r2.i32;else if (t == RV_I16) r.i16 = r1.i16 % r2.i16;else if (t == RV_I8) r.i8 = r1.i8 % r2.i8; };
    if (op == OP_LESS) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 < r2.u64;else if (t == RV_U32) r.u32 = r1.u32 < r2.u32;else if (t == RV_U16) r.u16 = r1.u16 < r2.u16;else if (t == RV_U8) r.u8 = r1.u8 < r2.u8;else if (t == RV_I64) r.i64 = r1.i64 < r2.i64;else if (t == RV_I32) r.i32 = r1.i32 < r2.i32;else if (t == RV_I16) r.i16 = r1.i16 < r2.i16;else if (t == RV_I8) r.i8 = r1.i8 < r2.i8; else if (t == RV_F32) r.f32 = r1.f32 < r2.f32;else if (t == RV_F64) r.f64 = r1.f64 < r2.f64;else assert(0); };
    if (op == OP_GREATER) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 > r2.u64;else if (t == RV_U32) r.u32 = r1.u32 > r2.u32;else if (t == RV_U16) r.u16 = r1.u16 > r2.u16;else if (t == RV_U8) r.u8 = r1.u8 > r2.u8;else if (t == RV_I64) r.i64 = r1.i64 > r2.i64;else if (t == RV_I32) r.i32 = r1.i32 > r2.i32;else if (t == RV_I16) r.i16 = r1.i16 > r2.i16;else if (t == RV_I8) r.i8 = r1.i8 > r2.i8; else if (t == RV_F32) r.f32 = r1.f32 > r2.f32;else if (t == RV_F64) r.f64 = r1.f64 > r2.f64;else assert(0); };
    if (op == OP_EQUAL) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 == r2.u64;else if (t == RV_U32) r.u32 = r1.u32 == r2.u32;else if (t == RV_U16) r.u16 = r1.u16 == r2.u16;else if (t == RV_U8) r.u8 = r1.u8 == r2.u8;else if (t == RV_I64) r.i64 = r1.i64 == r2.i64;else if (t == RV_I32) r.i32 = r1.i32 == r2.i32;else if (t == RV_I16) r.i16 = r1.i16 == r2.i16;else if (t == RV_I8) r.i8 = r1.i8 == r2.i8; else if (t == RV_F32) r.f32 = r1.f32 == r2.f32;else if (t == RV_F64) r.f64 = r1.f64 == r2.f64;else assert(0); };
    if (op == OP_LESS_OR_EQUAL) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 <= r2.u64;else if (t == RV_U32) r.u32 = r1.u32 <= r2.u32;else if (t == RV_U16) r.u16 = r1.u16 <= r2.u16;else if (t == RV_U8) r.u8 = r1.u8 <= r2.u8;else if (t == RV_I64) r.i64 = r1.i64 <= r2.i64;else if (t == RV_I32) r.i32 = r1.i32 <= r2.i32;else if (t == RV_I16) r.i16 = r1.i16 <= r2.i16;else if (t == RV_I8) r.i8 = r1.i8 <= r2.i8; else if (t == RV_F32) r.f32 = r1.f32 <= r2.f32;else if (t == RV_F64) r.f64 = r1.f64 <= r2.f64;else assert(0); };
    if (op == OP_GREATER_OR_EQUAL) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 >= r2.u64;else if (t == RV_U32) r.u32 = r1.u32 >= r2.u32;else if (t == RV_U16) r.u16 = r1.u16 >= r2.u16;else if (t == RV_U8) r.u8 = r1.u8 >= r2.u8;else if (t == RV_I64) r.i64 = r1.i64 >= r2.i64;else if (t == RV_I32) r.i32 = r1.i32 >= r2.i32;else if (t == RV_I16) r.i16 = r1.i16 >= r2.i16;else if (t == RV_I8) r.i8 = r1.i8 >= r2.i8; else if (t == RV_F32) r.f32 = r1.f32 >= r2.f32;else if (t == RV_F64) r.f64 = r1.f64 >= r2.f64;else assert(0); };
    if (op == OP_NOT_EQUAL) {r.type = t; if (t == RV_U64) r.u64 = r1.u64 != r2.u64;else if (t == RV_U32) r.u32 = r1.u32 != r2.u32;else if (t == RV_U16) r.u16 = r1.u16 != r2.u16;else if (t == RV_U8) r.u8 = r1.u8 != r2.u8;else if (t == RV_I64) r.i64 = r1.i64 != r2.i64;else if (t == RV_I32) r.i32 = r1.i32 != r2.i32;else if (t == RV_I16) r.i16 = r1.i16 != r2.i16;else if (t == RV_I8) r.i8 = r1.i8 != r2.i8; else if (t == RV_F32) r.f32 = r1.f32 != r2.f32;else if (t == RV_F64) r.f64 = r1.f64 != r2.f64;else assert(0); };
    assert(r.type != -1);





    return r;
}


int is_reg_value_zero(RValue v)
{
    if (v.type == RV_U64) return v.u64 == 0;
    else if (v.type == RV_U32) return v.u32 == 0;
    else if (v.type == RV_U16) return v.u16 == 0;
    else if (v.type == RV_U8) return v.u8 == 0;
    else if (v.type == RV_I64) return v.i64 == 0;
    else if (v.type == RV_I32) return v.i32 == 0;
    else if (v.type == RV_I16) return v.i16 == 0;
    else if (v.type == RV_I8) return v.i8 == 0;
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
        sprintf(s, "%c", r1_value.i8);
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

int sim_ir_code(IR_Code *c)
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
 regs[REG_SP].type = RV_U64;

    while (1)
    {

        IR_Instruction *e = &c->instructions[ip];


  RValue r1_value = e->r1.imm ? e->r1.value : regs[e->r1.i];
  RValue r2_value = e->r2.imm ? e->r2.value : regs[e->r2.i];






        if (e->op < OP_BINARY)
  {
            int s = 0;

            if (e->r1.type->t == PTR)
                s = e->r1.type->ptr_to->size;
            else if (e->r2.type->t == PTR)
                s = e->r1.type->ptr_to->size;
# 193 "sim_ir.c"
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
                    do{if (r2_value.type == RV_U64) regs[e->r0.i].u64 = r1_value.u64 + r2_value.u64 * s; else if (r2_value.type == RV_U32) regs[e->r0.i].u64 = r1_value.u64 + r2_value.u32 * s; else if (r2_value.type == RV_U16) regs[e->r0.i].u64 = r1_value.u64 + r2_value.u16 * s; else if (r2_value.type == RV_U8) regs[e->r0.i].u64 = r1_value.u64 + r2_value.u8 * s; else if (r2_value.type == RV_I64) regs[e->r0.i].u64 = r1_value.u64 + r2_value.i64 * s; else if (r2_value.type == RV_I32) regs[e->r0.i].u64 = r1_value.u64 + r2_value.i32 * s; else if (r2_value.type == RV_I16) regs[e->r0.i].u64 = r1_value.u64 + r2_value.i16 * s; else if (r2_value.type == RV_I8) regs[e->r0.i].u64 = r1_value.u64 + r2_value.i8 * s; else assert(0);regs[e->r0.i].type = RV_U64; } while(0);
                else
                    do{if (r2_value.type == RV_U64) regs[e->r0.i].u64 = r1_value.u64 - r2_value.u64 * s; else if (r2_value.type == RV_U32) regs[e->r0.i].u64 = r1_value.u64 - r2_value.u32 * s; else if (r2_value.type == RV_U16) regs[e->r0.i].u64 = r1_value.u64 - r2_value.u16 * s; else if (r2_value.type == RV_U8) regs[e->r0.i].u64 = r1_value.u64 - r2_value.u8 * s; else if (r2_value.type == RV_I64) regs[e->r0.i].u64 = r1_value.u64 - r2_value.i64 * s; else if (r2_value.type == RV_I32) regs[e->r0.i].u64 = r1_value.u64 - r2_value.i32 * s; else if (r2_value.type == RV_I16) regs[e->r0.i].u64 = r1_value.u64 - r2_value.i16 * s; else if (r2_value.type == RV_I8) regs[e->r0.i].u64 = r1_value.u64 - r2_value.i8 * s; else assert(0);regs[e->r0.i].type = RV_U64; } while(0);
            }
            else if (e->r2.type->t == PTR && e->op == OP_ADD)
                do{if (r1_value.type == RV_U64) regs[e->r0.i].u64 = r2_value.u64 + r1_value.u64 * s; else if (r1_value.type == RV_U32) regs[e->r0.i].u64 = r2_value.u64 + r1_value.u32 * s; else if (r1_value.type == RV_U16) regs[e->r0.i].u64 = r2_value.u64 + r1_value.u16 * s; else if (r1_value.type == RV_U8) regs[e->r0.i].u64 = r2_value.u64 + r1_value.u8 * s; else if (r1_value.type == RV_I64) regs[e->r0.i].u64 = r2_value.u64 + r1_value.i64 * s; else if (r1_value.type == RV_I32) regs[e->r0.i].u64 = r2_value.u64 + r1_value.i32 * s; else if (r1_value.type == RV_I16) regs[e->r0.i].u64 = r2_value.u64 + r1_value.i16 * s; else if (r1_value.type == RV_I8) regs[e->r0.i].u64 = r2_value.u64 + r1_value.i8 * s; else assert(0);regs[e->r0.i].type = RV_U64; } while(0);
            else
            {
                regs[e->r0.i] = eval_op(e->op, r1_value, r2_value);
                regs[e->r0.i].type = get_rvalue_type_from_ctype(e->r0.type);
            }

        }
        else if (e->op == OP_MOV)
        {
            assert(!e->r0.imm);

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
        else if (e->op == OP_PRINT)
        {
            print_reg_value(r1_value);

            printf("\n");


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
            if ((void *)regs[REG_SP].u64 == stack + stack_max)
            {
                printf("SIM ERROR: stack overflow\n");
                err = 1;
                break ;
            }
            *(uint64_t *)(regs[REG_SP].u64) = ip + 1;
            regs[REG_SP].u64 += sizeof(uint64_t);

            ip = c->labels[e->label];
            continue ;
        }
        else if (e->op == OP_RET)
        {
            if ((void *)regs[REG_SP].u64 == stack)
                break ;

            regs[REG_SP].u64 -= sizeof(uint64_t);
            ip = *(uint64_t *)(regs[REG_SP].u64);
            continue ;
        }
        else if (e->op == OP_LOAD)
        {
            assert(!e->r2.imm);

            void *s = (void *)((e->r1.imm ? regs[REG_SP].u64 - (uint64_t)e->r1.value.u64 : r1_value.u64));

            int t = get_rvalue_type_from_ctype(e->r2.type);
            if (t == RV_U64) regs[e->r2.i].u64 = *(uint64_t *)s;
            else if (t == RV_U32) regs[e->r2.i].u32 = *(uint32_t *)s;
            else if (t == RV_U16) regs[e->r2.i].u16 = *(uint16_t *)s;
            else if (t == RV_U8) regs[e->r2.i].u8 = *(uint8_t *)s;
            else if (t == RV_I64) regs[e->r2.i].i64 = *(int64_t *)s;
            else if (t == RV_I32) regs[e->r2.i].i32 = *(int32_t *)s;
            else if (t == RV_I16) regs[e->r2.i].i16 = *(int16_t *)s;
            else if (t == RV_I8) regs[e->r2.i].i8 = *(int8_t *)s;
            else if (t == RV_F32) regs[e->r2.i].f32 = *(float *)s;
            else if (t == RV_F64) regs[e->r2.i].f64 = *(double *)s;
            else assert(0);
            regs[e->r2.i].type = t;
        }
        else if (e->op == OP_STORE)
        {
            assert(e->r1.imm || r1_value.type == RV_U64);
            void *s = (void *)((e->r1.imm ? regs[REG_SP].u64 - (uint64_t)e->r1.value.u64 : r1_value.u64));

            int t = get_rvalue_type_from_ctype(e->r2.type);
            if (t == RV_U64) *(uint64_t *)s = r2_value.u64;
            else if (t == RV_U32) *(uint32_t *)s = r2_value.u32;
            else if (t == RV_U16) *(uint16_t *)s = r2_value.u16;
            else if (t == RV_U8) *(uint8_t *)s = r2_value.u8;
            else if (t == RV_I64) *(int64_t *)s = r2_value.i64;
            else if (t == RV_I32) *(int32_t *)s = r2_value.i32;
            else if (t == RV_I16) *(int16_t *)s = r2_value.i16;
            else if (t == RV_I8) *(int8_t *)s = r2_value.i8;
            else if (t == RV_F32) *(float *)s = r2_value.f32;
            else if (t == RV_F64) *(double *)s = r2_value.f64;
            else assert(0);
        }
        else if (e->op == OP_CAST)
        {

            int t0 = get_rvalue_type_from_ctype(e->r0.type);
            int t1 = get_rvalue_type_from_ctype(e->r1.type);
# 347 "sim_ir.c"
            RValue cast_table[] = {
                [RV_U64 * RVALUE_COUNT + RV_U64] = {.u64 = (uint64_t)r1_value.u64, .type = t0}, [RV_U64 * RVALUE_COUNT + RV_U32] = {.u64 = (uint64_t)r1_value.u32, .type = t0}, [RV_U64 * RVALUE_COUNT + RV_U16] = {.u64 = (uint64_t)r1_value.u16, .type = t0}, [RV_U64 * RVALUE_COUNT + RV_U8] = {.u64 = (uint64_t)r1_value.u8, .type = t0}, [RV_U64 * RVALUE_COUNT + RV_I64] = {.u64 = (uint64_t)r1_value.i64, .type = t0}, [RV_U64 * RVALUE_COUNT + RV_I32] = {.u64 = (uint64_t)r1_value.i32, .type = t0}, [RV_U64 * RVALUE_COUNT + RV_I16] = {.u64 = (uint64_t)r1_value.i16, .type = t0}, [RV_U64 * RVALUE_COUNT + RV_I8] = {.u64 = (uint64_t)r1_value.i8, .type = t0}, [RV_I64 * RVALUE_COUNT + RV_U64] = {.i64 = (int64_t)r1_value.u64, .type = t0}, [RV_I64 * RVALUE_COUNT + RV_U32] = {.i64 = (int64_t)r1_value.u32, .type = t0}, [RV_I64 * RVALUE_COUNT + RV_U16] = {.i64 = (int64_t)r1_value.u16, .type = t0}, [RV_I64 * RVALUE_COUNT + RV_U8] = {.i64 = (int64_t)r1_value.u8, .type = t0}, [RV_I64 * RVALUE_COUNT + RV_I64] = {.i64 = (int64_t)r1_value.i64, .type = t0}, [RV_I64 * RVALUE_COUNT + RV_I32] = {.i64 = (int64_t)r1_value.i32, .type = t0}, [RV_I64 * RVALUE_COUNT + RV_I16] = {.i64 = (int64_t)r1_value.i16, .type = t0}, [RV_I64 * RVALUE_COUNT + RV_I8] = {.i64 = (int64_t)r1_value.i8, .type = t0},
                [RV_U32 * RVALUE_COUNT + RV_U64] = {.u32 = (uint32_t)r1_value.u64, .type = t0}, [RV_U32 * RVALUE_COUNT + RV_U32] = {.u32 = (uint32_t)r1_value.u32, .type = t0}, [RV_U32 * RVALUE_COUNT + RV_U16] = {.u32 = (uint32_t)r1_value.u16, .type = t0}, [RV_U32 * RVALUE_COUNT + RV_U8] = {.u32 = (uint32_t)r1_value.u8, .type = t0}, [RV_U32 * RVALUE_COUNT + RV_I64] = {.u32 = (uint32_t)r1_value.i64, .type = t0}, [RV_U32 * RVALUE_COUNT + RV_I32] = {.u32 = (uint32_t)r1_value.i32, .type = t0}, [RV_U32 * RVALUE_COUNT + RV_I16] = {.u32 = (uint32_t)r1_value.i16, .type = t0}, [RV_U32 * RVALUE_COUNT + RV_I8] = {.u32 = (uint32_t)r1_value.i8, .type = t0}, [RV_I32 * RVALUE_COUNT + RV_U64] = {.i32 = (int32_t)r1_value.u64, .type = t0}, [RV_I32 * RVALUE_COUNT + RV_U32] = {.i32 = (int32_t)r1_value.u32, .type = t0}, [RV_I32 * RVALUE_COUNT + RV_U16] = {.i32 = (int32_t)r1_value.u16, .type = t0}, [RV_I32 * RVALUE_COUNT + RV_U8] = {.i32 = (int32_t)r1_value.u8, .type = t0}, [RV_I32 * RVALUE_COUNT + RV_I64] = {.i32 = (int32_t)r1_value.i64, .type = t0}, [RV_I32 * RVALUE_COUNT + RV_I32] = {.i32 = (int32_t)r1_value.i32, .type = t0}, [RV_I32 * RVALUE_COUNT + RV_I16] = {.i32 = (int32_t)r1_value.i16, .type = t0}, [RV_I32 * RVALUE_COUNT + RV_I8] = {.i32 = (int32_t)r1_value.i8, .type = t0},
                [RV_U16 * RVALUE_COUNT + RV_U64] = {.u16 = (uint16_t)r1_value.u64, .type = t0}, [RV_U16 * RVALUE_COUNT + RV_U32] = {.u16 = (uint16_t)r1_value.u32, .type = t0}, [RV_U16 * RVALUE_COUNT + RV_U16] = {.u16 = (uint16_t)r1_value.u16, .type = t0}, [RV_U16 * RVALUE_COUNT + RV_U8] = {.u16 = (uint16_t)r1_value.u8, .type = t0}, [RV_U16 * RVALUE_COUNT + RV_I64] = {.u16 = (uint16_t)r1_value.i64, .type = t0}, [RV_U16 * RVALUE_COUNT + RV_I32] = {.u16 = (uint16_t)r1_value.i32, .type = t0}, [RV_U16 * RVALUE_COUNT + RV_I16] = {.u16 = (uint16_t)r1_value.i16, .type = t0}, [RV_U16 * RVALUE_COUNT + RV_I8] = {.u16 = (uint16_t)r1_value.i8, .type = t0}, [RV_I16 * RVALUE_COUNT + RV_U64] = {.i16 = (int16_t)r1_value.u64, .type = t0}, [RV_I16 * RVALUE_COUNT + RV_U32] = {.i16 = (int16_t)r1_value.u32, .type = t0}, [RV_I16 * RVALUE_COUNT + RV_U16] = {.i16 = (int16_t)r1_value.u16, .type = t0}, [RV_I16 * RVALUE_COUNT + RV_U8] = {.i16 = (int16_t)r1_value.u8, .type = t0}, [RV_I16 * RVALUE_COUNT + RV_I64] = {.i16 = (int16_t)r1_value.i64, .type = t0}, [RV_I16 * RVALUE_COUNT + RV_I32] = {.i16 = (int16_t)r1_value.i32, .type = t0}, [RV_I16 * RVALUE_COUNT + RV_I16] = {.i16 = (int16_t)r1_value.i16, .type = t0}, [RV_I16 * RVALUE_COUNT + RV_I8] = {.i16 = (int16_t)r1_value.i8, .type = t0},
                [RV_U8 * RVALUE_COUNT + RV_U64] = {.u8 = (uint8_t)r1_value.u64, .type = t0}, [RV_U8 * RVALUE_COUNT + RV_U32] = {.u8 = (uint8_t)r1_value.u32, .type = t0}, [RV_U8 * RVALUE_COUNT + RV_U16] = {.u8 = (uint8_t)r1_value.u16, .type = t0}, [RV_U8 * RVALUE_COUNT + RV_U8] = {.u8 = (uint8_t)r1_value.u8, .type = t0}, [RV_U8 * RVALUE_COUNT + RV_I64] = {.u8 = (uint8_t)r1_value.i64, .type = t0}, [RV_U8 * RVALUE_COUNT + RV_I32] = {.u8 = (uint8_t)r1_value.i32, .type = t0}, [RV_U8 * RVALUE_COUNT + RV_I16] = {.u8 = (uint8_t)r1_value.i16, .type = t0}, [RV_U8 * RVALUE_COUNT + RV_I8] = {.u8 = (uint8_t)r1_value.i8, .type = t0}, [RV_I8 * RVALUE_COUNT + RV_U64] = {.i8 = (int8_t)r1_value.u64, .type = t0}, [RV_I8 * RVALUE_COUNT + RV_U32] = {.i8 = (int8_t)r1_value.u32, .type = t0}, [RV_I8 * RVALUE_COUNT + RV_U16] = {.i8 = (int8_t)r1_value.u16, .type = t0}, [RV_I8 * RVALUE_COUNT + RV_U8] = {.i8 = (int8_t)r1_value.u8, .type = t0}, [RV_I8 * RVALUE_COUNT + RV_I64] = {.i8 = (int8_t)r1_value.i64, .type = t0}, [RV_I8 * RVALUE_COUNT + RV_I32] = {.i8 = (int8_t)r1_value.i32, .type = t0}, [RV_I8 * RVALUE_COUNT + RV_I16] = {.i8 = (int8_t)r1_value.i16, .type = t0}, [RV_I8 * RVALUE_COUNT + RV_I8] = {.i8 = (int8_t)r1_value.i8, .type = t0},
                [RV_F64 * RVALUE_COUNT + RV_I64] = {.f64 = (float64)r1_value.i64, .type = t0}, [RV_I64 * RVALUE_COUNT + RV_F64] = {.u64 = (int64_t)r1_value.f64, .type = t0}, [RV_F64 * RVALUE_COUNT + RV_I32] = {.f64 = (float64)r1_value.i32, .type = t0}, [RV_I32 * RVALUE_COUNT + RV_F64] = {.u32 = (int32_t)r1_value.f64, .type = t0}, [RV_F64 * RVALUE_COUNT + RV_I16] = {.f64 = (float64)r1_value.i16, .type = t0}, [RV_I16 * RVALUE_COUNT + RV_F64] = {.u16 = (int16_t)r1_value.f64, .type = t0}, [RV_F64 * RVALUE_COUNT + RV_I8] = {.f64 = (float64)r1_value.i8, .type = t0}, [RV_I8 * RVALUE_COUNT + RV_F64] = {.u8 = (int8_t)r1_value.f64, .type = t0}, [RV_F64 * RVALUE_COUNT + RV_U64] = {.f64 = (float64)r1_value.u64, .type = t0}, [RV_U64 * RVALUE_COUNT + RV_F64] = {.u64 = (uint64_t)r1_value.f64, .type = t0}, [RV_F64 * RVALUE_COUNT + RV_U32] = {.f64 = (float64)r1_value.u32, .type = t0}, [RV_U32 * RVALUE_COUNT + RV_F64] = {.u32 = (uint32_t)r1_value.f64, .type = t0}, [RV_F64 * RVALUE_COUNT + RV_U16] = {.f64 = (float64)r1_value.u16, .type = t0}, [RV_U16 * RVALUE_COUNT + RV_F64] = {.u16 = (uint16_t)r1_value.f64, .type = t0}, [RV_F64 * RVALUE_COUNT + RV_U8] = {.f64 = (float64)r1_value.u8, .type = t0}, [RV_U8 * RVALUE_COUNT + RV_F64] = {.u8 = (uint8_t)r1_value.f64, .type = t0},
                [RV_F32 * RVALUE_COUNT + RV_I64] = {.f32 = (float32)r1_value.i64, .type = t0}, [RV_I64 * RVALUE_COUNT + RV_F32] = {.u64 = (int64_t)r1_value.f32, .type = t0}, [RV_F32 * RVALUE_COUNT + RV_I32] = {.f32 = (float32)r1_value.i32, .type = t0}, [RV_I32 * RVALUE_COUNT + RV_F32] = {.u32 = (int32_t)r1_value.f32, .type = t0}, [RV_F32 * RVALUE_COUNT + RV_I16] = {.f32 = (float32)r1_value.i16, .type = t0}, [RV_I16 * RVALUE_COUNT + RV_F32] = {.u16 = (int16_t)r1_value.f32, .type = t0}, [RV_F32 * RVALUE_COUNT + RV_I8] = {.f32 = (float32)r1_value.i8, .type = t0}, [RV_I8 * RVALUE_COUNT + RV_F32] = {.u8 = (int8_t)r1_value.f32, .type = t0}, [RV_F32 * RVALUE_COUNT + RV_U64] = {.f32 = (float32)r1_value.u64, .type = t0}, [RV_U64 * RVALUE_COUNT + RV_F32] = {.u64 = (uint64_t)r1_value.f32, .type = t0}, [RV_F32 * RVALUE_COUNT + RV_U32] = {.f32 = (float32)r1_value.u32, .type = t0}, [RV_U32 * RVALUE_COUNT + RV_F32] = {.u32 = (uint32_t)r1_value.f32, .type = t0}, [RV_F32 * RVALUE_COUNT + RV_U16] = {.f32 = (float32)r1_value.u16, .type = t0}, [RV_U16 * RVALUE_COUNT + RV_F32] = {.u16 = (uint16_t)r1_value.f32, .type = t0}, [RV_F32 * RVALUE_COUNT + RV_U8] = {.f32 = (float32)r1_value.u8, .type = t0}, [RV_U8 * RVALUE_COUNT + RV_F32] = {.u8 = (uint8_t)r1_value.f32, .type = t0},
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





        }
        else
            assert(0);
        ip++;
    }
    free(stack);
    return err;
}
