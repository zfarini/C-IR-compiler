Type *type_void		= &(Type){.t = VOID};
Type *type_double	= &(Type){.t = DOUBLE,	.size = 8};
Type *type_float	= &(Type){.t = FLOAT,	.size = 4};
Type *type_long		= &(Type){.t = LONG,	.size = 8};
Type *type_int		= &(Type){.t = INT,		.size = 4};
Type *type_short	= &(Type){.t = SHORT,	.size = 2};
Type *type_char		= &(Type){.t = CHAR,	.size = 1};
Type *type_ulong	= &(Type){.t = LONG,	.size = 8, .is_unsigned = 1};
Type *type_uint		= &(Type){.t = INT,		.size = 4, .is_unsigned = 1};
Type *type_ushort	= &(Type){.t = SHORT,	.size = 2, .is_unsigned = 1};
Type *type_uchar	= &(Type){.t = CHAR,	.size = 1, .is_unsigned = 1};

// TODO: change this
Token *plus_token = &(Token){.type = '+', .line = -1};
Token *minus_token = &(Token){.type = '-', .line = -1};


Type *register_value_to_ctype(RValue value)
{
    Type *t = 0;
    
    if (value.type == RV_U64) t = type_ulong;
    else if (value.type == RV_U32) t = type_uint;
    else if (value.type == RV_U16) t = type_ushort;
    else if (value.type == RV_U8) t = type_uchar;
    else if (value.type == RV_I64) t = type_long;
    else if (value.type == RV_I32) t = type_int;
    else if (value.type == RV_I16) t = type_short;
    else if (value.type == RV_I8) t = type_char;
    else if (value.type == RV_F32) t = type_float;
    else if (value.type == RV_F64) t = type_double;
    else
        assert(0);
    return t;
}

int is_bin_op(int type)
{
    return (find_char_in_str("+-*/%=<>", type) ||
            (type > TOKEN_BINARY_BEGIN && type < TOKEN_BINARY_END));
}

int is_node_lvalue(Node *node)
{
	return node->type == NODE_VAR ||
        node->type == NODE_DEREF;
}

int is_typename(int type)
{
	return type > TOKEN_BEGIN_TYPES && type < TOKEN_END_TYPES;
}

int is_type_integer(Type *t)
{
	return (t->t == INT || t->t == CHAR || t->t == SHORT || t->t == LONG);
}

Node *make_node(Parser *p, int type)
{
    Node *node = &p->nodes[p->first_free_node++];
    
    node->type = type;
    node->token = &p->tokens[p->curr_token];
	node->function = p->curr_func;
    node->value = node->token->value;
    
    return node;
}

Type *make_type(Parser *p, int t)
{
	Type *type = &p->types[p->first_free_type++];
    
	type->t = t;
    
	if (t == PTR)
		type->is_unsigned = 1;
	if (t == LONG || t == PTR || t == DOUBLE)
		type->size = 8;
	else if (t == FLOAT || t == INT)
		type->size = 4;
	else if (t == SHORT)
		type->size = 2;
	else if (t == CHAR)
		type->size = 1;
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
	{
		error_token(token, "expected token `%s` but found `%s`", 
					get_token_typename(type),
					get_token_typename(token->type));
	}
    p->curr_token++;
	return token;
}

Token *expect_curr_token(Parser *p, int type)
{
	Token *t = expect_token(p, type);
	p->curr_token--;
	return t;
}

Node *parse_function(Parser *p);
Node *parse_statement(Parser *p);
Node *parse_expr(Parser *p, int min_prec);
Node *parse_atom(Parser *p);
Type *add_type(Parser *p, Node *node);

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
    p.scopes = calloc(sizeof(*p.scopes), 512);
	p.types = calloc(sizeof(*p.types), 512);
	p.functions = calloc(sizeof(*p.functions), 128);
    
    enter_scope(&p);
    
    Node *res = parse_function(&p);
	add_type(&p, res);
    
    Node *curr = res;
    
    while (p.tokens[p.curr_token].type)
    {
        curr->next_func = parse_function(&p);
        curr = curr->next_func;
		add_type(&p, curr);
    }
    
    leave_scope(&p);
    return res;
}

