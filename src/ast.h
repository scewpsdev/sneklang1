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
	EXPR_TYPE_COMPOUND_EXPR,

	EXPR_TYPE_ASSIGN,
	EXPR_TYPE_BINARY_OP,
	EXPR_TYPE_UNARY_OP,
	EXPR_TYPE_COMPOUND,
	EXPR_TYPE_RETURN,
	EXPR_TYPE_IF_STATEMENT,
	EXPR_TYPE_LOOP,
	EXPR_TYPE_BREAK,
	EXPR_TYPE_CONTINUE,
	EXPR_TYPE_FUNC_CALL,

	EXPR_TYPE_FUNC_DECL,

	EXPR_TYPE_IMPORT
};

typedef struct EXPRESSION_t EXPRESSION;

typedef struct TYPENAME_t {
	char* name;
	bool cpy;
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

typedef struct COMPOUND_EXPR_t {
	EXPRESSION* expr;
} COMPOUND_EXPR;

typedef struct ASSIGN_t {
	char* op;
	EXPRESSION* left;
	EXPRESSION* right;
} ASSIGN;

typedef struct BINARY_OP_t {
	char* op;
	EXPRESSION* left;
	EXPRESSION* right;
} BINARY_OP;

typedef struct UNARY_OP_t {
	char* op;
	bool position;
	EXPRESSION* expr;
} UNARY_OP;

typedef struct COMPOUND_t {
	struct AST* ast;
} COMPOUND;

typedef struct RETURN_t {
	EXPRESSION* value;
} RETURN;

typedef struct IF_t {
	EXPRESSION* condition;
	EXPRESSION* then_block;
	EXPRESSION* else_block;
} IF;

typedef struct LOOP_t {
	EXPRESSION* condition;
	EXPRESSION* body;
} LOOP;

typedef struct BREAK_t {
	uint8_t idx;
} BREAK;

typedef struct CONTINUE_t {
	uint8_t idx;
} CONTINUE;

typedef struct FUNC_CALL_t {
	EXPRESSION* callee;
	EXPRESSION* args;
	uint8_t num_args;
} FUNC_CALL;

typedef struct VAR_DECL_t {
	TYPE type;
	char* name;
} VAR_DECL;

typedef struct FUNC_DECL_t {
	char* funcname;
	VAR_DECL* args;
	uint8_t num_args;
} FUNC_DECL;

typedef struct IMPORT_t {
	char* module_name;
} IMPORT;

typedef struct EXPRESSION_t {
	uint8_t type;
	union {
		INT int_literal;
		CHAR char_literal;
		BOOL bool_literal;
		FLOAT float_literal;
		STRING string_literal;
		IDENTIFIER identifier;
		COMPOUND_EXPR compound_expr;

		ASSIGN assign;
		BINARY_OP binary_op;
		UNARY_OP unary_op;
		COMPOUND compound;
		RETURN ret_statement;
		IF if_statement;
		LOOP loop;
		BREAK break_statement;
		CONTINUE continue_statement;
		FUNC_CALL func_call;
		FUNC_DECL func_decl;

		IMPORT import;
	};
} EXPRESSION;

typedef struct AST_t {
	EXPRESSION* expressions;
	uint64_t num_expressions;
} AST;
