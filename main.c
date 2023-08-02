#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "compiler.h"

char *ft_strchr(char *s, char c)
{
    while (*s)
    {
        if (*s == c)
            return s;
        s++;
    }
    return 0;
}

Token *tokenize(char *s)
{
    Token   *tokens = calloc(sizeof(Token), strlen(s) + 1);
    int     i = 0;
    int     j = 0;
    
    while (s[i]) {
        while (isspace(s[i]))
            i++;
        if (!s[i])
            break ;

        Token *token = &tokens[j];
        token->type = TOKEN_UNKNOWN;
        token->c0 = i;

        if (isdigit(s[i])) {
            token->type = TOKEN_NUMBER;
            while (isdigit(s[i])) {
                token->value = token->value * 10 + (s[i] - '0');
                i++;
            }
        }
        else if (isalpha(s[i]) || s[i] == '_')
        {
            token->type = TOKEN_IDENTIFIER;
            while (isalnum(s[i]) || s[i] == '_')
                i++;
            token->name = calloc(i - token->c0 + 1, 1);
            memcpy(token->name, s + token->c0, i - token->c0);
            if (!strcmp(token->name, "while"))
                token->type = TOKEN_WHILE;
            else if (!strcmp(token->name, "if"))
                token->type = TOKEN_IF;
            else if (!strcmp(token->name, "else"))
                token->type = TOKEN_ELSE;
            else if (!strcmp(token->name, "fn"))
                token->type = TOKEN_FN;

        }
        else if (ft_strchr("+-*/%<>()=;{}", s[i]))
        {
            token->type = s[i];
            i++;
        }
        else
        {
            printf("unknown token '%c' (ascii %d)\n", s[i], s[i]);
            exit(0);
        }
        token->c1 = i;
        j++;
    }
    tokens[j].c0 = i;
    return (tokens);
}

Node *make_node(Parser *p, int type)
{
    Node *node = &p->nodes[p->first_free_node];
    p->first_free_node++;
    node->type = type;
    node->token = &p->tokens[p->curr_token];
    node->value = node->token->value;
    node->op = node->token->type;
    return (node);
}

Token *get_curr_token(Parser *p)
{
    return &p->tokens[p->curr_token];
}

Token *skip_token(Parser *p)
{
    Token *token = &p->tokens[p->curr_token];
    
    if (token->type)
        p->curr_token++;
    return token;
}

Node *parse_function(Parser *p);
Node *parse_statement(Parser *p);
Node *parse_expr(Parser *p, int min_prec);
Node *parse_atom(Parser *p);

Node *parse(Token *tokens)
{
    Parser p;

    int token_count = 0;
    while (tokens[token_count].type)
        token_count++;

    p.nodes = calloc(sizeof(Node), token_count + 1);
    p.tokens = tokens;
    p.curr_token = 0;
    p.first_free_node = 0;
    
    Node *res = parse_statement(&p);
    Node *curr = res;

    while (p.tokens[p.curr_token].type)
    {
        if (p.tokens[p.curr_token].type == TOKEN_FN)
            curr->next_expr = parse_function(&p);
        else
            curr->next_expr = parse_statement(&p);
        curr = curr->next_expr;
    }
    return res;
}

Node *parse_function(Parser *p)
{
    skip_token(p);
    Node *node = make_node(p, NODE_FUNC);
    skip_token(p);

    skip_token(p);
    // read args
    skip_token(p);
    node->left = parse_statement(p);
    return node;
}

Node *parse_statement(Parser *p)
{
    Node *node;

    if (get_curr_token(p)->type == TOKEN_WHILE)
    {
        node = make_node(p, NODE_WHILE);
        skip_token(p);
        node->left = parse_atom(p);
        node->right = parse_statement(p);
    }
    else if (get_curr_token(p)->type == TOKEN_IF)
    {
        node = make_node(p, NODE_IF);
        skip_token(p);
        node->left = parse_atom(p);
        node->right = parse_statement(p);
        if (get_curr_token(p)->type == TOKEN_ELSE)
        {
            skip_token(p);
            node->else_node = parse_statement(p);
        }
    }
    else if (get_curr_token(p)->type == '{')
    {
        node = make_node(p, NODE_BLOCK);
        skip_token(p);
        Node *curr = node;
        
        while (get_curr_token(p)->type &&
               get_curr_token(p)->type != '}')
        {
            Node *n = parse_statement(p);
            if (curr == node)
                node->first_stmt = n;
            else
                curr->next_stmt = n;
            curr = n;
        }
        skip_token(p);
    }
    else
    {
        node = parse_expr(p, 0);
        //if (get_curr_token(p)->type == ';')
        //    skip_token(p);
        //else
        //    printf("expected ';' at the end of expression\n");
    }
    return (node);
}

