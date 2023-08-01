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

        curr->next_expr = parse_statement(&p);

        curr = curr->next_expr;
    }
    return res;
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
        IR_Instruction  *e = add_instruction(OP_IMM_MOV);
        e->r0 = ir_reg_curr;
        e->r1 = node->value;
        reg = ir_reg_curr++;
    }
    else if (node->type == NODE_VAR) {
        reg = get_var_register(node->token->name);
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

void *sim_ir(void *arg)
{
    (void)arg;

    int regs[128] = {};

    int ip = 0;
    while (ip < ir_inst_count)
    {
        IR_Instruction *e = &ir_code[ip];
        
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
        else if (e->op == OP_IMM_MOV)
            regs[e->r0] = e->r1;
        else if (e->op == OP_MOV)
            regs[e->r0] = regs[e->r1];
        else if (e->op == OP_ADD)
            regs[e->r0] = regs[e->r1] + regs[e->r2];
        else if (e->op == OP_SUB)
            regs[e->r0] = regs[e->r1] - regs[e->r2];
        else if (e->op == OP_MUL)
            regs[e->r0] = regs[e->r1] * regs[e->r2];
        else if (e->op == OP_DIV)
            regs[e->r0] = regs[e->r1] / regs[e->r2];
        else if (e->op == OP_MOD)
            regs[e->r0] = regs[e->r1] % regs[e->r2];
        else if (e->op == OP_LESS)
            regs[e->r0] = regs[e->r1] < regs[e->r2];
        else if (e->op == OP_GREATER)
            regs[e->r0] = regs[e->r1] > regs[e->r2];
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

    printf("IR:\n");

#if 1
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
            if (e->op == OP_IMM_MOV)
                printf("%d", e->r1);
            else if (e->op == OP_MOV)
                printf("t%d", e->r1);
            else
                printf("t%d %c t%d", e->r1, e->op, e->r2);
        }
        printf("\n");
    }
    for (int j = 0; j < curr_label; j++)
    {
        if (labels[j] == ir_inst_count)
            printf("L%d:\n", j);
        assert(labels[j] <= ir_inst_count);
    }
#endif

    pthread_t thread;
    pthread_create(&thread, 0, sim_ir, 0);

    pthread_join(thread, 0);
}
