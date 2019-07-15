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

bool next_is_keyword(PARSER* p, const char* kw) {
	TOKEN tok = lexer_peek(p->input);
	return tok.type != TOKEN_TYPE_NULL && tok.type == TOKEN_TYPE_KEYWORD && (kw[0] == 0 || strcmp(kw, tok.value) == 0);
}

bool next_is_punc(PARSER* p, const char c) {
	TOKEN tok = lexer_peek(p->input);
	return tok.type != TOKEN_TYPE_NULL && tok.type == TOKEN_TYPE_PUNC && (c == 0 || tok.value[0] == c);
}

bool next_is_op(PARSER* p, const char* c) {
	TOKEN tok = lexer_peek(p->input);
	return tok.type != TOKEN_TYPE_NULL && tok.type == TOKEN_TYPE_OP && (c[0] == 0 || strcmp(tok.value, c) == 0);
}

void skip_separator(PARSER* p) {
	if (lexer_peek(p->input).type == TOKEN_TYPE_SEPARATOR) lexer_next(p->input);
	else lexer_error(p->input, "SEPARATOR expected");
}

void skip_all_separators(PARSER* p) {
	while (!lexer_eof(p->input) && lexer_peek(p->input).type == TOKEN_TYPE_SEPARATOR) {
		skip_separator(p);
	}
}

void skip_keyword(PARSER* p, const char* kw) {
	if (next_is_keyword(p, kw)) lexer_next(p->input);
	else lexer_error(p->input, "Keyword '%s' expected", kw);
}

void skip_punc(PARSER* p, char c) {
	if (next_is_punc(p, c)) lexer_next(p->input);
	else lexer_error(p->input, "TOKEN '%c' expected", c);
}

void skip_op(PARSER* p, const char* op) {
	if (next_is_op(p, op)) lexer_next(p->input);
	else lexer_error(p->input, "TOKEN '%s' expected", op);
}

int get_op_precedence(const char* op) {
	for (int i = 0; i < NUM_OPS; i++) {
		if (strcmp(OP_MAP_K[i], op) == 0) return OP_MAP_V[i];
	}
	return -1;
}

AST parse_ast(PARSER* p);
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

EXPRESSION parse_compound(PARSER* p) {
	skip_punc(p, '{');
	AST* ast = malloc(sizeof(AST));
	*ast = parse_ast(p);
	skip_punc(p, '}');
	return (EXPRESSION) { EXPR_TYPE_COMPOUND, .compound = { ast } };
}

EXPRESSION parse_return(PARSER* p) {
	skip_keyword(p, KEYWORD_RETURN);
	EXPRESSION* value = malloc(sizeof(EXPRESSION));
	*value = parse_expr(p);
	return (EXPRESSION) { EXPR_TYPE_RETURN, .ret_statement = { value } };
}

EXPRESSION parse_if(PARSER* p) {
	skip_keyword(p, KEYWORD_IF);
	EXPRESSION* condition = malloc(sizeof(EXPRESSION));
	*condition = parse_expr(p);
	EXPRESSION* then_block = malloc(sizeof(EXPRESSION));
	*then_block = parse_expr(p);
	EXPRESSION* else_block = NULL;
	if (next_is_keyword(p, KEYWORD_ELSE)) {
		skip_keyword(p, KEYWORD_ELSE);
		else_block = malloc(sizeof(EXPRESSION));
		*else_block = parse_expr(p);
	}
	return (EXPRESSION) { EXPR_TYPE_IF_STATEMENT, .if_statement = { condition, then_block, else_block } };
}

EXPRESSION parse_loop(PARSER* p) {
	skip_keyword(p, KEYWORD_LOOP);
	EXPRESSION* body = malloc(sizeof(EXPRESSION));
	*body = parse_expr(p);
	return (EXPRESSION) { EXPR_TYPE_LOOP, .loop = { NULL, body } };
}

