#include "lexer.h"

LEXER lexer_new(INPUTSTREAM* input) {
	LEXER l;
	l.input = input;
	return l;
}

void lexer_delete(LEXER l) {
}

TOKEN read_next(LEXER* l) {
	TOKEN t;
	t.type = TOKEN_NULL;
	return t;
}

TOKEN lexer_next(LEXER* l) {
	TOKEN t;
	t.type = TOKEN_NULL;
	return t;
}

TOKEN lexer_peek(LEXER* l) {
	TOKEN t;
	t.type = TOKEN_NULL;
	return t;
}

bool lexer_eof(LEXER* l) {
	return true;
}

void lexer_error(LEXER* l, const char* msg) {
	input_error(l->input, msg, 0, 0);
}