#include "parser.h"

PARSER parser_new(LEXER* lexer) {
	PARSER p;
	p.input = lexer;
	return p;
}

void parser_delete(PARSER parser) {
}

AST parse_toplevel(PARSER* p) {
	AST ast;
	ast.statements = NULL;
	return ast;
}