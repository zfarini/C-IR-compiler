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

int get_var_register(IR_Code *c, char *name)
{
    int i = 0;

    while (c->vars_reg[i].name && strcmp(c->vars_reg[i].name, name))
        i++;

    if (!c->vars_reg[i].name)
        return (-1);

    return c->vars_reg[i].reg;
}

void set_var_register(IR_Code *c, char *name, int reg)
{
    int i = 0;

    while (c->vars_reg[i].name && strcmp(c->vars_reg[i].name, name))
        i++;

    assert(i < array_length(c->vars_reg));
    c->vars_reg[i].name = name;
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
        reg = get_var_register(c, node->token->name);
        if (reg < 0) {
            reg = c->curr_reg++;
            set_var_register(c, node->token->name, reg);
        }
    }
    else if (node->type == NODE_CALL)
    {
 //       IR_Instruction *e = add_instruction(OP_JMP);
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
#if 0
    else if (node->type == NODE_WHILE)
    {
        int enter_label = c->label_count;
        int exit_label = c->label_count + 1;
        int else_label;
        
        c->label_count += 2;
        c->labels[enter_label] = c->instruction_count;

        if (node->else_node)
        {
            else_label = c->label_count;
            c->label_count++;
        }

        int r0 = gen_ir(c, node->left);
        IR_Instruction *e = add_instruction(c, OP_JMPZ);
        e->r0 = r0;
        e->r1 = (node->else_node ? else_label : exit_label);
        
        gen_ir(c, node->right);

        if (node->type == NODE_WHILE)
        {
            e = add_instruction(c, OP_JMP);
            e->r0 = enter_label;
        }
        else if (node->else_node)
        {
            e = add_instruction(c, OP_JMP);
            e->r0 = exit_label;
            c->labels[else_label] = c->instruction_count;
            gen_ir(c, node->else_node);
        }

        c->labels[exit_label] = c->instruction_count;
    }
#endif
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
    else
        assert(0);
    return reg;
}

