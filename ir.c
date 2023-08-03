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
        e->r0 = r0;
        
        gen_ir(c, node->right);

        IR_Instruction *f = 0;
        int exit_label, else_label;
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

        e->r1 = (node->else_node ? else_label : exit_label);
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
        res = r1 / r2;
    else if (op == OP_MOD)
        res = r1 % r2;
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
            if (!regs[e->r0])
            {
                ip = c->labels[e->r1];
                continue ;
            }
        }
        else if (e->op < OP_BINARY)
            regs[e->r0] = eval_op(e->op, r1_value, r2_value);
        else if (e->op == OP_MOV)
            regs[e->r0] = r1_value;
        else
            assert(0);

        ip++;
    }

    printf("sim result:\n");
    for (int i = 0; c->vars_reg[i].name; i++)
    {
        printf("%s -> %d\n", c->vars_reg[i].name, regs[c->vars_reg[i].reg]);
    }
}

void print_instruction(IR_Instruction *e, int in_block)
{
    if (e->op == OP_JMP)
        printf("jmp %s%d", (in_block ? "B" : "L"), e->r0);
    else if (e->op == OP_JMPZ)
        printf("jmpz t%d, %s%d", e->r0, (in_block ? "B" : "L"), e->r1);
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
    
    printf("variables:\n");
    for (int i = 0; c->vars_reg[i].name; i++)
        printf("%s -> t%d\n", c->vars_reg[i].name, c->vars_reg[i].reg);
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
    int v1_type;
    int v2_type;
} Value;

int find_value(Value *values, int value_count, Value *to_find)
{
    for (int i = 0; i < value_count; i++)
        if (!memcmp(&values[i], to_find, sizeof(*to_find)))
            return i;

    return value_count;
}

