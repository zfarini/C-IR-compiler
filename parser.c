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

Node *make_node(Parser *p, int type)
{
    Node *node = &p->nodes[p->first_free_node++];
    
    node->type = type;
    node->token = &p->tokens[p->curr_token];
    node->op = node->token->type;
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

Node *parse_decl(Parser *p, int multiple_decls, int allow_assign)
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
        
		if (!multiple_decls)
		{
			node = curr;
			break ;
		}
		if (allow_assign && get_curr_token(p)->type == '=')
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
		Node *decl = parse_decl(p, 0, 0);
		
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
    
	if (is_typename(get_curr_token(p)->type))
	{
		node = parse_decl(p, 1, 1);
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
             (!strcmp(get_curr_token(p)->name, "print")))
    {
        node = make_node(p, NODE_PRINT);
        skip_token(p);
        
        node->left = parse_expr(p, 0);
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
            if (node->arg_count != node->decl->arg_count)
                error_token(node->token, "expected %d arguments but found %d", node->decl->arg_count, node->arg_count);
			expect_token(p, ')');
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
        node->op = '-';
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
    else
        error_token(get_curr_token(p), "expected an expression");
    
    return (node);
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
        
		if (node->token->type == '=' && !is_node_lvalue(node->left))
			error_token(node->left->token, "expected an lvalue");
        res = node;
    }
    
    return (res);
}

char *get_type_str(Type *type)
{
	
	Type *base = type;
	int redir_count = 0;
	if (type->ptr_to)
	{
		while (base->ptr_to)
		{
			base = base->ptr_to;
			redir_count++;
		}
	}
    
	char *str = 0;
    
	if		(base->t == VOID)		str = "void";
	else if (base->t == CHAR)		str = "char";
	else if (base->t == INT)		str = "int";
	else if (base->t == SHORT)		str = "short";
	else if (base->t == LONG)		str = "long";
	else if (base->t == DOUBLE)		str = "double";
	else if (base->t == FLOAT)		str = "float";
	else
		assert(0);
    
	if (base->is_unsigned)
	{
		char *new = calloc(strlen(str) + 10, 1);
		memcpy(new, "unsigned ", 9);
		memcpy(new + 9, str, strlen(str));
		str = new;
	}
	if (redir_count)
	{
		int l = (int)strlen(str);
        
		char *new = calloc(l + redir_count + 2, 1);
		memcpy(new, str, l);
		new[l] = ' ';
		for (int j = 0; j < redir_count; j++)
			new[l + j + 1] = '*';
		if (base->is_unsigned)
			free(str);
		str = new;
	}
    
	return str;
}

int types_are_equal(Type *t1, Type *t2)
{
	if (!t1 || !t2)
		return (0);
	if (t1 == t2)
		return 1;
	if (t1->is_unsigned != t2->is_unsigned)
		return 0;
	if (t1->t != t2->t)
		return 0;
	if (!t1->ptr_to)
		return (t2->ptr_to == 0);
	return types_are_equal(t1->ptr_to, t2->ptr_to);
}

Type *implicit_cast(Parser *p, Node **node, Type *type)
{
	assert(*node);
	assert((*node)->t);
	//add_type(p, *node);
	if (types_are_equal(type, (*node)->t))
		return type;
	if ((*node)->t->t == VOID)
		error_token((*node)->token, "cannot cast `void` to `%s`", get_type_str(type));
	// TODO: check if its castable
	// return in a void function?
	Node *cast = make_node(p, NODE_CAST);
	cast->left = *node;
	cast->t = type;
	*node = cast;
	return type;
}

Type *find_common_type(Type *t1, Type *t2)
{
    if (t1->t == DOUBLE || t2->t == DOUBLE)
        return t1->t == DOUBLE ? t1 : t2;
    if (t1->t == FLOAT || t2->t == FLOAT)
        return t1->t == FLOAT ? t1 : t2;
    
	if (t1->ptr_to)
		return t1;
	if (t2->ptr_to)
		return t2;
	if (t1->size < 4)
		t1 = type_int;
	if (t2->size < 4)
		t2 = type_int;
	if (t1->size != t2->size)
		return(t1->size < t2->size) ? t2 : t1;
	return t2->is_unsigned ? t2 : t1;
}