EXPRESSION parse_while(PARSER* p) {
	skip_keyword(p, KEYWORD_WHILE);
	EXPRESSION* condition = malloc(sizeof(EXPRESSION));
	*condition = parse_expr(p);
	EXPRESSION* body = malloc(sizeof(EXPRESSION));
	*body = parse_expr(p);
	return (EXPRESSION) { EXPR_TYPE_LOOP, .loop = { condition, body } };
}

EXPRESSION parse_break(PARSER* p) {
	skip_keyword(p, KEYWORD_BREAK);
	uint8_t idx = 0;
	if (next_is_punc(p, '(')) {
		skip_punc(p, '(');
		TOKEN index_token = lexer_next(p->input);
		if (index_token.type != TOKEN_TYPE_INT) lexer_error(p->input, "Break index must be of type integer");
		else idx = (uint8_t)strtol(index_token.value, NULL, 0);
		skip_punc(p, ')');
	}
	return (EXPRESSION) { EXPR_TYPE_BREAK, .break_statement = { idx } };
}

EXPRESSION parse_continue(PARSER* p) {
	skip_keyword(p, KEYWORD_CONTINUE);
	uint8_t idx = 0;
	if (next_is_punc(p, '(')) {
		skip_punc(p, '(');
		TOKEN index_token = lexer_next(p->input);
		if (index_token.type != TOKEN_TYPE_INT) lexer_error(p->input, "Continue index must be of type integer");
		else idx = (uint8_t)strtol(index_token.value, NULL, 0);
		skip_punc(p, ')');
	}
	return (EXPRESSION) { EXPR_TYPE_CONTINUE, .continue_statement = { idx } };
}

EXPRESSION parse_call(PARSER* p, EXPRESSION func) {
	EXPR_VEC arglist = delimited_expr(p, '(', ')', ',', parse_expr);
	EXPRESSION* funcptr = malloc(sizeof(EXPRESSION));
	*funcptr = func;
	return (EXPRESSION) { EXPR_TYPE_FUNC_CALL, .func_call = (FUNC_CALL){ funcptr, arglist.buffer, (uint8_t)arglist.size } };
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
	skip_keyword(p, KEYWORD_FUNC_DECL);
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

	if (next_is_keyword(p, KEYWORD_TRUE) || next_is_keyword(p, KEYWORD_FALSE)) return (EXPRESSION) { EXPR_TYPE_BOOL_LITERAL, .bool_literal = { strcmp(lexer_next(p->input).value, KEYWORD_FALSE) } };
	if (next_is_op(p, "*")) {
		char* op = lexer_next(p->input).value;
		EXPRESSION* expr = malloc(sizeof(EXPRESSION));
		*expr = maybe_unary(p, parse_atom(p));
		return (EXPRESSION) { EXPR_TYPE_UNARY_OP, .unary_op = (UNARY_OP){ op, false, expr } };
	}
	if (next_is_punc(p, '{')) return parse_compound(p);
	if (next_is_keyword(p, KEYWORD_RETURN)) return parse_return(p);
	if (next_is_keyword(p, KEYWORD_IF)) return parse_if(p);
	if (next_is_keyword(p, KEYWORD_LOOP)) return parse_loop(p);
	if (next_is_keyword(p, KEYWORD_WHILE)) return parse_while(p);
	if (next_is_keyword(p, KEYWORD_BREAK)) return parse_break(p);
	if (next_is_keyword(p, KEYWORD_CONTINUE)) return parse_continue(p);
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
	skip_all_separators(p);
	while (!lexer_eof(p->input) && !next_is_punc(p, '}')) {
		evec_push(&expressions, parse_expr(p));
		if (!lexer_eof(p->input)) skip_separator(p);
	}
	return (AST) { expressions.buffer, expressions.size };
}

void delete_expr(EXPRESSION* expr) {

}

void delete_ast(AST* ast) {
	for (int i = 0; i < ast->num_expressions; i++) {
		delete_expr(&ast->expressions[i]);
	}
}