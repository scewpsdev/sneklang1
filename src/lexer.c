#include "lexer.h"

LEXER lexer_new(INPUTSTREAM* input) {
	LEXER l;
	l.input = input;
	return l;
}

void lexer_delete(LEXER l) {
}

bool is_whitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void read_while(LEXER* l, bool(*parser)(char)) {
	char* buffer = malloc(10);
	int length = 0;
	while (!input_eof(l->input) && parser(input_peek(l->input))) {
		buffer[length++] = input_next(l->input);
	}
}

TOKEN read_next(LEXER* l) {
	read_while(is_whitespace);
}

TOKEN lexer_next(LEXER* l) {
	TOKEN tok = l->current;
	l->current = TOKEN_NULL;
	return tok.type != TOKEN_TYPE_NULL ? tok : read_next();
}

TOKEN lexer_peek(LEXER* l) {
	return l->current.type != TOKEN_TYPE_NULL ? l->current : (l->current = read_next(l));
}

bool lexer_eof(LEXER* l) {
	return true;
}

void lexer_error(LEXER* l, const char* msg) {
	input_error(l->input, msg, 0, 0);
}