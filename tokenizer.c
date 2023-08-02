#include "compiler.h"

/*
    we propably need to deal with '\r' ?
    DON'T try to optimize / simply code until we write the preprocessor
*/
Token *tokenize(char *s)
{
    struct {
        char *name;
        int type;
    } keywords[] = {
        {"while",       TOKEN_WHILE},
        {"if",          TOKEN_IF},
        {"else",        TOKEN_ELSE},
        {"fn",          TOKEN_FN},
    };
    struct {
        char *name;
        int type;
    } multi_char_tokens[] = {
        {"==",      TOKEN_EQUAL},
        {"!=",      TOKEN_NOT_EQUAL},
        {"<=",      TOKEN_LESS_OR_EQUAL},
        {">=",      TOKEN_GREATER_OR_EQUAL},
        {"&&",      TOKEN_LOGICAL_AND},
        {"||",      TOKEN_LOGICAL_OR},
    };

    int max_token_count = strlen(s) + 1;
    Token   *tokens = calloc(sizeof(Token), max_token_count);

    int     i = 0;
    int     j = 0;
    int     line = 1;
    int     col = 1;
    
    while (s[i]) {
        while (isspace(s[i])) { // @Speed: a tab might be 4/8 spaces maybe we want something faster
                                // here
            if (s[i] == '\n') {
                line++;
                col = 1;
            }
            else
                col++;
            i++;
        }
        if (!s[i])
            break ;

        Token *token = &tokens[j];
        token->type = TOKEN_UNKNOWN;
        token->line = line;
        token->col = col;
        token->c0 = i;

        if (isdigit(s[i])) {
            token->type = TOKEN_NUMBER;
            while (isdigit(s[i])) {
                token->value = token->value * 10 + (s[i] - '0');
                i++;
            }
        }
        else if (isalpha(s[i]) || s[i] == '_') {
            token->type = TOKEN_IDENTIFIER;
            while (isalnum(s[i]) || s[i] == '_')
                i++;

            token->name = calloc(i - token->c0 + 1, 1);
            memcpy(token->name, s + token->c0, i - token->c0);
            // @Speed
            for (int j = 0; j < array_length(keywords); j++) {
                if (!strcmp(token->name, keywords[j].name)) { 
                    token->type = keywords[j].type;
                    break ;
                }
            }
        }
        else {
            // @Speed
            for (int j = 0; j < array_length(multi_char_tokens); j++) {
                int len = strlen(multi_char_tokens[j].name);
                if (!strncmp(s + i, multi_char_tokens[j].name, len)) {
                    token->type = multi_char_tokens[j].type;
                    i += len;
                    break ;
                }
            }
            
            if (token->type == TOKEN_UNKNOWN) {
                if (find_char_in_str("+-*/%<>()=;{}!", s[i])) {
                    token->type = s[i];
                    i++;
                }
                else
                    error_token(token, "unkown token '%c' (ascii %d)", s[i], s[i]); 
            }
        }

        token->c1 = i;
        j++;
        col++;
    }
    
    assert(j < max_token_count);
    tokens[j].c0 = i;

    return (tokens);
}
