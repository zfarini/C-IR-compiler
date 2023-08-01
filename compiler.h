#ifndef COMPILER_H
#define COMPILER_H

#define array_length(arr) ((int)(sizeof(arr) / sizeof(*arr)))

typedef enum {
    TOKEN_NUMBER = 256,
    TOKEN_IDENTIFIER,
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS_OR_EQ,
    TOKEN_GT_OR_EQ,
    TOKEN_UNKNOWN,
    TOKEN_WHILE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_MAX
} TokenType;

typedef struct {
    int     type;
    char    *name;
    int     value; 
    int     c0;
    int     c1;
} Token;

typedef struct Node Node;

enum NodeType {
    NODE_NUMBER,
    NODE_VAR,
    NODE_BINOP,
    NODE_UNARY,
    NODE_WHILE,
    NODE_BLOCK,
    NODE_IF,
};

struct Node {
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

typedef struct {
    Node    *nodes;
    int     first_free_node;
    Token   *tokens;
    int     curr_token;
} Parser;

enum {
    OP_ADD          = '+',
    OP_SUB          = '-',
    OP_MUL          = '*',
    OP_DIV          = '/',
    OP_MOD          = '%',
    OP_LESS         = '<',
    OP_GREATER      = '>',
    OP_MOV          = 256,
    OP_JMP,
    OP_JMPZ,
    OP_IMM_MOV,
};

typedef struct 
{
    int op;
    int r0;
    int r1;
    int r2;
} IR_Instruction;

#endif
