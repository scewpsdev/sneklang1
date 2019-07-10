#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

enum EXPR_TYPE {
	EXPR_TYPE_NULL,
	EXPR_TYPE_INT_LITERAL,
	EXPR_TYPE_CHAR_LITERAL,
	EXPR_TYPE_BOOL_LITERAL,
	EXPR_TYPE_FLOAT_LITERAL,
	EXPR_TYPE_STRING_LITERAL,
	EXPR_TYPE_IDENTIFIER,
	EXPR_TYPE_FUNC_CALL,

	EXPR_TYPE_FUNC_DECL
};

typedef struct EXPRESSION_t EXPRESSION;

typedef struct TYPENAME_t {
	char* name;
	bool ptr;
} TYPE;

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

typedef struct FUNC_CALL_t {
	EXPRESSION* callee;
	EXPRESSION* args;
	uint8_t num_args;
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

typedef struct VAR_DECL_t {
	TYPE type;
	char* name;
} VAR_DECL;

typedef struct FUNC_DECL_t {
	char* funcname;
	VAR_DECL* args;
	uint8_t num_args;
} FUNC_DECL;

typedef struct EXPRESSION_t {
	uint8_t type;
	union {
		INT int_literal;
		CHAR char_literal;
		BOOL bool_literal;
		FLOAT float_literal;
		STRING string_literal;
		IDENTIFIER identifier;
		FUNC_CALL func_call;
		FUNC_DECL func_decl;
	};
} EXPRESSION;

typedef struct AST_t {
	EXPRESSION* expressions;
	uint64_t num_expressions;
} AST;
