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

int is_castable(Type *from, Type *to)
{
	if (from->t == VOID && to->t != VOID)
		return 0;
	return 1;
}

Type *implicit_cast(Parser *p, Node **node, Type *type)
{
	assert(*node);
	assert((*node)->t);
	//add_type(p, *node);
	if (types_are_equal(type, (*node)->t))
		return type;
	if (!is_castable((*node)->t, type))
		error_token((*node)->token, "cannot cast from '%s' to '%s'",
					get_type_str((*node)->t), get_type_str(type));
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
	{
		Type *t1 = add_type(p, node->left);

		if (!is_castable(t1, node->t))
			error_token(node->left->token, "cannot cast from '%s' to '%s'",
					get_type_str(t1), get_type_str(node->t));
	}
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
	{
		if (node->decl->t->t == ARRAY)
		{
			t = make_type(p, PTR);
			*t = *node->decl->t;
			t->t = PTR;
		}
		else
			t = node->decl->t;
	}
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
				error_token(node->token, "invalid operands to binary expression `%s` ('%s' and '%s')", get_token_typename(node->token->type), get_type_str(t1), get_type_str(t2));
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
		add_type(p, node->left);

		if (node->left->type == NODE_VAR &&
			node->left->decl->t->t == ARRAY)
		{
			t = make_type(p, PTR);
			*t = *node->left->decl->t;
			t->t = PTR;
		}
		else
		{
			t = make_type(p, PTR);
			t->ptr_to = node->left->t;
		}
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

