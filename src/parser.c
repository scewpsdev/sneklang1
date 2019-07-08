#include "parser.h"

#include <string.h>

#include "utils.h"
#include "keywords.h"

PARSER parser_new(LEXER* lexer) {
	PARSER p;
	p.input = lexer;
	return p;
}

void parser_delete(PARSER* parser) {
}

bool next_is_keyword(PARSER* p, char* kw) {
	TOKEN tok = lexer_peek(p->input);
	return tok.type != TOKEN_TYPE_NULL && tok.type == TOKEN_TYPE_KEYWORD && (kw[0] == 0 || strcmp(kw, tok.value) == 0);
}

bool next_is_punc(PARSER* p, char c) {
	TOKEN tok = lexer_peek(p->input);
	return tok.type != TOKEN_TYPE_NULL && tok.type == TOKEN_TYPE_PUNC && (c == 0 || tok.value[0] == c);
}

bool next_is_op(PARSER* p, char c) {
	TOKEN tok = lexer_peek(p->input);
	return tok.type != TOKEN_TYPE_NULL && tok.type == TOKEN_TYPE_OP && (c == 0 || tok.value[0] == c);
}

void skip_punc(PARSER* p, char c) {
	if (next_is_punc(p, c)) lexer_next(p->input);
	else lexer_error(p->input, "TOKEN '%c' expected", c);
}

void skip_op(PARSER* p, char c) {
	if (next_is_op(p, c)) lexer_next(p->input);
	else lexer_error(p->input, "TOKEN '%c' expected", c);
}

EXPRESSION parse_expr(PARSER* p);

EXPR_VEC delimited_expr(PARSER* p, char start, char end, char separator, EXPRESSION(*parser)(PARSER*)) {
	EXPR_VEC result = evec_new(2);
	bool first = true;
	if (start) skip_punc(p, start);
	while (!lexer_eof(p->input)) {
		if (next_is_punc(p, end)) break;
		if (first) first = false;
		else skip_punc(p, separator);
		if (next_is_punc(p, end)) break;
		evec_push(&result, parser(p));
	}
	skip_punc(p, end);
	return result;
}

EXPRESSION parse_call(PARSER* p, EXPRESSION func) {
	EXPR_VEC arglist = delimited_expr(p, '(', ')', ',', parse_expr);
	EXPRESSION* funcptr = malloc(sizeof(EXPRESSION));
	*funcptr = func;
	return (EXPRESSION) { EXPR_TYPE_FUNC_CALL, .func_call = (FUNC_CALL){ funcptr, arglist.buffer, arglist.size } };
}

EXPRESSION maybe_binary(PARSER* p, EXPRESSION e, int prec) {
	return e;
}

EXPRESSION maybe_unary(PARSER* p, EXPRESSION e) {
	if (next_is_punc(p, '(')) return maybe_unary(p, parse_call(p, e));
	return e;
}

EXPRESSION parse_bool(PARSER* p) {
	return (EXPRESSION) { EXPR_TYPE_BOOL, .bool_literal = { strcmp(lexer_next(p->input).value, "false") } };
}

EXPRESSION parse_atom(PARSER* p) {
	if (next_is_keyword(p, "true") || next_is_keyword(p, "false")) return parse_bool(p);

	TOKEN tok = lexer_next(p->input);
	switch (tok.type) {
	case TOKEN_TYPE_INT: return (EXPRESSION) { EXPR_TYPE_INT, .int_literal = { strtol(tok.value, NULL, 10) } };
	case TOKEN_TYPE_CHAR: return (EXPRESSION) { EXPR_TYPE_CHAR, .char_literal = { tok.value[0] } };
	case TOKEN_TYPE_FLOAT: return (EXPRESSION) { EXPR_TYPE_FLOAT, .float_literal = { strtod(tok.value, NULL) } };
	case TOKEN_TYPE_STRING: return (EXPRESSION) { EXPR_TYPE_STRING, .string_literal = { tok.value } };
	case TOKEN_TYPE_IDENTIFIER: return (EXPRESSION) { EXPR_TYPE_IDENTIFIER, .identifier = { tok.value } };
	default: return (EXPRESSION) { 0 };
	}
}

EXPRESSION parse_expr(PARSER* p) {
	return maybe_binary(p, maybe_unary(p, parse_atom(p)), 0);
}

STATEMENT parse_expr_statement(PARSER* p) {
	EXPRESSION e = parse_expr(p);
	skip_punc(p, ';');
	return (STATEMENT) { STATEMENT_TYPE_EXPR, .expr = (EXPR_STATEMENT){ e } };
}

TYPE parse_type(PARSER* p) {
	char* name = lexer_next(p->input).value;
	bool ptr = false;
	if (next_is_op(p, '*')) {
		ptr = true;
		skip_op(p, '*');
	}
	return (TYPE) { name, ptr };
}

VAR_DECL parse_arg(PARSER* p) {
	TYPE type = parse_type(p);
	char* name = NULL;
	if (lexer_peek(p->input).type == TOKEN_TYPE_IDENTIFIER) name = lexer_next(p->input).value;
	return (VAR_DECL) { type, name };
}

VAR_DECL_VEC parse_arg_list(PARSER* p) {
	VAR_DECL_VEC vec = vdvec_new(2);
	while (true) {
		VAR_DECL arg = parse_arg(p);
		vdvec_push(&vec, arg);
		if (!next_is_punc(p, ',')) break;
		skip_punc(p, ',');
	}
	return vec;
}

STATEMENT parse_func_decl(PARSER* p) {
	lexer_next(p->input);
	char* funcname = lexer_next(p->input).value;
	skip_punc(p, '(');
	VAR_DECL_VEC argvec = parse_arg_list(p);
	VAR_DECL* args = argvec.buffer;
	int num_args = argvec.size;
	skip_punc(p, ')');
	skip_punc(p, ';');
	return (STATEMENT) { STATEMENT_TYPE_FUNC_DECL, .func_decl = (FUNC_DECL){ funcname, args, num_args } };
}

STATEMENT parse_statement(PARSER* p) {
	if (next_is_keyword(p, KEYWORD_FUNC_DECL)) return parse_func_decl(p);
	return parse_expr_statement(p);
}

AST parse_ast(PARSER* p) {
	STATEMENT_VEC statements = stvec_new(10);
	while (!lexer_eof(p->input)) {
		stvec_push(&statements, parse_statement(p));
	}
	return (AST) { statements.buffer, statements.size };
}