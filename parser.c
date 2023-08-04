#include "compiler.h"

Node *make_node(Parser *p, int type)
{
    Node *node = &p->nodes[p->first_free_node];

    node->type = type;
    node->token = &p->tokens[p->curr_token];
    node->value = node->token->value;
    node->op = node->token->type;

    p->first_free_node++;

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
    else if (get_curr_token(p)->type == TOKEN_IDENTIFIER &&
             !strcmp(get_curr_token(p)->name, "print"))
    {
        node = make_node(p, NODE_PRINT);
        skip_token(p);

        node->left = parse_expr(p, 0);
    }
    else
    {
        node = parse_expr(p, 0);
    }
    return (node);
}

Node *parse_atom(Parser *p)
{
    Node *res = 0;

    Token *token = get_curr_token(p);

    if (token->type == TOKEN_NUMBER)
    {
        res = make_node(p, NODE_NUMBER);
        skip_token(p);
    }
    else if (token->type == TOKEN_IDENTIFIER)
    {
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
    else if (token->type == '(')
    {
        skip_token(p);
        res = parse_expr(p, 0);
        if (get_curr_token(p)->type != ')')
            error_token(get_curr_token(p), "expected ')'");
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
        error_token(get_curr_token(p), "expected an expression");
    return (res);
}

char *find_char_in_str(char *s, char c);

int is_bin_op(int type)
{
    return (find_char_in_str("+-*/%=<>", type) ||
        (type > TOKEN_BINARY_BEGIN && type < TOKEN_BINARY_END));
}

/*
    ref: https://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
    so much simpler than RD parser
*/
Node *parse_expr(Parser *p, int min_prec)
{
    enum
    {
        ASSOC_LEFT,
        ASSOC_RIGHT,
    };
    // make sure the compiler doesn't init this every time
    struct
    {
        int prec;
        int assoc;
    } op_table[TOKEN_MAX] =
    {
        ['=']                       = {100, ASSOC_RIGHT},
    
        [TOKEN_LOGICAL_AND]         = {130, ASSOC_LEFT},
        [TOKEN_LOGICAL_OR]          = {130, ASSOC_LEFT},
    
        [TOKEN_EQUAL]               = {140, ASSOC_LEFT},
        [TOKEN_NOT_EQUAL]           = {140, ASSOC_LEFT},
    
        ['<']                       = {150, ASSOC_LEFT},
        ['>']                       = {150, ASSOC_LEFT},
        [TOKEN_LESS_OR_EQUAL]       = {150, ASSOC_LEFT},
        [TOKEN_GREATER_OR_EQUAL]    = {150, ASSOC_LEFT},
    
        ['+']                       = {200, ASSOC_LEFT},
        ['-']                       = {200, ASSOC_LEFT},
    
        ['*']                       = {300, ASSOC_LEFT},
        ['/']                       = {300, ASSOC_LEFT},
        ['%']                       = {300, ASSOC_LEFT},
    };

    Node *res = parse_atom(p);
    
    while (1)
    {
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
