#include "compiler.h"

static struct {
    char *name;
    int reg;
} vars_reg[128];

global IR_Instruction  ir_code[4096];
global int             ir_inst_count;
global int             ir_reg_curr;
global int             labels[256];
global int             curr_label;

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

    if (type < 256) {
        s[type][0] = type;
        return s[type];
    } else {
        for (int i = 0; i < array_length(op_str); i++)
            if (op_str[i].type == type)
                return op_str[i].name;
    }

    assert(0);
    return "UNKOWN_OP_STR";
}


int get_var_register(char *name)
{
    int i = 0;

    while (vars_reg[i].name && strcmp(vars_reg[i].name, name))
        i++;

    if (!vars_reg[i].name)
        return (-1);

    return vars_reg[i].reg;
}

void set_var_register(char *name, int reg)
{
    int i = 0;

    while (vars_reg[i].name && strcmp(vars_reg[i].name, name))
        i++;

    assert(i < array_length(vars_reg));
    vars_reg[i].name = name;
    vars_reg[i].reg = reg;
}

IR_Instruction *add_instruction(int op)
{
    IR_Instruction *e = &ir_code[ir_inst_count];

    e->op = op;
    ir_inst_count++;

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

int gen_ir(Node *node)
{
    int reg = -1;

    if (node->type == NODE_NUMBER) {
        IR_Instruction  *e = add_instruction(OP_MOV);
        e->r0 = ir_reg_curr;
        e->r1 = node->value;
        e->r1_imm = 1;
        reg = ir_reg_curr++;
    }
    else if (node->type == NODE_VAR) {
        reg = get_var_register(node->token->name);
        if (reg < 0) {
            reg = ir_reg_curr++;
            set_var_register(node->token->name, reg);
        }
    }
    else if (node->type == NODE_CALL) {
 //       IR_Instruction *e = add_instruction(OP_JMP);
    }
    else if (node->type == NODE_BINOP && node->op == '=') {
        int r1 = gen_ir(node->right);

        IR_Instruction *e = add_instruction(OP_MOV);
        e->r0 = gen_ir(node->left);
        e->r1 = r1;
        reg = e->r0;
    }
    else if (node->type == NODE_BINOP) {
        int r1 = gen_ir(node->left);
        int r2 = gen_ir(node->right);

        IR_Instruction *e = add_instruction(token_type_to_ir_op(node->op));
        e->r0 = ir_reg_curr;
        e->r1 = r1;
        e->r2 = r2;

        reg = ir_reg_curr++;
    } 
    else if (node->type == NODE_WHILE || node->type == NODE_IF) {
        int enter_label = curr_label;
        int exit_label = curr_label + 1;
        int else_label;
        
        curr_label += 2;
        labels[enter_label] = ir_inst_count;

        if (node->else_node) {
            else_label = curr_label;
            curr_label++;
        }

        int r0 = gen_ir(node->left);
        IR_Instruction *e = add_instruction(OP_JMPZ);
        e->r0 = r0;
        e->r1 = (node->else_node ? else_label : exit_label);
        
        gen_ir(node->right);

        if (node->type == NODE_WHILE) {
            e = add_instruction(OP_JMP);
            e->r0 = enter_label;
        }
        else if (node->else_node) {
            e = add_instruction(OP_JMP);
            e->r0 = exit_label;
            labels[else_label] = ir_inst_count;
            gen_ir(node->else_node);
        }

        labels[exit_label] = ir_inst_count;
    }
    else if (node->type == NODE_BLOCK) {
        Node *curr = node->first_stmt;

        while (curr) {
            gen_ir(curr);
            curr = curr->next_stmt;
        }
    }
    else
        assert(0);
    return reg;
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

void *sim_ir(void *arg)
{
    (void)arg;

    int regs[128] = {};
    int ip = 0;

    while (ip < ir_inst_count) {
        IR_Instruction *e = &ir_code[ip];
        
        int r1_value = (e->r1_imm ? e->r1 : regs[e->r1]);
        int r2_value = (e->r2_imm ? e->r2 : regs[e->r2]);

        if (e->op == OP_JMP) {
            ip = labels[e->r0];
            continue ;
        }
        else if (e->op == OP_JMPZ) {
            if (!regs[e->r0]) {
                ip = labels[e->r1];
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

    for (int i = 0; vars_reg[i].name; i++) {
        printf("%s -> %d\n", vars_reg[i].name, regs[vars_reg[i].reg]);
    }
    return (0);
}

void optimize_ir()
{
    int i = 0;

    int last_write[256] = {0};
    memset(last_write, -1, sizeof(last_write));

    int is_var_reg[256] = {0};
    for (int j = 0; vars_reg[j].name; j++)
        is_var_reg[vars_reg[j].reg] = 1;

    while (i < ir_inst_count) {
        IR_Instruction *e = &ir_code[i];

        for (int j = 0; j < curr_label; j++) {
            if (labels[j] == i) {
                memset(last_write, -1, sizeof(last_write));
                break ;
            }
        }

        if (e->op == OP_MOV) {
            if (!e->r1_imm && last_write[e->r1] != -1) {
                // if a register was used once to store and immideatly got assigned to another
                int expand = !is_var_reg[e->r1];
                for (int j = i + 1; j < ir_inst_count && expand; j++)
                    if ((!ir_code[j].r1_imm && ir_code[j].r1 == e->r1) || 
                        (!ir_code[j].r2_imm && ir_code[j].r2 == e->r1))
                        expand = 0;

                if (expand)
                {
                    int dest = e->r0;
                    *e = ir_code[last_write[e->r1]];
                    e->r0 = dest;
                }
                else if (ir_code[last_write[e->r1]].op == OP_MOV) {
                    e->r1_imm = ir_code[last_write[e->r1]].r1_imm;
                    e->r1 = ir_code[last_write[e->r1]].r1;

                }
            }
        }
#if 1
        else if (e->op < OP_BINARY) {
            if (!e->r1_imm && last_write[e->r1] != -1 && 
                    ir_code[last_write[e->r1]].op == OP_MOV) {
                e->r1_imm = ir_code[last_write[e->r1]].r1_imm;
                e->r1 = ir_code[last_write[e->r1]].r1;
            }

            if (!e->r2_imm && last_write[e->r2] != -1 && 
                    ir_code[last_write[e->r2]].op == OP_MOV) {
                e->r2_imm = ir_code[last_write[e->r2]].r1_imm;
                e->r2 = ir_code[last_write[e->r2]].r1;
            }

            if (e->r1_imm && e->r2_imm) {
                e->r1 = eval_op(e->op, e->r1, e->r2);
                e->op = OP_MOV;
            }
        }
#endif

        if (e->op != OP_JMP && e->op != OP_JMPZ)
            last_write[e->r0] = i;
        else
            memset(last_write, -1, sizeof(last_write));

        i++;
    }

#if 1

    // remove instruction that aren't read
    int last_read[256] = {0};
    memset(last_read, -1, sizeof(last_read));

    i = ir_inst_count - 1;

    while (i >= 0) {
        IR_Instruction *e = &ir_code[i];
        int r1 = e->r1, r2 = e->r2;

        if (e->op < OP_BINARY)
            ;
        else if (e->op == OP_MOV)
            r2 = -1;
        else if (e->op == OP_JMPZ)
            r1 = e->r0, r2 = -1;
        else
            r1 = -1, r2 = -1;

        if (!e->r1_imm && r1 != -1)
            last_read[r1] = i;
        if (!e->r2_imm && r2 != -1)
            last_read[r2] = i;
        
        if (e->op <= OP_MOV && last_read[e->r0] == -1 && !is_var_reg[e->r0]) {

            for (int j = 0; j < curr_label; j++) {
                if (labels[j] > i)
                    labels[j]--;
            }

            for (int j = i + 1; j < ir_inst_count; j++)
                ir_code[j - 1] = ir_code[j];
            ir_inst_count--;
        }
        if (e->op != OP_JMP && e->op != OP_JMPZ)
            last_read[e->r0] = -1;
        i--;
    }
#endif
}

void print_ir()
{
    printf("count: %d\n", ir_inst_count);

    for (int i = 0; i < ir_inst_count; i++) {
        IR_Instruction *e = &ir_code[i];

        // labels can be unordered
        for (int j = 0; j < curr_label; j++)
            if (labels[j] == i)
                printf("L%d:\n", j);

        if (e->op == OP_JMP)
            printf("jmp L%d", e->r0);
        else if (e->op == OP_JMPZ)
            printf("jmpz t%d, L%d", e->r0, e->r1);
        else {
            printf("t%d = ", e->r0);
            if (e->op == OP_MOV)
                printf("%s%d", e->r1_imm ? "" : "t", e->r1);
            else {
                printf("%s%d %s %s%d", e->r1_imm ? "" : "t", e->r1,
                        get_ir_op_str(e->op), e->r2_imm ? "" : "t", e->r2);
            }
        }
        printf("\n");
    }

    for (int j = 0; j < curr_label; j++) {
        if (labels[j] == ir_inst_count)
            printf("L%d:\n", j);
        assert(labels[j] <= ir_inst_count);
    }
}