Node *find_var_decl(Parser *p, char *name)
{
    Scope *curr = p->curr_scope;
    
    while (curr)
    {
        for (int j = 0; j < curr->decl_count; j++)
            if (!strcmp(curr->decls[j]->token->name, name))
            return curr->decls[j];
        curr = curr->parent;
    }
    
    return 0;
}

Node *find_function_decl(Parser *p, char *name)
{
	for (int j = 0; j < p->function_count; j++)
	{
		if (!strcmp(name, p->functions[j]->token->name))
			return p->functions[j];
	}
    
	return 0;
}

Type *parse_base_type(Parser *p)
{
	if (!is_typename(get_curr_token(p)->type))
		error_token(get_curr_token(p), "expected a typename in var declaration");
    
	Type *base_type = 0;
	Token *first_tok = get_curr_token(p);
    
	int c[TOKEN_COUNT] = {0};
    
	int last = -1;
	while (is_typename(get_curr_token(p)->type))
	{
		int t = get_curr_token(p)->type;
        
		if ((c[TOKEN_VOID] || c[TOKEN_FLOAT] || c[TOKEN_DOUBLE])
			|| (last != -1 && (t == TOKEN_VOID || t == TOKEN_FLOAT || t == TOKEN_DOUBLE)))				
			error_token(get_curr_token(p), "both `%s` and `%s` in declaration specifiers", get_token_typename(last), get_token_typename(t));
        
		c[t]++;
		if (c[t] > 1 && t != TOKEN_LONG)
			error_token(get_curr_token(p), "duplicate `%s`", get_token_typename(t));
		if (c[t] > 2 && t == TOKEN_LONG)
			error_token(get_curr_token(p), "`long long long` is too long");
		skip_token(p);
		last = t;
	}
	if (c[TOKEN_UNSIGNED] && c[TOKEN_SIGNED])
		error_token(first_tok, "both `signed` and `unsigned` in declaration specifiers");
	
	int count = 0;
	for (int i = TOKEN_BEGIN_TYPES; i < TOKEN_UNSIGNED; i++)
		if (c[i])
        count++;
    
	if (count > 1)
		error_token(first_tok, "two or more data types in declaration specifiers");
	
	int u = c[TOKEN_UNSIGNED];
    
	if		(c[TOKEN_VOID])		base_type = type_void;
	else if (c[TOKEN_FLOAT])	base_type = type_float;
	else if (c[TOKEN_DOUBLE])	base_type = type_double;
	else if (c[TOKEN_LONG])		base_type = u ? type_ulong	: type_long;
	else if (c[TOKEN_INT]) 		base_type = u ? type_uint	: type_int;
	else if (c[TOKEN_SHORT])	base_type = u ? type_ushort	: type_short;
	else if (c[TOKEN_CHAR])		base_type = u ? type_uchar	: type_char;
    else if (c[TOKEN_SIGNED]) base_type = type_int;
    else if (c[TOKEN_UNSIGNED]) base_type = type_uint;
    assert(base_type);
	return base_type;
}

Type *parse_type(Parser *p)
{
    Type *base = parse_base_type(p);
    
    Type *res = base;
    while (get_curr_token(p)->type == '*')
    {
        Type *new = &p->types[p->first_free_type++];
        
        *new = (Type){.t = PTR, .ptr_to = res, .size = 8, .is_unsigned = 1};
        res = new;
        skip_token(p);
    }
    return res;
}