IR_Code *gen_ir_code(Node *node)
{
    IR_Code *c = calloc(1, sizeof(*c));

    while (node)
    {
        gen_ir(c, node);
        node = node->next_expr;
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
    while (ip < c->instruction_count)
    {
        IR_Instruction *e = &c->instructions[ip];
        
        int r1_value = (e->r1_imm ? e->r1 : regs[e->r1]);
        int r2_value = (e->r2_imm ? e->r2 : regs[e->r2]);

        if (e->op == OP_JMP)
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
        else if (e->op < OP_BINARY)
            regs[e->r0] = eval_op(e->op, r1_value, r2_value);
        else if (e->op == OP_MOV)
            regs[e->r0] = r1_value;
        else
            assert(0);

        ip++;
    }
}

void print_instruction(IR_Instruction *e, int in_block)
{
    if (e->op == OP_JMP)
        printf("jmp %s%d", (in_block ? "B" : "L"), e->r0);
    else if (e->op == OP_JMPZ)
        printf("jmpz %s%d t%d", (in_block ? "B" : "L"), e->r0, e->r1);
    else if (e->op == OP_PRINT)
        printf("print %s%d", e->r1_imm ? "" : "t", e->r1);
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
    printf("count: %d\n", c->instruction_count);

    for (int i = 0; i < c->instruction_count; i++)
    {
        IR_Instruction *e = &c->instructions[i];

        for (int j = 0; j < c->label_count; j++)
        {
            if (c->labels[j] == i)
                printf("\tL%d:\n", j);
        }

        printf("%-16d", i);
        print_instruction(e, 0);
    }

    for (int j = 0; j < c->label_count; j++)
    {
        if (c->labels[j] == c->instruction_count)
            printf("\tL%d:\n", j);
        assert(c->labels[j] <= c->instruction_count);
    }
    
    //printf("variables:\n");
    //for (int i = 0; c->vars_reg[i].name; i++)
    //    printf("%s -> t%d\n", c->vars_reg[i].name, c->vars_reg[i].reg);
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

int find_value(Value *values, int value_count, Value *to_find)
{
    for (int i = 0; i < value_count; i++)
        if (!memcmp(&values[i], to_find, sizeof(*to_find)))
            return i;

    return value_count;
}

void assign_register_to_value(int *value_owner, 
                                int *register_value, int value_count, int reg, int i)
{
    assert(i < value_count); 

    for (int j = 0; j < value_count; j++)
    {
        if (value_owner[j] == reg)
        {
            value_owner[j] = -1;
            for (int k = 0; k < 256; k++)
            {
                if (k != reg && register_value[k] == j)
                {
                    value_owner[j] = k;
                    break ;
                }
            }
        }
    }

    if (value_owner[i] == -1)
        value_owner[i] = reg;
    register_value[reg] = i;
}

int is_binop_commutative(int op)
{
    assert(op < OP_BINARY);
    return (op == '+' || op == '*' || op == OP_EQUAL ||
            op == OP_NOT_EQUAL);
}

void local_value_numbering_for_basic_block(IR_Basic_Block *b)
{

    Value   values[1024];
    int     value_count = 0;
    int     value_owner[1024];
    memset(value_owner, -1, sizeof(value_owner));
    
    int register_value[256];
    memset(register_value, -1, sizeof(register_value));

    for (int i = 0; i < b->instruction_count; i++)
    {
        IR_Instruction *e = &b->instructions[i];

        if (e->op == OP_MOV || e->op == OP_PRINT)
        {
            Value v = {0};
            v.type = (e->r1_imm ? VALUE_CONST : VALUE_REGISTER);
            v.v1 = e->r1;
            
            int j = find_value(values, value_count, &v);

            if (!e->r1_imm && register_value[e->r1] == -1)
            {
                assert(j == value_count);
                register_value[e->r1] = value_count;
                values[value_count].type = VALUE_REGISTER;
                values[value_count].v1 = e->r1;
                value_owner[value_count] = e->r1;
                value_count++;
            }
            if (!e->r1_imm)
                j = register_value[e->r1];
            else if (j == value_count)
                values[value_count++] = v;

            if (!e->r1_imm)
                assert(value_owner[j] != -1);

            if (e->op != OP_PRINT)
                assign_register_to_value(value_owner, register_value, value_count, e->r0, j);

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

            assert(!e->r1_imm && !e->r2_imm);

            if (register_value[e->r1] == -1)
            {
                register_value[e->r1] = value_count;
                values[value_count].type = VALUE_REGISTER;
                values[value_count].v1 = e->r1;
                value_owner[value_count] = e->r1;
                value_count++;
            }
            if (register_value[e->r2] == -1)
            {
                register_value[e->r2] = value_count;
                values[value_count].type = VALUE_REGISTER;
                values[value_count].v1 = e->r2;
                value_owner[value_count] = e->r2;
                value_count++;
            }
            v.v1 = register_value[e->r1];
            v.v2 = register_value[e->r2];

            if (v.v1 > v.v2 && is_binop_commutative(e->op))
            {
                int temp = v.v1;
                v.v1 = v.v2;
                v.v2 = temp;
            }

            int j = find_value(values, value_count, &v);

            if (j == value_count)
                values[value_count++] = v;

            if (value_owner[j] != -1)
            {
                e->op = OP_MOV;
                e->r1 = value_owner[j];
            }
            else
            {
                if (values[register_value[e->r1]].type == VALUE_CONST)
                    e->r1_imm = 1, e->r1 = values[register_value[e->r1]].v1;
                else
                    e->r1 = value_owner[register_value[e->r1]];

                if (values[register_value[e->r2]].type == VALUE_CONST)
                    e->r2_imm = 1, e->r2 = values[register_value[e->r2]].v1;
                else
                    e->r2 = value_owner[register_value[e->r2]];

                if (e->r1_imm && e->r2_imm)
                {
                    e->r1 = eval_op(e->op, e->r1, e->r2);
                    e->op = OP_MOV;
                    e->r1_imm = 1;
                    Value v3 = {0};
                    v3.type = VALUE_CONST;
                    v3.v1 = e->r1;
                    j = find_value(values, value_count, &v3);
                    if (j == value_count)
                        values[value_count++] = v3;
                }
            }

            assign_register_to_value(value_owner, register_value, value_count, e->r0, j);
        }
    }
#if 0
    printf("values:\n");
    for (int j = 0; j < value_count; j++)
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

Control_Flow_Graph *gen_control_flow_graph(IR_Code *c)
{
    Control_Flow_Graph *g = calloc(1, sizeof(*g));

    IR_Basic_Block *curr_block = &g->blocks[g->block_count++];

    int label_to_block_index[256];
    memset(label_to_block_index, -1, sizeof(label_to_block_index));

    for (int i = 0; i < c->instruction_count; i++)
    {
        int label_here = 0;

        for (int j = 0; j < c->label_count; j++)
        {
            if (c->labels[j] == i)
            {
                label_to_block_index[j] = (i ? g->block_count : 0);
                label_here = 1;
            }
        }

        if (i && (label_here || c->instructions[i - 1].op == OP_JMP ||
            c->instructions[i - 1].op == OP_JMPZ))
        {
            curr_block = &g->blocks[g->block_count];
            curr_block->index = g->block_count;
            curr_block->first_instruction = i;
            g->block_count++;
        }

        IR_Instruction *e = &curr_block->instructions[curr_block->instruction_count];
        *e = c->instructions[i];

        curr_block->instruction_count++;
    }
    
    // should we increment block_count here or not?
    // because this is more like a virtual block in the end of function
    g->blocks[g->block_count].index = g->block_count;

    for (int j = 0; j < c->label_count; j++)
    {
        if (c->labels[j] == c->instruction_count)
            label_to_block_index[j] = g->block_count;
    }

    for (int i = 0; i < g->block_count; i++)
    {
        IR_Basic_Block *block = &g->blocks[i];

        // change jumps to point to block indices instead of labels
        for (int j = 0; j < block->instruction_count; j++)
        {
            if (block->instructions[j].op == OP_JMP || block->instructions[j].op == OP_JMPZ)
                block->instructions[j].r0 = label_to_block_index[block->instructions[j].r0];
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

    printf("\033[1;32mblocks generated:\033[0m\n");
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
            print_instruction(e, 1);
        }
    }

#if 1
    printf("\033[1;32moptimized:\033[0m\n");

    for (int i = 0; i < g->block_count; i++)
    {
        IR_Basic_Block *block = &g->blocks[i];

        local_value_numbering_for_basic_block(block);

        printf("optimized block %d (%d instructions, childs: [", i, block->instruction_count);
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
            print_instruction(e, 1);
        }
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
#endif
#endif


    return g;
}