void local_value_numbering_for_basic_block(IR_Basic_Block *b)
{

    Value   values[1024];
    int     value_count = 0; 
    
    int register_value[256];
    memset(register_value, -1, sizeof(register_value));

    for (int i = 0; i < b->instruction_count; i++)
    {
        IR_Instruction *e = &b->instructions[i];

        if (e->op == OP_MOV)
        {
            Value v = {0};
            v.type = (e->r1_imm ? VALUE_CONST : VALUE_REGISTER);
            v.v1 = e->r1;
            
            int j = find_value(values, value_count, &v);

            if (!e->r1_imm && register_value[e->r1] != -1)
                j = register_value[e->r1];
            else if (j == value_count)
                values[value_count++] = v;

            register_value[e->r0] = j;

            if (values[j].type == VALUE_CONST)
            {
                e->r1 = values[j].v1;
                e->r1_imm = 1;
            }
            else if (values[j].type == VALUE_REGISTER)
            {
                e->r1 = values[j].v1;
                assert(!e->r1_imm);
            }
        }
        else if (e->op < OP_BINARY)
        {
            Value v = {0};
            v.type = VALUE_OP;
            v.op = e->op;
            {
                Value v2 = {0};
                v2.type = (e->r1_imm ? VALUE_CONST : VALUE_REGISTER);
                v2.v1 = e->r1;

                int j = find_value(values, value_count, &v2);

                if (!e->r1_imm && register_value[e->r1] != -1)
                    j = register_value[e->r1];
                if (j < value_count)
                {
                    v.v1_type = VALUE_VALUE;
                    v.v1 = j;
                }
                else
                {
                    v.v1_type = v2.type;
                    v.v1 = e->r1;
                }
            }
            {
                Value v2 = {0};
                v2.type = (e->r2_imm ? VALUE_CONST : VALUE_REGISTER);
                v2.v1 = e->r2;

                int j = find_value(values, value_count, &v2);

                if (!e->r2_imm && register_value[e->r2] != -1)
                    j = register_value[e->r2];
                if (j < value_count)
                {
                    v.v2_type = VALUE_VALUE;
                    v.v2 = j;
                }
                else
                {
                    v.v2_type = v2.type;
                    v.v2 = e->r2;
                }
            }
    //        if (v.v1_type == VALUE_CONST && v.v2_type == VALUE_CONST)
            int j = find_value(values, value_count, &v);

            if (j == value_count)
                values[value_count++] = v;
            else
            {
                for (int k = 0; k < array_length(register_value); k++)
                {
                    if (register_value[k] == j)
                    {
                        e->op = OP_MOV;
                        e->r1_imm = 0;
                        e->r1 = k;
                        break ;
                    }
                }
            }
            if (e->op < OP_BINARY)
            {
                if (v.v1_type == VALUE_VALUE)
                {
                    if (values[v.v1].type == VALUE_CONST || 
                        values[v.v1].type == VALUE_REGISTER)
                    {
                        e->r1_imm = (values[v.v1].type == VALUE_CONST);
                        e->r1 = values[v.v1].v1;
                    }
                    else
                    {
                        for (int k = 0; k < array_length(register_value); k++)
                        {
                            if (register_value[k] == v.v1)
                            {
                                e->r1 = k;
                                e->r1_imm = 0;
                                break ;
                            }
                        }
                    }
                }
                if (v.v2_type == VALUE_VALUE)
                {
                    if (values[v.v2].type == VALUE_CONST || 
                        values[v.v2].type == VALUE_REGISTER)
                    {
                        e->r2_imm = (values[v.v2].type == VALUE_CONST);
                        e->r2 = values[v.v2].v1;
                    }
                    else
                    {
                        for (int k = 0; k < array_length(register_value); k++)
                        {
                            if (register_value[k] == v.v2)
                            {
                                e->r2 = k;
                                e->r2_imm = 0;
                                break ;
                            }
                        }
                    }
                }
            }
            register_value[e->r0] = j;
        }
    }
    printf("values:\n");
    for (int j = 0; j < value_count; j++)
    {
        printf("#%d: ", j);
        if (values[j].type == VALUE_CONST)
            printf("const(%d)", values[j].v1);
        else if (values[j].type == VALUE_REGISTER)
            printf("t%d", values[j].v1);
        else if (values[j].type == VALUE_OP)
        {
            if (values[j].v1_type == VALUE_VALUE)
                printf("#%d", values[j].v1);
            else if (values[j].v1_type == VALUE_CONST)
                printf("const(%d)", values[j].v1);
            else if (values[j].v1_type == VALUE_REGISTER)
                printf("t%d", values[j].v1);
            else
                assert(0);
                
            printf(" %s ", get_ir_op_str(values[j].op));

            if (values[j].v2_type == VALUE_VALUE)
                printf("#%d", values[j].v2);
            else if (values[j].v2_type == VALUE_CONST)
                printf("const(%d)", values[j].v2);
            else if (values[j].v2_type == VALUE_REGISTER)
                printf("t%d", values[j].v2);
            else
                assert(0);

        }
        else
            assert(0);
        printf("\n");
    }
#if 0
#if 1
    int register_use_count[256] = {0};
    for (int i = b->instruction_count - 1; i >= 0; i--)
    {
        IR_Instruction *e = &b->instructions[i];

        if ((e->op == OP_MOV || e->op < OP_BINARY) && !register_use_count[e->r0] && i != b->instruction_count - 1)
        {
            for (int j = i + 1; j < b->instruction_count; j++)
                b->instructions[j - 1] = b->instructions[j];
            b->instruction_count--;
        }
        else
        {
            if (e->op == OP_MOV && !e->r1_imm)
                register_use_count[e->r1]++;
            else if (e->op < OP_BINARY)
            {
                if (!e->r1_imm) register_use_count[e->r1]++;
                if (!e->r2_imm) register_use_count[e->r2]++;
            }
        }
    }
#else
    while (1)
    {
        int prev_inst_count = b->instruction_count;
        
        int register_used[256] = {0};
        for (int i = 0; i < b->instruction_count; i++)
        {
            IR_Instruction *e = &b->instructions[i];
            if (e->op == OP_MOV && !e->r1_imm)
                register_used[e->r1] = 1;
            else if (e->op < OP_BINARY)
            {
                if (!e->r1_imm) register_used[e->r1] = 1;
                if (!e->r2_imm) register_used[e->r2] = 1;
            }
        }
        
        if (b->instruction_count == prev_inst_count)
            break ;
    }
#endif
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
            if (block->instructions[j].op == OP_JMP)
                block->instructions[j].r0 = label_to_block_index[block->instructions[j].r0];
            if (block->instructions[j].op == OP_JMPZ)
                block->instructions[j].r1 = label_to_block_index[block->instructions[j].r1];
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

    printf("\033[1;33moptimized:\033[0m\n\n");

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
    return g;
}