Node *parse_atom(Parser *p)
{
    Node *res;

    Token *token = get_curr_token(p);
    if (token->type == TOKEN_NUMBER) {
        res = make_node(p, NODE_NUMBER);
        skip_token(p);
    }
    else if (token->type == TOKEN_IDENTIFIER) {
        res = make_node(p, NODE_VAR);
        skip_token(p);
        if (get_curr_token(p)->type == '(')
        {
            skip_token(p);
            // read args
            skip_token(p);
            res->type = NODE_CALL;
        }
    }
    else if (token->type == '(') {
        skip_token(p);
        res = parse_expr(p, 0);
        if (get_curr_token(p)->type != ')')
            printf("error: expected ')' at position %d\n", get_curr_token(p)->c0);
        else
            skip_token(p);
    }
    else if (token->type == '+')
    {
        skip_token(p);
        res = parse_atom(p);
    }
    else if (token->type == '-')
    {
        res = make_node(p, NODE_BINOP);
        skip_token(p);
        res->op = '-';
        res->left = make_node(p, NODE_NUMBER);
        res->left->value = 0;
        res->right = parse_atom(p);
    }
    else
    {
        printf("error: expected an atom at position %d\n", token->c0);
        exit(1);
    }

    return (res);
}

int is_bin_op(int type)
{
    return ft_strchr("+-*/%=<>", type) != 0;
}

enum {
    ASSOC_LEFT,
    ASSOC_RIGHT,
};

struct {
    int prec;
    int assoc;
} op_table[TOKEN_MAX] = {
    ['=']                   = {100, ASSOC_RIGHT},

    [TOKEN_LOGICAL_AND]     = {130, ASSOC_LEFT},
    [TOKEN_LOGICAL_OR]      = {130, ASSOC_LEFT},

    [TOKEN_EQUAL]           = {140, ASSOC_LEFT},
    [TOKEN_NOT_EQUAL]       = {140, ASSOC_LEFT},

    ['<']                   = {150, ASSOC_LEFT},
    ['>']                   = {150, ASSOC_LEFT},
    [TOKEN_LESS_OR_EQ]      = {150, ASSOC_LEFT},
    [TOKEN_GT_OR_EQ]        = {150, ASSOC_LEFT},

    ['+']                   = {200, ASSOC_LEFT},
    ['-']                   = {200, ASSOC_LEFT},

    ['*']                   = {300, ASSOC_LEFT},
    ['/']                   = {300, ASSOC_LEFT},
    ['%']                   = {300, ASSOC_LEFT},
};

Node *parse_expr(Parser *p, int min_prec)
{
    Node *res = parse_atom(p);
    
    while (1) {
        Token *token = get_curr_token(p);
        if (!is_bin_op(token->type) || op_table[token->type].prec < min_prec)
            break;

        int next_prec = op_table[token->type].prec;
        if (op_table[token->type].assoc == ASSOC_LEFT)
            next_prec++;

        Node *node = make_node(p, NODE_BINOP);
        node->left = res;
        skip_token(p);
        node->right = parse_expr(p, next_prec);
        res = node;
    }
    return (res);
}

static struct {
    char *name;
    int reg;
} vars_reg[128];

