/*
    we propably need to deal with '\r' ?
    DON'T try to optimize / simply code until we write the preprocessor

	we can generate tokens on demande from the parser but do we really care?
*/
global struct
{
	char *name;
	int type;
} token_typenames[] = {
	// keywords
    {"while",       TOKEN_WHILE},
    {"if",          TOKEN_IF},
    {"else",        TOKEN_ELSE},
    {"fn",          TOKEN_FN},
    {"int",         TOKEN_INT},
	{"char",		TOKEN_CHAR},
	{"short",		TOKEN_SHORT},
	{"long",		TOKEN_LONG},
	{"unsigned",	TOKEN_UNSIGNED},
	{"signed",		TOKEN_SIGNED},
	{"void",		TOKEN_VOID},
	{"float",		TOKEN_FLOAT},
	{"double",		TOKEN_DOUBLE},
	{"return",		TOKEN_RETURN},
	{"sizeof",		TOKEN_SIZEOF},
	{"for",			TOKEN_FOR},
	// operators (sort by length)
    {"==",      TOKEN_EQUAL},
    {"!=",      TOKEN_NOT_EQUAL},
    {"<=",      TOKEN_LESS_OR_EQUAL},
    {">=",      TOKEN_GREATER_OR_EQUAL},
    {"&&",      TOKEN_LOGICAL_AND},
    {"||",      TOKEN_LOGICAL_OR},
	{"+=",		TOKEN_ADD_EQUAL},
	{"-=",		TOKEN_SUB_EQUAL},
	{"*=",		TOKEN_MUL_EQUAL},
	{"/=",		TOKEN_DIV_EQUAL},
	{"%=",		TOKEN_MOD_EQUAL},
	{"+", '+'},
	{"-", '-'},
	{"*", '*'},
	{"/", '/'},
	{"%", '%'},
	{"<", '<'},
	{">", '>'},
	{"(", '('},
	{")", ')'},
	{"=", '='},
	{";", ';'},
	{"{", '{'},
	{"}", '}'},
	{"!", '!'},
	{",", ','},
	{"&", '&'},
	{"[", '['},
	{"]", ']'},
    
	{"end of file", 0},
	{"unknown", TOKEN_UNKNOWN},
	{"identifier", TOKEN_IDENTIFIER},
	{"number", TOKEN_NUMBER},
	{0, 0},
};

char *get_token_typename(int type)
{
	for (int i = 0; token_typenames[i].name; i++)
		if (token_typenames[i].type == type)
        return token_typenames[i].name;
	assert(0);
	return "UNDEFINED";
}

void error_token(Token *token, char *fmt, ...)
{
    fprintf(stderr, "\033[1;37m%d:%d: \033[1;31merror: \033[1;37m",
            token->line, token->col);
    
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    
    fprintf(stderr, "\033[0m\n");
    exit(1);
}

void skip_chars(char *s, int *i, int *line, int *col, int count)
{
	while (count)
	{
		if (s[*i] == '\n')
		{
			*line = *line + 1;
			*col = 1;
		}
		else
			*col = *col + 1;
		*i = *i + 1;
		count--;
	}
}

char get_backspaced_char(char c)
{
	switch (c)
    {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'f': return '\f';
        case 'b': return '\b';
        case 'a': return '\a';
        default : return c;
    }
}

