#include "parser.h"

#include <string.h>

#include "utils.h"

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

void skip_punc(PARSER* p, char c) {
	if (next_is_punc(p, c)) lexer_next(p->input);
	else lexer_error(p->input, "TOKEN '%c' expected", c);
}

EXPRESSION maybe_binary(PARSER* p, EXPRESSION e, int prec) {
	return e;
}

EXPRESSION maybe_unary(PARSER* p, EXPRESSION(*parser)(PARSER*)) {
	return parser(p);
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
	return maybe_binary(p, maybe_unary(p, parse_atom), 0);
}

STATEMENT parse_expr_statement(PARSER* p) {
	EXPRESSION e = parse_expr(p);
	skip_punc(p, ';');
	return (STATEMENT) { STATEMENT_TYPE_EXPR, .expr = (EXPR_STATEMENT){ e } };
}

STATEMENT parse_statement(PARSER* p) {
	return parse_expr_statement(p);
}

AST parse_ast(PARSER* p) {
	STATEMENT_VEC statements = stvec_new(10);
	while (!lexer_eof(p->input)) {
		stvec_push(&statements, parse_statement(p));
	}
	return (AST) { statements.buffer, statements.size };
}