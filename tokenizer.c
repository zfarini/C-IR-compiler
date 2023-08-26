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
	// operators (sort by length)
    {"==",      TOKEN_EQUAL},
    {"!=",      TOKEN_NOT_EQUAL},
    {"<=",      TOKEN_LESS_OR_EQUAL},
    {">=",      TOKEN_GREATER_OR_EQUAL},
    {"&&",      TOKEN_LOGICAL_AND},
    {"||",      TOKEN_LOGICAL_OR},
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
        while (isspace(s[i]))
        {
            if (s[i] == '\n')
            {
                line++;
                col = 1;
            }
            else
                col++;
            i++;
        }
        if (s[i] == '/' && s[i + 1] == '/')
        {
            i += 2;
            while (s[i] && s[i] != '\n')
                i++;
            continue ;
        }
		else if (s[i] == '/' && s[i + 1] == '*')
		{
			i += 2;
			while (s[i] && !(s[i] == '*' && s[i + 1] == '/'))
				i++;
			//TODO: error if not
			if (s[i] == '*')
				i += 2;
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
            while (isdigit(s[i]))
            {
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
            for (int j = 0; token_typenames[j].name; j++)
            {
                if (!strcmp(token->name, token_typenames[j].name))
                {
                    token->type = token_typenames[j].type;
                    break ;
                }
            }
        }
        else {
            for (int j = 0; token_typenames[j].name; j++)
            {
                int len = (int)strlen(token_typenames[j].name);
                if (!strncmp(s + i, token_typenames[j].name, len))
                {
                    token->type = token_typenames[j].type;
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