Token *tokenize(char *s)
{
    int 	max_token_count = (int)strlen(s) + 1;
    Token   *tokens = calloc(sizeof(Token), max_token_count);
    
    int     i = 0;
    int     j = 0;
    int     line = 1;
    int     col = 1;
    
    while (s[i])
    {
        // @Speed: a tab might be 4/8 spaces maybe we want something faster
        while (isspace(s[i]) || s[i] == '\\')
            skip_chars(s, &i, &line, &col, 1);
        if (s[i] == '/' && s[i + 1] == '/')
        {
            while (s[i] && s[i] != '\n')
                skip_chars(s, &i, &line, &col, 1);
            continue ;
        }
        else if (s[i] == '/' && s[i + 1] == '*')
        {
            skip_chars(s, &i, &line, &col, 2);
            while (s[i] && !(s[i] == '*' && s[i + 1] == '/'))
                skip_chars(s, &i, &line, &col, 1);
            //TODO: error if not
            if (s[i] == '*')
                skip_chars(s, &i, &line, &col, 2);
            continue ;
        }
        if (!s[i])
            break ;
        
        Token *token = &tokens[j];
        token->type = TOKEN_UNKNOWN;
        token->line = line;
        token->col = col;
        token->c0 = i;
        
        if (isdigit(s[i]))
        {
            token->type = TOKEN_NUMBER;
            int base = 10;
            
            if (s[i] == '0')
            {
                i++;
                if (s[i] == 'x')
                    base = 16, i++;
                else if (s[i] == 'b')
                    base = 2, i++;
                else if (isdigit(s[i]))
                    base = 8;
            }
            uint64_t value = 0;
            
            while (1)
            {
                char c = tolower(s[i]);
                
                if (!isalnum(c) || !(c - '0' < base || (base == 16 && c >= 'a' && c <= 'f')))
                    break ;
                int d = (c >= 'a' ? c - 'a' + 10 : c - '0');
                value = value * base + d;
                i++;
            }
            if (s[i] == '.')
            {
                i++;
                if (base != 10)
                    error_token(token, "expected base 10 in floating-point number");
                
                while (isdigit(s[i]))
                    i++;
                double fvalue = atof(s + token->c0);
                if (s[i] == 'f')
                {
                    i++;
                    token->value.type = RV_F32;
                    token->value.f32 = fvalue;
                }
                else
                {
                    token->value.type = RV_F64;
                    token->value.f64 = fvalue;
                }
				//printf("line %d: %.20lf\n", line, fvalue);
            }
            else
            {
                int type = RV_I32;
                // TODO: check this
                if (value > INT32_MAX && value <= UINT32_MAX)
                    type = RV_U32;
                else if (value > UINT32_MAX && value <= INT64_MAX)
                    type = RV_I64;
                else if (value > INT64_MAX)
                    type = RV_U64;
                
                int u_suffix = 0;
                int l_suffix = 0;
                
                while (!u_suffix || !l_suffix)
                {
                    if (tolower(s[i]) == 'u')
                    {
                        i++;
                        u_suffix = 1;
                    }
                    else if (tolower(s[i]) == 'l')
                    {
                        i++;
                        l_suffix = 1;
                        if (tolower(s[i]) == 'l')
                            i++;
                    }
                    else
                        break;
                }
                if (l_suffix && u_suffix)
                    type = RV_U64;
                else if (l_suffix)
                    type = RV_I64;
                else if (u_suffix)
                    type = RV_U32;
                token->value.type = type;
                // TODO: is this just token->value.u64 = value? propably not
                if (type == RV_I32) token->value.i32 = value;
                else if (type == RV_U32) token->value.u32 = value;
                else if (type == RV_I64) token->value.i64 = value;
                else if (type == RV_U64) token->value.u64 = value;
                else assert(0);
            }
            
        }
        else if (isalpha(s[i]) || s[i] == '_')
        {
            token->type = TOKEN_IDENTIFIER;
            while (isalnum(s[i]) || s[i] == '_')
                i++;
            
            token->name = calloc(i - token->c0 + 1, 1);
            memcpy(token->name, s + token->c0, i - token->c0);
            for (int k = 0; token_typenames[k].name; k++)
            {
                if (!strcmp(token->name, token_typenames[k].name))
                {
                    token->type = token_typenames[k].type;
                    break ;
                }
            }
        }
        else if (s[i] == '\'')
        {
            i++;
            token->type = TOKEN_NUMBER;
            token->value.type = RV_I32;
            if (s[i] == '\\')
            {
                i++;
				token->value.i32 = get_backspaced_char(s[i]); 
            }
            else
                token->value.i32 = s[i];
            i++;
            if (s[i] != '\'')
                error_token(token, "expected token `'`");
            i++;
        }
		else if (s[i] == '"')
		{
			i++;
			token->type = TOKEN_STRING;
			int len = 0;
			while (s[i] != '"' && s[i])
			{
				if (s[i] == '\\')
					i++;
				i++;
				len++;
			}
			if (!s[i])
				error_token(token, "expected token `\"`");
			i++;
			token->str = calloc(1, len + 1);
			int j = token->c0 + 1, k = 0;
			while (k < len)
			{
				if (s[j] == '\\')
				{
					j++;
					token->str[k] = get_backspaced_char(s[j]);
				}
				else
					token->str[k] = s[j];
				k++;
				j++;
			}
		}
        else {
            for (int k = 0; token_typenames[k].name; k++)
            {
                int len = (int)strlen(token_typenames[k].name);
                if (!strncmp(s + i, token_typenames[k].name, len))
                {
                    token->type = token_typenames[k].type;
                    i += len;
                    break ;
                }
            }
            
            if (token->type == TOKEN_UNKNOWN)
                error_token(token, "unkown token '%c' (ascii %d)", s[i], s[i]);
        }
        
        token->c1 = i;
        j++;
        col += token->c1 - token->c0;
    }
    
    assert(j < max_token_count);
    tokens[j].c0 = i;
    
    return (tokens);
}
