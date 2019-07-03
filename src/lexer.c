#include "lexer.h"

LEXER lexer_new(INPUTSTREAM* input) {
	LEXER l;
	l.input = input;
	return l;
}

void lexer_delete(LEXER l) {
}

TOKEN read_next(LEXER* l) {
	
}
