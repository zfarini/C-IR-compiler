#ifndef COMPILER_H
#define COMPILER_H

#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#endif
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>



#define array_length(arr) ((int)(sizeof(arr) / sizeof(*arr)))
#undef max
#undef min
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define global static
#define internal static
#define local static

typedef float float32;
typedef double float64;

enum RValue_Type
{
	RV_U64 = 1,
    RV_U32,
    RV_U16,
    RV_U8,
    RV_I64,
    RV_I32,
    RV_I16,
    RV_I8,
    RV_PTR,
    RV_F32,
    RV_F64,
    RVALUE_COUNT,
};

typedef struct RValue {
	union {
		uint64_t u64;
		uint32_t u32;
		uint16_t u16;
		uint8_t	 u8;
		int64_t  i64;
		int32_t  i32;
		int16_t  i16;
		int8_t	 i8;
        float f32;
        double f64;
	};
	int type;
} RValue;

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
    
    TOKEN_WHILE,
	TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
	TOKEN_SIZEOF,
    
	TOKEN_BEGIN_TYPES,
    TOKEN_INT,
	TOKEN_CHAR,
	TOKEN_SHORT,
	TOKEN_LONG,
	TOKEN_VOID,
	TOKEN_FLOAT,
	TOKEN_DOUBLE,
	TOKEN_UNSIGNED,
	TOKEN_SIGNED,
	TOKEN_END_TYPES,
    
    TOKEN_FN,
    
    TOKEN_UNKNOWN,
    
    TOKEN_COUNT
} TokenType;

typedef struct
{
    int     type;
    char    *name;
    RValue  value;
    int     line;
    int     col;
    int     c0;
    int     c1;
} Token;

typedef struct Node Node;

enum NodeType
{
    NODE_FUNC_DEF = 256,
    NODE_FUNC_CALL,
    NODE_NUMBER,
    NODE_VAR,
    NODE_BINOP,
    NODE_WHILE,
    NODE_BLOCK,
    NODE_IF,
    NODE_PRINT,
	NODE_ASSERT,
	NODE_VARS_DECL,
    NODE_VAR_DECL,
	NODE_RETURN,
	NODE_DEREF,
	NODE_ADDR,
	NODE_CAST,
	NODE_NOT,
	NODE_SUBSCRIPT,
};

typedef struct Type Type;

enum {
	PTR,
	VOID,
	INT,
	CHAR,
	SHORT,
	LONG,
	FLOAT,
	DOUBLE,
	ARRAY,
};

struct Type
{
	int	t;
	int	is_unsigned;
	uint64_t	size;
	uint64_t length;
	Type *ptr_to;
};

struct Node
{
	Type	*t;
	Type	*ret_type;
    
    RValue value;
    int     type;
    int     op;
    Node    *body;
    Node    *next_expr;
    Node    *left;
    Node    *right;
    Token   *token;
    Node    *first_stmt;
    Node    *next_stmt;
    Node    *else_node;
    Node    *decl;
	Node	*next_func;
    
    
	Node	*next_decl;
	Node	*assign;
    
	Node	*first_arg;
	Node	*next_arg;
	int		arg_count;
    uint64_t stack_offset; // NODE_FUNC_DEF
    uint64_t arg_offset; // NODE_FUNC_CALL
    
	int		var_index;
	int		func_index;
    
	Node	*function;
};

typedef struct Scope Scope;


struct Scope
{
    Node    *decls[128];
    int     decl_count;
    Scope   *parent;
};

typedef struct
{
    Node    *nodes;
    int     first_free_node;
    Token   *tokens;
    int     curr_token;
    Scope   *curr_scope;
    Scope   *scopes;
    int     scope_count;
    
	Node	**functions;
	int	     function_count;
    
	Type	*types;
	int 	first_free_type;
	Node	*curr_func;
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
    
    
    OP_EQUAL = 256,
    OP_LESS_OR_EQUAL,
    OP_GREATER_OR_EQUAL,
    OP_NOT_EQUAL,
    
    OP_BINARY,
    
    OP_NOT,
    
    OP_MOV,
    
    OP_JMP,
    OP_JMPZ,
    OP_PRINT,
    OP_CALL,
    OP_RET,
	OP_LOAD,
	OP_STORE,
	OP_ASSERT,
	OP_CAST,
};

enum
{
	REG_RT,
	REG_SP,
	REG_COUNT,
	REG_ARG0 = REG_COUNT,
};

typedef struct
{
	Type *type;
	union {
		int		i;
		RValue	value;
	};
	int	 imm;
} Register;

typedef struct
{
    int op;
    
	Register r0, r1, r2;
	int label;
    //    int r0;
    //    int r1;
    //    int r2;
    //
    //    int r1_imm;
    //    int r2_imm;
    //
	Node	*node;
} IR_Instruction;

typedef struct Control_Flow_Graph Control_Flow_Graph;

typedef struct
{
	Node				*decl;
	char				*name;
	int					first_instruction;
	int					instruction_count;
	int					label;
	int					exit_label;
    uint64_t stack_size;
	Control_Flow_Graph *cfg;
} Function;

typedef struct
{
    IR_Instruction  *instructions;
    int             instruction_count;
    int             curr_reg;
	int				reserved_reg;
    int             *labels;
    int             label_count;
    
	Function		*functions;
	int				function_count;
	Function		*curr_func;
    
	Node			*curr_node;
    
	Register		*vars_reg;
	int				var_count;
	int				func_index;
} IR_Code;

typedef struct IR_Basic_Block IR_Basic_Block;

struct IR_Basic_Block
{
    IR_Instruction  instructions[512];
    int             instruction_count;
    IR_Basic_Block  *childs[2];
    int             child_count;
    int             index;
};

struct Control_Flow_Graph
{
    IR_Basic_Block  blocks[32];
    int             block_count;
};

void error_token(Token *token, char *fmt, ...);

#endif
