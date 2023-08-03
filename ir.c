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
    else if (node->type == NODE_WHILE || node->type == NODE_IF)
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

void print_ir_code(IR_Code *c)
{
    printf("count: %d\n", c->instruction_count);

    for (int i = 0; i < c->instruction_count; i++)
    {
        IR_Instruction *e = &c->instructions[i];

        for (int j = 0; j < c->label_count; j++)
        {
            if (c->labels[j] == i)
                printf("L%d:\n", j);
        }

        if (e->op == OP_JMP)
            printf("jmp L%d", e->r0);
        else if (e->op == OP_JMPZ)
            printf("jmpz t%d, L%d", e->r0, e->r1);
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

    for (int j = 0; j < c->label_count; j++)
    {
        if (c->labels[j] == c->instruction_count)
            printf("L%d:\n", j);
        assert(c->labels[j] <= c->instruction_count);
    }
    
    printf("variables:\n");
    for (int i = 0; c->vars_reg[i].name; i++)
        printf("%s -> t%d\n", c->vars_reg[i].name, c->vars_reg[i].reg);
}

void gen_basic_blocks()
{
}
