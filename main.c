#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_ASCII 256

typedef enum {
    TOKEN_NUMBER = MAX_ASCII,
    TOKEN_IDENTIFIER,
    TOKEN_UNKNOWN,
} TokenType;

typedef struct {
    int     type;
    char    *name;
    int     value; 
    int     c0;
    int     c1;
} Token;

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
        }
        else if (ft_strchr("+-*/^()=", s[i]))
        {
            token->type = s[i];
            i++;
        }
        else
            printf("unknown token '%c' (ascii %d)\n", s[i], s[i]);
        token->c1 = i;
        j++;
    }
    tokens[j].c0 = i;
    return (tokens);
}

typedef struct Node Node;

enum NodeType {
    NODE_NUMBER,
    NODE_VAR,
    NODE_BINOP,
};

struct Node {
    int     type;
    Node    *next_expr;
    Node    *left;
    Node    *right;
    Token   *token;
    int     var_idx;
    int     value;
};

typedef struct {
    Node    *nodes;
    int     first_free_node;
    Token   *tokens;
    int     curr_token;
} Parser;

Node *make_node(Parser *p, int type)
{
    Node *node = &p->nodes[p->first_free_node];
    p->first_free_node++;
    node->type = type;
    node->token = &p->tokens[p->curr_token];
    return (node);
}

Node *parse_expr(Parser *p, int min_prec);

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
    
    Node *res = parse_expr(&p, 1);
    Node *curr = res;

    while (p.tokens[p.curr_token].type)
    {
        curr->next_expr = parse_expr(&p, 0);
        curr = curr->next_expr;
    }
    return res;
}

enum {
    ASSOC_LEFT,
    ASSOC_RIGHT,
};

struct {
    int prec;
    int assoc;
} op_table[MAX_ASCII] = {
    ['='] = {1, ASSOC_RIGHT},
    ['+'] = {2, ASSOC_LEFT},
    ['-'] = {2, ASSOC_LEFT},
    ['*'] = {3, ASSOC_LEFT},
    ['/'] = {3, ASSOC_LEFT},
    ['^'] = {4, ASSOC_RIGHT},
};

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
    return ft_strchr("+-*/^=", type) != 0;
}

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

int eval(Node *node)
{
    int res;

    if (node->type == NODE_NUMBER)
    {
        node->value = node->token->value;
        res = node->value;
    }
    else if (node->type == NODE_BINOP)
    {
        int l = eval(node->left);
        int r = eval(node->right);

        switch (node->token->type)
        {
            case '+':
                res =  l + r;
                break ;
            case '-':
                res =  l - r;
                break ;
            case '*':
                res =  l * r;
                break ;
            case '/':
                res =  l / r;
                break ;
            case '^':
            {
                res = 1;
                while (r)
                {
                    res *= l;
                    r--;
                }
                break ;
            }
        }
    }
    else
        assert(0);
    node->value = res;
    return res;
}

int curr_var_idx = 0;

static struct {
    char *name;
    int reg;
} vars_reg[128];

#define array_length(arr) ((int)(sizeof(arr) / sizeof(*arr)))

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

enum {
    OP_MOV,
    OP_IMM_MOV,
    OP_ADD = '+',
    OP_SUB = '-',
    OP_MUL = '*',
    OP_DIV = '/',
};

typedef struct 
{
    int op;
    int r0;
    int r1;
    int r2;
} IR_Instruction;

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

int gen_ir(Node *node)
{
    int reg = -1;

    if (node->type == NODE_NUMBER) {
        IR_Instruction  *e = add_instruction(OP_IMM_MOV);
        e->r0 = ir_reg_curr;
        e->r1 = node->token->value;
        reg = ir_reg_curr++;
    }
    else if (node->type == NODE_VAR) {
        reg = get_var_register(node->token->name);
    }
    else if (node->type == NODE_BINOP && node->token->type == '=') {
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

        IR_Instruction *e = add_instruction(node->token->type);
        e->r0 = ir_reg_curr;
        e->r1 = r1;
        e->r2 = r2;
        reg = ir_reg_curr++;
    }
    else
        assert(0);
    return reg;
}

int main()
{
    char *s = "a = 5\nb = 6\n";
    Token *tokens = tokenize(s);

    for (int i = 0; tokens[i].type; i++)
    {
        if (tokens[i].type == TOKEN_NUMBER)
            printf("Number(%d)", tokens[i].value);
        else if (tokens[i].type == TOKEN_UNKNOWN)
            printf("Unknown(%c)",   s[tokens[i].c0]);
        else if (tokens[i].type == TOKEN_IDENTIFIER)
            printf("Ident(%s)", tokens[i].name);
        else
            printf("('%c')", tokens[i].type);
        printf(" ");
    }
    printf("\n");
    Node *node = parse(tokens);
 //   int res = eval(node);
   // printf("res = %d\n", res);

    while (node)
    {
        gen_ir(node);
        node = node->next_expr;
    }
    printf("variables:\n");
    for (int i = 0; vars_reg[i].name; i++)
        printf("%s -> t%d\n", vars_reg[i].name, vars_reg[i].reg);

    printf("IR:\n");

    for (int i = 0; i < ir_inst_count; i++)
    {
        IR_Instruction *e = &ir_code[i];

        printf("t%d = ", e->r0);
        if (e->op == OP_IMM_MOV)
            printf("%d", e->r1);
        else if (e->op == OP_MOV)
            printf("t%d", e->r1);
        else
            printf("t%d %c t%d", e->r1, e->op, e->r2);
        printf("\n");
    }

}
