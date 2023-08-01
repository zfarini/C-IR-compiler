#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_ASCII 256

typedef enum {
    TOKEN_NUMBER = MAX_ASCII,
    TOKEN_UNKNOWN,
} TokenType;

typedef struct {
    int type;
    int value; 
    int c0;
    int c1;
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
        else if (ft_strchr("+-*/^()", s[i]))
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
    NODE_BINOP,
};

struct Node {
    int     type;
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
    
    return parse_expr(&p, 1);
}

enum {
    ASSOC_LEFT,
    ASSOC_RIGHT,
};

struct {
    int prec;
    int assoc;
} op_table[MAX_ASCII] = {
    ['+'] = {1, ASSOC_LEFT},
    ['-'] = {1, ASSOC_LEFT},
    ['*'] = {2, ASSOC_LEFT},
    ['/'] = {2, ASSOC_LEFT},
    ['^'] = {3, ASSOC_RIGHT},
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
    if (token->type != TOKEN_NUMBER && token->type != '(') {
        printf("error: expected an atom at position %d\n", token->c0);
        exit(1);
    }
    if (token->type == TOKEN_NUMBER) {
        res = make_node(p, NODE_NUMBER);
        skip_token(p);
    }
    else if (token->type == '(') {
        skip_token(p);
        res = parse_expr(p, 1);
        if (get_curr_token(p)->type != ')')
            printf("error: expected ')' at position %d\n", get_curr_token(p)->c0);
    }
    else
        assert(0);

    return (res);
}

Node *parse_expr(Parser *p, int min_prec)
{
    Node *res = parse_atom(p);

    while (1) {
        Token *token = get_curr_token(p);
        if (token->type >= MAX_ASCII || op_table[token->type].prec < min_prec)
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

void gen_ir(Node *node)
{
    if (node->type == NODE_NUMBER)
    {
        node->var_idx = curr_var_idx++;
        printf("x%d = %d\n", node->var_idx, node->value);
    }
    else if (node->type == NODE_BINOP)
    {
        gen_ir(node->left);
        gen_ir(node->right);
        node->var_idx = curr_var_idx++;
        printf("x%d = x%d %c x%d // %d\n", node->var_idx, node->left->var_idx, node->token->type, node->right->var_idx, node->value);
    }
    else
        assert(0);
}

int main()
{
    char *s = "2 + 3 * 2^2 - 4 * (7 / 3)";
    Token *tokens = tokenize(s);

    for (int i = 0; tokens[i].type; i++)
    {
        if (tokens[i].type == TOKEN_NUMBER)
            printf("Number(%d)", tokens[i].value);
        else if (tokens[i].type == TOKEN_UNKNOWN)
            printf("Unknown(%c)",   s[tokens[i].c0]);
        else
            printf("('%c')", tokens[i].type);
        printf(" ");
    }
    printf("\n");
    Node *node = parse(tokens);
    int res = eval(node);
    printf("res = %d\n", res);

    gen_ir(node);
}