Type *add_type(Parser *p, Node *node)
{
	Type *t = type_void;
	
	if (!node)
		return 0;
	if (node->type == NODE_CAST)
		add_type(p, node->left);
	else if (node->type == NODE_VARS_DECL)
	{
		Node *curr = node->next_decl;
		while (curr)
		{
			if (curr->assign)
			{
				add_type(p, curr->assign);
				implicit_cast(p, &curr->assign, curr->t);
			}
			curr = curr->next_decl;
		}
	}
	if (node->t)
		t = node->t;	
	else if (node->type == NODE_FUNC_DEF)
	{
		add_type(p, node->body);
		// TODO: return function type
	}
	else if (node->type == NODE_FUNC_CALL)
	{
        Node *curr_param = node->decl->first_arg;
		Node *curr = node->first_arg;
        Node *prev = 0;
        
		while (curr)
		{
            add_type(p, curr);
            
            if (!types_are_equal(curr->t, curr_param->t))
            {
                Node *cast = make_node(p, NODE_CAST);
                
                cast->t = curr_param->t;
                cast->left = curr;
                cast->next_arg = curr->next_arg;
                curr->next_arg = 0;
                if (!prev)
                    node->first_arg = cast;
                else
                    prev->next_arg = cast;
                curr = cast;
            }
            prev = curr;
			curr = curr->next_arg;
            curr_param = curr_param->next_arg;
        }
        assert(!curr_param);
		t = node->decl->ret_type;
	}
	else if (node->type == NODE_NUMBER)
    {
        t = register_value_to_ctype(node->value);
    }
    else if (node->type == NODE_VAR)
		t = node->decl->t;
	else if (node->type == NODE_WHILE)
	{
		add_type(p, node->left);
		add_type(p, node->right);
	}
	else if (node->type == NODE_IF)
	{
		add_type(p, node->left);
		add_type(p, node->right);
		add_type(p, node->else_node);
	}
	else if (node->type == NODE_RETURN)
	{
		if (node->left)
		{
			add_type(p, node->left);
			implicit_cast(p, &node->left, node->function->ret_type);
		}
	}
	else if (node->type == NODE_PRINT || node->type == NODE_ASSERT)
    {
		add_type(p, node->left);
        if (node->left->t->t == VOID)
            error_token(node->left->token, "expected a number");
    }
    else if (node->type == NODE_BLOCK)
	{
		Node *curr = node->first_stmt;
		while (curr)
		{
			add_type(p, curr);
			curr = curr->next_stmt;
		}
	}
	else if (node->type == NODE_BINOP && node->token->type == '=')
	{
		t = add_type(p, node->left);
        add_type(p, node->right);
		implicit_cast(p, &node->right, t);
    }
	else if (node->type == NODE_BINOP)
	{
		Type *t1 = add_type(p, node->left);
		Type *t2 = add_type(p, node->right);
        
		if (t1->t == VOID || t2->t == VOID)
		{
			error_token(t1->t == VOID ? node->left->token
                        : node->right->token, "`void` type in binary expression `%s`", get_token_typename(node->token->type));
		}
		int tt = node->token->type;
        
		if (t1->ptr_to || t2->ptr_to)
		{
			if ((tt == '*' || tt == '/' || tt == '%') || (!t1->ptr_to && tt == '-')
				|| (t1->ptr_to && t2->ptr_to && tt == '+') 
				|| (t1->ptr_to && t2->ptr_to && tt == '-' && t1->ptr_to->size != t2->ptr_to->size)
                || (t1->ptr_to && (t2->t == FLOAT || t2->t == DOUBLE || !t1->ptr_to->size))
                || (t2->ptr_to && (t1->t == FLOAT || t1->t == DOUBLE
                                   || !t2->ptr_to->size)))
				error_token(node->token, "invalid operands to binary expression ('%s' and '%s')", get_type_str(t1), get_type_str(t2));
			if (t1->ptr_to && !t2->ptr_to && (tt == '+' || tt == '-'))
				t = t1;
			else if (t2->ptr_to && !t1->ptr_to && tt == '+')
				t = t2;
			else
				t = type_int;
		}
		else
		{
			Type *ct = find_common_type(t1, t2);
			implicit_cast(p, &node->left, ct);
			implicit_cast(p, &node->right, ct);
			if (tt == '+' || tt == '-' || tt == '*' || tt == '/' || tt == '%')
				t = ct;
			else
            {
                t = type_int;
            }
        }
	}
	else if (node->type == NODE_DEREF)
	{
		t = add_type(p, node->left);
		if (!t->ptr_to)
			error_token(node->token, "derefrencing a non-pointer");
		if (t->ptr_to->t == VOID)
			error_token(node->token, "derefrencing a void pointer");
		t = t->ptr_to;
	}
	else if (node->type == NODE_ADDR)
	{
		t = make_type(p, PTR);
		t->ptr_to = add_type(p, node->left);
		if (t->ptr_to->t == VOID)
			assert(0);
	}
	else if (node->type == NODE_NOT)
	{
		add_type(p, node->left);
		t = type_int; // TODO: 
	}
	else
		assert(0);
    node->t = t;
    return t;
}