int get_var_register(char *name)
{
    int i = 0;

    while (vars_reg[i].name && strcmp(vars_reg[i].name, name))
        i++;
    if (!vars_reg[i].name)
    {
        return (-1);
    }
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

IR_Instruction  ir_code[4096];
int             ir_inst_count;
int             ir_reg_curr;

IR_Instruction *add_instruction(int op)
{
    IR_Instruction *e = &ir_code[ir_inst_count];
    e->op = op;
    ir_inst_count++;
    return e;
}

int labels[256];
int curr_label;

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
        if (reg < 0)
        {
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
        e->r0 = get_var_register(node->left->token->name);
        if (e->r0 < 0) {
            set_var_register(node->left->token->name, ir_reg_curr);
            e->r0 = ir_reg_curr++;
        }
        e->r1 = r1;
    }
    else if (node->type == NODE_BINOP)
    {
        int r1 = gen_ir(node->left);
        int r2 = gen_ir(node->right);

        IR_Instruction *e = add_instruction(node->op);
        e->r0 = ir_reg_curr;
        e->r1 = r1;
        e->r2 = r2;
        reg = ir_reg_curr++;
    }
    else if (node->type == NODE_WHILE || node->type == NODE_IF)
    {
        int enter_label = curr_label;
        int exit_label = curr_label + 1;
        int else_label;
        

        curr_label += 2;
        labels[enter_label] = ir_inst_count;

        if (node->else_node)
        {
            else_label = curr_label;
            curr_label++;
        }

        int r0 = gen_ir(node->left);
        IR_Instruction *e = add_instruction(OP_JMPZ);
        e->r0 = r0;
        e->r1 = (node->else_node ? else_label : exit_label);
        
        gen_ir(node->right);
        if (node->type == NODE_WHILE)
        {
            e = add_instruction(OP_JMP);
            e->r0 = enter_label;
        }
        else if (node->else_node)
        {
            e = add_instruction(OP_JMP);
            e->r0 = exit_label;
            labels[else_label] = ir_inst_count;
            printf("%d: %d\n", else_label, ir_inst_count);
            gen_ir(node->else_node);
        }

        labels[exit_label] = ir_inst_count;
    }

    else if (node->type == NODE_BLOCK)
    {
        Node *curr = node->first_stmt;
        while (curr)
        {
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
    else
        assert(0);
    return res;
}

void *sim_ir(void *arg)
{
    (void)arg;

    int regs[128] = {};

    int ip = 0;
    while (ip < ir_inst_count)
    {
        IR_Instruction *e = &ir_code[ip];
        
        int r1_value = (e->r1_imm ? e->r1 : regs[e->r1]);
        int r2_value = (e->r2_imm ? e->r2 : regs[e->r2]);

        if (e->op == OP_JMP)
        {
            ip = labels[e->r0];
            continue ;
        }
        else if (e->op == OP_JMPZ)
        {
            if (!regs[e->r0])
            {
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
    for (int i = 0; vars_reg[i].name; i++)
    {
        printf("%s -> %d\n", vars_reg[i].name, regs[vars_reg[i].reg]);
    }
    return (0);
}

char *load_entire_file(char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        printf("failed to load file: %s\n", filename);
        assert(f);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *result = malloc(length + 1);
    assert(result);
    fread(result, 1, length, f);
    result[length] = 0;
    fclose(f);
    return result;
}



void optimize_ir()
{
    int i = 0;
    
    int last_write[256] = {0};
    memset(last_write, -1, sizeof(last_write));

    int is_var_reg[256] = {0};
    for (int j = 0; vars_reg[j].name; j++)
        is_var_reg[vars_reg[j].reg] = 1;

    while (i < ir_inst_count)
    {
        IR_Instruction *e = &ir_code[i];

        for (int j = 0; j < curr_label; j++)
        {
            if (labels[j] == i)
            {
                memset(last_write, -1, sizeof(last_write));
                break ;
            }
        }
        if (e->op == OP_MOV)
        {
            if (!e->r1_imm && last_write[e->r1] != -1)
            {

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
                else if (ir_code[last_write[e->r1]].op == OP_MOV)
                {
                    e->r1_imm = ir_code[last_write[e->r1]].r1_imm;
                    e->r1 = ir_code[last_write[e->r1]].r1;

                }
            }
        }
#if 1
        else if (e->op < OP_BINARY)
        {
            if (!e->r1_imm && last_write[e->r1] != -1 && 
                    ir_code[last_write[e->r1]].op == OP_MOV)
            {
                e->r1_imm = ir_code[last_write[e->r1]].r1_imm;
                e->r1 = ir_code[last_write[e->r1]].r1;
            }
            if (!e->r2_imm && last_write[e->r2] != -1 && 
                    ir_code[last_write[e->r2]].op == OP_MOV)
            {
                e->r2_imm = ir_code[last_write[e->r2]].r1_imm;
                e->r2 = ir_code[last_write[e->r2]].r1;
            }
            if (e->r1_imm && e->r2_imm)
            {
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
    while (i >= 0)
    {
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
        
        if (e->op <= OP_MOV && last_read[e->r0] == -1 && !is_var_reg[e->r0])
        {

            for (int j = 0; j < curr_label; j++)
            {
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
    for (int i = 0; i < ir_inst_count; i++)
    {
        IR_Instruction *e = &ir_code[i];

        // labels can be unordered
        for (int j = 0; j < curr_label; j++)
            if (labels[j] == i)
                printf("L%d:\n", j);

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
                printf("%s%d %c %s%d", e->r1_imm ? "" : "t", e->r1,
                        e->op, e->r2_imm ? "" : "t", e->r2);
            }
        }
        printf("\n");
    }
    for (int j = 0; j < curr_label; j++)
    {
        if (labels[j] == ir_inst_count)
            printf("L%d:\n", j);
        assert(labels[j] <= ir_inst_count);
    }
   // printf("\n");
}
int main()
{
    char *s = load_entire_file("code.txt");
    Token *tokens = tokenize(s);

    for (int i = 0; tokens[i].type; i++)
    {
        if (tokens[i].type == TOKEN_NUMBER)
            printf("Number(%d)", tokens[i].value);
        else if (tokens[i].type == TOKEN_UNKNOWN)
            printf("Unknown(%c)",   s[tokens[i].c0]);
        else if (tokens[i].name)
            printf("Ident(%s)", tokens[i].name);
        else
            printf("('%c')", tokens[i].type);
        printf(" ");
    }
    printf("\n");
    Node *node = parse(tokens);
    while (node)
    {
        gen_ir(node);
        node = node->next_expr;
    }
    printf("variables:\n");
    for (int i = 0; vars_reg[i].name; i++)
        printf("%s -> t%d\n", vars_reg[i].name, vars_reg[i].reg);


#if 1

#endif
    printf("IR:\n");
    print_ir();

   // pthread_t thread;
   // pthread_create(&thread, 0, sim_ir, 0);
   // pthread_join(thread, 0);


   // optimize_ir();
   // printf("Optimized:\n");
   // print_ir();
   // 
   // pthread_create(&thread, 0, sim_ir, 0);
   // pthread_join(thread, 0);

}
