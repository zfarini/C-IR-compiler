#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>

#define array_length(arr) ((int)(sizeof(arr) / sizeof(*arr)))
#define global static
#define internal static
#define local static

typedef enum
{
    TOKEN_NUMBER = 256,
    TOKEN_IDENTIFIER,


    TOKEN_BINARY_BEGIN,
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS_OR_EQUAL,
    TOKEN_GREATER_OR_EQUAL,
    TOKEN_BINARY_END,

    TOKEN_UNKNOWN,
    TOKEN_WHILE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FN,
    TOKEN_MAX
} TokenType;

typedef struct
{
    int     type;
    char    *name;
    int     value; 
    int     line;
    int     col;
    int     c0;
    int     c1;
} Token;

typedef struct Node Node;

enum NodeType
{
    NODE_FUNC,
    NODE_CALL,
    NODE_NUMBER,
    NODE_VAR,
    NODE_BINOP,
    NODE_UNARY,
    NODE_WHILE,
    NODE_BLOCK,
    NODE_IF,
};

struct Node
{
    int     type;
    int     op;
    int     value;
    Node    *next_expr;
    Node    *left;
    Node    *right;
    Token   *token;
    Node    *first_stmt;
    Node    *next_stmt;
    Node    *else_node;
};

typedef struct
{
    Node    *nodes;
    int     first_free_node;
    Token   *tokens;
    int     curr_token;
} Parser;

enum
{
    OP_ADD          = '+',
    OP_SUB          = '-',
    OP_MUL          = '*',
    OP_DIV          = '/',
    OP_MOD          = '%',
    OP_LESS         = '<',
    OP_GREATER      = '>',
    OP_NOT          = '!',

    OP_EQUAL = 256,
    OP_LESS_OR_EQUAL,
    OP_GREATER_OR_EQUAL,
    OP_NOT_EQUAL,
    
    OP_BINARY,

    OP_MOV,

    OP_JMP,
    OP_JMPZ,
};

typedef struct 
{
    int op;
    int r0;
    int r1;
    int r2;
    int r1_imm;
    int r2_imm;
} IR_Instruction;

typedef struct
{
    IR_Instruction  instructions[4096];
    int             instruction_count;
    int             curr_reg;
    int             labels[256];
    int             label_count;
    struct
    {
        char *name;
        int reg;
    }               vars_reg[256];
} IR_Code;

void error_token(Token *token, char *fmt, ...);

#endif
