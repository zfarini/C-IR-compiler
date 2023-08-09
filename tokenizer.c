#include "compiler.h"

/*
    we propably need to deal with '\r' ?
    DON'T try to optimize / simply code until we write the preprocessor

    we can get away with generating a token on demand from parser
    instead of generate all at once but do we really care?
*/

char *find_char_in_str(char *s, char c);

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
    struct
    {
        char *name;
        int type;
    } keywords[] =
    {
        {"while",       TOKEN_WHILE},
        {"if",          TOKEN_IF},
        {"else",        TOKEN_ELSE},
        {"fn",          TOKEN_FN},
        {"int",         TOKEN_INT},
		{"return",		TOKEN_RETURN},
    };

    struct
    {
        char *name;
        int type;
    } multi_char_tokens[] =
    {
        {"==",      TOKEN_EQUAL},
        {"!=",      TOKEN_NOT_EQUAL},
        {"<=",      TOKEN_LESS_OR_EQUAL},
        {">=",      TOKEN_GREATER_OR_EQUAL},
        {"&&",      TOKEN_LOGICAL_AND},
        {"||",      TOKEN_LOGICAL_OR},
    };

	char	*single_char_tokens = "+-*/%<>()=;{}!,&";

    int max_token_count = (int)strlen(s) + 1;
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
            // @Speed
            for (int j = 0; j < array_length(keywords); j++)
            {
                if (!strcmp(token->name, keywords[j].name))
                {
                    token->type = keywords[j].type;
                    break ;
                }
            }
        }
        else {
            // @Speed
            for (int j = 0; j < array_length(multi_char_tokens); j++)
            {
                int len = (int)strlen(multi_char_tokens[j].name);
                if (!strncmp(s + i, multi_char_tokens[j].name, len))
                {
                    token->type = multi_char_tokens[j].type;
                    i += len;
                    break ;
                }
            }
            
            if (token->type == TOKEN_UNKNOWN)
            {
                if (find_char_in_str(single_char_tokens, s[i]))
                {
                    token->type = s[i];
                    i++;
                }
                else
                    error_token(token, "unkown token '%c' (ascii %d)", s[i], s[i]);
            }
        }

        token->c1 = i;
        j++;
        col += token->c1 - token->c0;
    }
    
    assert(j < max_token_count);
    tokens[j].c0 = i;

    return (tokens);
}
