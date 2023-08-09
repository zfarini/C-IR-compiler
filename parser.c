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

Type *make_type(Parser *p, int t)
{
	Type *type = &p->types[p->first_free_type++];

	type->t = t;
	type->ptr_to = 0;
	return type;
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

Token *expect_token(Parser *p, int type)
{
	Token *token = &p->tokens[p->curr_token];

	if (token->type != type)
		error_token(token, "unexpected token"); // use type
	p->curr_token++;
	return token;
}

Node *parse_function(Parser *p);
Node *parse_statement(Parser *p);
Node *parse_expr(Parser *p, int min_prec);
Node *parse_atom(Parser *p);

void enter_scope(Parser *p)
{
    Scope *new = &p->scopes[p->scope_count++];

    new->parent = p->curr_scope;
    p->curr_scope = new;
}

void leave_scope(Parser *p)
{
    p->curr_scope = p->curr_scope->parent;
}

Node *parse(Token *tokens)
{
    Parser p = {0};

    int token_count = 0;
    while (tokens[token_count].type)
        token_count++;

    p.nodes = calloc(sizeof(*p.nodes), token_count + 1);
    p.tokens = tokens;
    p.scopes = calloc(sizeof(*p.scopes), 128);
	p.types = calloc(sizeof(*p.types), 128);
    
    enter_scope(&p);

    Node *res = parse_function(&p);

    Node *curr = res;

    while (p.tokens[p.curr_token].type)
    {
        curr->next_func = parse_function(&p);
        curr = curr->next_func;
    }

    leave_scope(&p);
    return res;
}

Node    *find_var_decl(Parser *p, char *name)
{
    Scope *curr = p->curr_scope;

    while (curr)
    {
        for (int j = 0; j < curr->decl_count; j++)
            if (!strcmp(curr->decls[j]->token->name, name)) // !1
                return curr->decls[j];
        curr = curr->parent;
    }

    return 0;
}

Node *parse_decl(Parser *p)
{
	p->curr_func->total_vars++;

	expect_token(p, TOKEN_INT);

	Type *t = make_type(p, TYPE_INT);

	while (get_curr_token(p)->type == '*')
	{
		Type *prev = t;
		t = make_type(p, TYPE_PTR);
		t->ptr_to = prev;
		skip_token(p);
	}
    Node *node = make_node(p, NODE_VAR_DECL);

    for (int i = 0; i < p->curr_scope->decl_count; i++)
        if (!strcmp(p->curr_scope->decls[i]->token->name, node->token->name))
            error_token(node->token, "redeclaration of variable '%s'", node->token->name);

    p->curr_scope->decls[p->curr_scope->decl_count] = node;
    p->curr_scope->decl_count++;

	expect_token(p, TOKEN_IDENTIFIER);
	
	return node;
}

Node *parse_function(Parser *p)
{
	expect_token(p, TOKEN_FN);

    Node *node = make_node(p, NODE_FUNC_DEF);

	p->curr_func = node;

	expect_token(p, TOKEN_IDENTIFIER);
	expect_token(p, '(');

	enter_scope(p);

	Node *curr = node;

	while (get_curr_token(p)->type != ')' 
			&& get_curr_token(p)->type)
	{
		Node *decl = parse_decl(p);
		
		if (curr == node)
			node->first_arg = decl;
		else
			curr->next_arg = decl;

		curr = decl;

		node->arg_count++;

		if (get_curr_token(p)->type == ',')
			skip_token(p);
		else if (get_curr_token(p)->type != ')')
			error_token(get_curr_token(p), "expected ',' or ')'");
	}
    // read args
	expect_token(p, ')');

	expect_token(p, '{');
	p->curr_token--; // we call expect just for the error

    node->body = parse_statement(p);

	leave_scope(p);
    return node;
}

Node *parse_statement(Parser *p)
{
    Node *node = 0;

    if (get_curr_token(p)->type == TOKEN_INT)
	{
		node = parse_decl(p);
		expect_token(p, ';');
	}
	else if (get_curr_token(p)->type == TOKEN_RETURN)
	{
		node = make_node(p, NODE_RETURN);
		skip_token(p);
		node->left = parse_expr(p, 0);
		expect_token(p, ';');
	}
    else if (get_curr_token(p)->type == TOKEN_WHILE)
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

        enter_scope(p);

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

        leave_scope(p);
    }
    else if (get_curr_token(p)->type == TOKEN_IDENTIFIER &&
             !strcmp(get_curr_token(p)->name, "print"))
    {
        node = make_node(p, NODE_PRINT);
        skip_token(p);

        node->left = parse_expr(p, 0);
		expect_token(p, ';');
    }
    else
    {
        node = parse_expr(p, 0);
		expect_token(p, ';');
    }
    return (node);
}

Node *parse_atom(Parser *p)
{
    Node *node = 0;

    Token *token = get_curr_token(p);

    if (token->type == TOKEN_NUMBER)
    {
        node = make_node(p, NODE_NUMBER);
        skip_token(p);
    }
    else if (token->type == TOKEN_IDENTIFIER)
    {
        node = make_node(p, NODE_VAR);
        skip_token(p);

        if (get_curr_token(p)->type == '(')
        {
            skip_token(p);

			Node *curr = node;

			// very similar to parse_function
			while (get_curr_token(p)->type != ')' &&
					get_curr_token(p)->type)
			{
				Node *arg = parse_expr(p, 0);

				if (curr == node)
					node->first_arg = arg;
				else
					curr->next_arg = arg;
				curr = arg;

				node->arg_count++;

				if (get_curr_token(p)->type == ',')
					skip_token(p);
				else if (get_curr_token(p)->type != ')')
					error_token(get_curr_token(p), "expected ',' or ')'");
			}
			expect_token(p, ')');
            node->type = NODE_FUNC_CALL;
        }
        else
        {
            node->decl = find_var_decl(p, node->token->name);
            if (!node->decl)
                error_token(node->token, "undeclared variable '%s'", node->token->name);
        }
    }
    else if (token->type == '(')
    {
        skip_token(p);
        node = parse_expr(p, 0);
        if (get_curr_token(p)->type != ')')
            error_token(get_curr_token(p), "expected ')'");
        else
            skip_token(p);
    }
	else if (token->type == '*')
	{
		node = make_node(p, NODE_DEREF);
		skip_token(p);
		node->left = parse_atom(p);
	}
	else if (token->type == '&')
	{
		node = make_node(p, NODE_ADDR);
		skip_token(p);
		node->left = parse_atom(p);
	}
    else if (token->type == '+')
    {
        skip_token(p);
        node = parse_atom(p);
    }
    else if (token->type == '-')
    {
        node = make_node(p, NODE_BINOP);
        skip_token(p);
        node->op = '-';
        node->left = make_node(p, NODE_NUMBER);
        node->left->value = 0;
        node->right = parse_atom(p);
    }
    else
        error_token(get_curr_token(p), "expected an expression");

    return (node);
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
