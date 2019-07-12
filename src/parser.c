#include "parser.h"

#include <string.h>

#include "utils.h"
#include "keywords.h"

const char* OP_MAP_K[] = { "=", "+=", "-=", "*=", "/=", "%=", "||", "&&", "<", ">", "<=", ">=", "==", "!=", "+", "-", "*", "/", "%" };
const int OP_MAP_V[] = { 1, 1, 1, 1, 1, 1, 2, 3, 7, 7, 7, 7, 7, 7, 10, 10, 20, 20, 20 };
const int NUM_OPS = 19;

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

bool next_is_op(PARSER* p, char* c) {
	TOKEN tok = lexer_peek(p->input);
	return tok.type != TOKEN_TYPE_NULL && tok.type == TOKEN_TYPE_OP && (c[0] == 0 || strcmp(tok.value, c) == 0);
}

void skip_separator(PARSER* p) {
	if (lexer_peek(p->input).type == TOKEN_TYPE_SEPARATOR) lexer_next(p->input);
	else lexer_error(p->input, "SEPARATOR expected");
}

void skip_punc(PARSER* p, char c) {
	if (next_is_punc(p, c)) lexer_next(p->input);
	else lexer_error(p->input, "TOKEN '%c' expected", c);
}

void skip_op(PARSER* p, char* op) {
	if (next_is_op(p, op)) lexer_next(p->input);
	else lexer_error(p->input, "TOKEN '%s' expected", op);
}

int get_op_precedence(char* op) {
	for (int i = 0; i < NUM_OPS; i++) {
		if (strcmp(OP_MAP_K[i], op) == 0) return OP_MAP_V[i];
	}
	return -1;
}

EXPRESSION parse_expr(PARSER* p);
EXPRESSION parse_atom(PARSER* p);
EXPRESSION parse_call(PARSER* p, EXPRESSION func);

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

EXPRESSION maybe_unary(PARSER* p, EXPRESSION e) {
	if (next_is_punc(p, '(')) return maybe_unary(p, parse_call(p, e));
	return e;
}

EXPRESSION maybe_binary(PARSER* p, EXPRESSION e, int prec) {
	if (next_is_op(p, "")) {
		TOKEN token = lexer_peek(p->input);
		int token_prec = get_op_precedence(token.value);
		if (token_prec > prec) {
			lexer_next(p->input);
			EXPRESSION* left = malloc(sizeof(EXPRESSION));
			EXPRESSION* right = malloc(sizeof(EXPRESSION));
			*left = e;
			*right = maybe_binary(p, maybe_unary(p, parse_atom(p)), token_prec);
			if (strcmp(token.value, "=") == 0
				|| strlen(token.value) == 2 && (token.value[0] == '+' || token.value[0] == '-' || token.value[0] == '*' || token.value[0] == '/' || token.value[0] == '%') && token.value[1] == '=') {
				return maybe_unary(p, maybe_binary(p, (EXPRESSION) { EXPR_TYPE_ASSIGN, .assign = { token.value, left, right } }, prec));
			} else {
				return maybe_unary(p, maybe_binary(p, (EXPRESSION) { EXPR_TYPE_BINARY_OP, .binary_op = (BINARY_OP){ token.value, left, right } }, prec));
			}
		}
	}
	return e;
}

EXPRESSION parse_bool(PARSER* p) {
	return (EXPRESSION) { EXPR_TYPE_BOOL_LITERAL, .bool_literal = { strcmp(lexer_next(p->input).value, "false") } };
}

EXPRESSION parse_call(PARSER* p, EXPRESSION func) {
	EXPR_VEC arglist = delimited_expr(p, '(', ')', ',', parse_expr);
	EXPRESSION* funcptr = malloc(sizeof(EXPRESSION));
	*funcptr = func;
	return (EXPRESSION) { EXPR_TYPE_FUNC_CALL, .func_call = (FUNC_CALL){ funcptr, arglist.buffer, arglist.size } };
}

TYPE parse_type(PARSER* p) {
	char* name = NULL;
	bool cpy = false;
	if (next_is_op(p, "*")) {
		cpy = true;
		skip_op(p, "*");
	}
	name = lexer_next(p->input).value;

	return (TYPE) { name, cpy };
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

EXPRESSION parse_func_decl(PARSER* p) {
	lexer_next(p->input);
	char* funcname = lexer_next(p->input).value;
	skip_punc(p, '(');
	VAR_DECL_VEC argvec = parse_arg_list(p);
	VAR_DECL* args = argvec.buffer;
	int num_args = argvec.size;
	skip_punc(p, ')');
	return (EXPRESSION) { EXPR_TYPE_FUNC_DECL, .func_decl = (FUNC_DECL){ funcname, args, num_args } };
}

EXPRESSION parse_atom(PARSER* p) {
	while (lexer_peek(p->input).type == TOKEN_TYPE_SEPARATOR) lexer_next(p->input);

	if (next_is_keyword(p, "true") || next_is_keyword(p, "false")) return parse_bool(p);
	if (next_is_op(p, "*")) {
		char* op = lexer_next(p->input).value;
		EXPRESSION* expr = malloc(sizeof(EXPRESSION));
		*expr = maybe_unary(p, parse_atom(p));
		return (EXPRESSION) { EXPR_TYPE_UNARY_OP, .unary_op = (UNARY_OP){ op, false, expr } };
	}
	if (next_is_keyword(p, KEYWORD_FUNC_DECL)) return parse_func_decl(p);

	TOKEN tok = lexer_next(p->input);
	switch (tok.type) {
	case TOKEN_TYPE_INT: return (EXPRESSION) { EXPR_TYPE_INT_LITERAL, .int_literal = { strtol(tok.value, NULL, 10) } };
	case TOKEN_TYPE_CHAR: return (EXPRESSION) { EXPR_TYPE_CHAR_LITERAL, .char_literal = { tok.value[0] } };
	case TOKEN_TYPE_FLOAT: return (EXPRESSION) { EXPR_TYPE_FLOAT_LITERAL, .float_literal = { strtod(tok.value, NULL) } };
	case TOKEN_TYPE_STRING: return (EXPRESSION) { EXPR_TYPE_STRING_LITERAL, .string_literal = { tok.value } };
	case TOKEN_TYPE_IDENTIFIER: return (EXPRESSION) { EXPR_TYPE_IDENTIFIER, .identifier = { tok.value } };
	default: return (EXPRESSION) { 0 };
	}
}

EXPRESSION parse_expr(PARSER* p) {
	EXPRESSION atom = parse_atom(p);
	if (atom.type == TOKEN_TYPE_NULL) return parse_expr(p);
	else return maybe_binary(p, maybe_unary(p, atom), 0);
}

AST parse_ast(PARSER* p) {
	EXPR_VEC expressions = evec_new(10);
	while (!lexer_eof(p->input)) {
		evec_push(&expressions, parse_expr(p));
		if (!lexer_eof(p->input)) skip_separator(p);
	}
	return (AST) { expressions.buffer, expressions.size };
}