Node *parse_decl(Parser *p, int in_function)
{
	Type *base_type = parse_base_type(p);
    
    Node *node = make_node(p, NODE_VARS_DECL);
	node->t = base_type;
    
	Node *curr = node;
    
	if (get_curr_token(p)->type == ';')
		return node;
	while (1)
	{
		Type *t = base_type;
        
		while (get_curr_token(p)->type == '*')
		{
			Type *new = &p->types[p->first_free_type++];
            
			new->t = PTR;
			new->ptr_to = t;
			new->size = 8;
			new->is_unsigned = 1;
			t = new;
            
			skip_token(p);
		}	

		curr->next_decl = make_node(p, NODE_VAR_DECL);
		curr = curr->next_decl;
		curr->decl = node;
		curr->t = t;
        
    	for (int i = 0; i < p->curr_scope->decl_count; i++)
    	    if (!strcmp(p->curr_scope->decls[i]->token->name, curr->token->name)
                && curr->token->type == TOKEN_IDENTIFIER)
            error_token(curr->token, "redeclaration of variable '%s'", curr->token->name);
        
    	p->curr_scope->decls[p->curr_scope->decl_count] = curr;
    	p->curr_scope->decl_count++;
        
		expect_token(p, TOKEN_IDENTIFIER);
        
		if (get_curr_token(p)->type == '[')
		{
			//!!!!!!!!!!!!!!!!!!!!!!!!!!1
			//if you want to add multi dimension arrays check a comment about them in gen_ir
			skip_token(p);
			Token *length = expect_token(p, TOKEN_NUMBER);
			if (length->value.type == RV_F32 || length->value.type == RV_F64)
				error_token(length, "expected an integer for array length");

			Type *arr = &p->types[p->first_free_type++];
			arr->t = ARRAY;
			arr->is_unsigned = 1;
			arr->size = 8;
			arr->ptr_to = t;

			// TODO: I really don't like doing this all over
			if (length->value.type == RV_U64) 	   arr->length = length->value.u64;
			else if (length->value.type == RV_U32) arr->length = length->value.u32;
			else if (length->value.type == RV_I64) arr->length = length->value.i64;
			else if (length->value.type == RV_I32) arr->length = length->value.i32;
			else assert(0);

			expect_token(p, ']');
			curr->t = arr;
		}

		if (in_function)
		{
			node = curr;
			break ;
		}
		if (get_curr_token(p)->type == '=')
		{
			skip_token(p);
			curr->assign = parse_expr(p, 0);
		}
		if (get_curr_token(p)->type == ';')
			break ;
		else if (get_curr_token(p)->type == ',')
			skip_token(p);
		else
			expect_token(p, ';');
	}
	
	return node;
}

