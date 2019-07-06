#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

enum EXPR_TYPE {
	EXPR_TYPE_NULL,
	EXPR_TYPE_INT,
	EXPR_TYPE_CHAR,
	EXPR_TYPE_BOOL,
	EXPR_TYPE_FLOAT,
	EXPR_TYPE_STRING,
	EXPR_TYPE_IDENTIFIER
};

typedef struct EXPRESSION_t EXPRESSION;

typedef struct INT_t {
	int64_t value;
} INT;

typedef struct CHAR_t {
	uint8_t value;
} CHAR;

typedef struct BOOL_t {
	bool value;
} BOOL;

typedef struct FLOAT_t {
	double value;
} FLOAT;

typedef struct STRING_t {
	char* value;
} STRING;

typedef struct IDENTIFIER_t {
	char* name;
} IDENTIFIER;

typedef struct EXPRESSION_t* ARGUMENT;

typedef struct FUNC_CALL_t {
	EXPRESSION* callee;
	ARGUMENT* args;
} FUNC_CALL;

typedef struct ASSIGN_t {
	uint8_t op;
	EXPRESSION* left;
	EXPRESSION* right;
} ASSIGN;

typedef struct BINARY_OP_t {
	uint8_t op;
	EXPRESSION* left;
	EXPRESSION* right;
} BINARY_OP;

typedef struct UNARY_OP_t {
	uint8_t op;
	bool position;
	EXPRESSION* expr;
} UNARY_OP;

typedef struct EXPRESSION_t {
	uint8_t type;
	union {
		INT int_literal;
		CHAR char_literal;
		BOOL bool_literal;
		FLOAT float_literal;
		STRING string_literal;
		IDENTIFIER identifier;
	};
} EXPRESSION;

#define STATEMENT_TYPE_EXPR 0x10

typedef struct EXPR_STATEMENT_t {
	EXPRESSION expr;
} EXPR_STATEMENT;

typedef struct STATEMENT_t {
	uint8_t type;
	union {
		EXPR_STATEMENT expr;
	};
} STATEMENT;

typedef struct AST_t {
	STATEMENT* statements;
	uint64_t num_statements;
} AST;