Node *parse_function(Parser *p)
{
    Node *node = make_node(p, NODE_FUNC_DEF);
	node->ret_type = parse_type(p);
    
	node->token = get_curr_token(p);
	expect_token(p, TOKEN_IDENTIFIER);
    
	if (find_function_decl(p, node->token->name))
		error_token(node->token, "redeclartion of function `%s`", node->token->name);
    
	p->functions[p->function_count++] = node;
    
	p->curr_func = node;
    
	expect_token(p, '(');
    
	enter_scope(p);
    
	Node *curr = node;
    
	while (get_curr_token(p)->type != ')' 
           && get_curr_token(p)->type)
	{
		Node *decl = parse_decl(p, 1);
		
		if (decl->t->t == ARRAY)
			decl->t->t = PTR;
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
    
	expect_curr_token(p, '{');
    
    node->body = parse_statement(p);
    
	leave_scope(p);
    return node;
}

Node *parse_function_args(Parser *p, Node *node)
{
	expect_token(p, '(');

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

	return node->first_arg;
}

Node *parse_statement(Parser *p)
{
    Node *node = 0;
    
	if (is_typename(get_curr_token(p)->type))
	{
		node = parse_decl(p, 0);
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
		expect_curr_token(p, '(');
        node->condition = parse_atom(p);
        node->body = parse_statement(p);
    }
	else if (get_curr_token(p)->type == TOKEN_FOR)
	{
		enter_scope(p);

		node = make_node(p, NODE_FOR);
		skip_token(p);
		expect_token(p, '(');

		if (get_curr_token(p)->type != ';')
			node->decl = parse_decl(p, 0);
		expect_token(p, ';');
		node->condition = parse_expr(p, 0);
		expect_token(p, ';');
		node->increment = parse_expr(p, 0);
		expect_token(p, ')');
		node->body = parse_statement(p);

		leave_scope(p);
	}
    else if (get_curr_token(p)->type == TOKEN_IF)
    {
        node = make_node(p, NODE_IF);
        skip_token(p);
		expect_curr_token(p, '(');
        node->condition = parse_atom(p);
        node->body = parse_statement(p);
        
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
             (!strcmp(get_curr_token(p)->name, "print")))
    {
        node = make_node(p, NODE_PRINT);
        skip_token(p);
        
		parse_function_args(p, node);
		expect_token(p, ';');
    }
    else if (get_curr_token(p)->type == TOKEN_IDENTIFIER &&
             (!strcmp(get_curr_token(p)->name, "assert")))
    {
        node = make_node(p, NODE_ASSERT);
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
            node->type = NODE_FUNC_CALL;
			node->decl = find_function_decl(p, node->token->name);
			if (!node->decl)
				error_token(node->token, "undeclared function `%s`", node->token->name);

			parse_function_args(p, node);

			if (node->arg_count != node->decl->arg_count)
                error_token(node->token, "expected %d arguments but found %d", node->decl->arg_count, node->arg_count);
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
        if (is_typename(get_curr_token(p)->type))
        {
            node = make_node(p, NODE_CAST);
            
            Type *t = parse_type(p);
            
            node->t = t;
            expect_token(p, ')');
            node->left = parse_atom(p);
        }
        else
        {
            node = parse_expr(p, 0);
            expect_token(p, ')');
        }
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
		if (!is_node_lvalue(node->left))
			error_token(node->left->token, "expected an lvalue");
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
		node->token = minus_token;
        node->left = make_node(p, NODE_NUMBER);
        node->left->value = (RValue){.type = RV_I32, .i32 = 0};
        node->right = parse_atom(p);
    }
	else if (token->type == '!')
	{
		node = make_node(p, NODE_NOT);
		skip_token(p);
		node->left = parse_atom(p);
	}
	else if (token->type == TOKEN_SIZEOF)
	{
		node = make_node(p, NODE_NUMBER);
		node->value.type = RV_U32;
		skip_token(p);

		if (get_curr_token(p)->type == '('
				&& is_typename(p->tokens[p->curr_token + 1].type))
		{
			skip_token(p);
			node->value.u32 = parse_type(p)->size;
			expect_token(p, ')');
		}
		else
		{
			Node *e = parse_atom(p);

			if (e->type == NODE_VAR && e->decl->t->t == ARRAY)
				node->value.u32 = e->decl->t->length * e->decl->t->ptr_to->size;
			else
				node->value.u32 = add_type(p, e)->size;
		}
	}
    else
	{
        error_token(get_curr_token(p), "expected an expression");
	}

	if (get_curr_token(p)->type == '[')
	{
		// (e1)[e2] = *(e1 + (e2) * sizeof(*e1))
		Node *new = make_node(p, NODE_DEREF);

		skip_token(p);

		add_type(p, node);
		if (!node->t->ptr_to)
			error_token(node->token, "subscripted value is neither array nor pointer");

		Node *expr = parse_expr(p, 0);
		add_type(p, expr);
		if (!is_type_integer(expr->t))
			error_token(new->token, "expected an integer");

		new->left = make_node(p, NODE_BINOP);
		new->left->token = plus_token;
		new->left->left = node;
		new->left->right = expr;
		
		expect_token(p, ']');
		node = new;
	}

    return node;
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
    } op_table[TOKEN_COUNT] =
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
        
		if (node->token->type == '=' && (!is_node_lvalue(node->left)
			|| (node->left->type == NODE_VAR && node->left->decl->t->t == ARRAY)))
			error_token(node->left->token, "expected an lvalue");
        res = node;
    }
    
    return (res);
}

