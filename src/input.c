#include "input.h"

INPUTSTREAM input_new(char* buffer) {
	INPUTSTREAM i;
	i.buffer = buffer;
	i.ptr    = buffer;
	i.line   = 1;
	i.col    = 1;
	return i;
}

void input_delete(INPUTSTREAM i) {
	free(i.buffer);
}

char input_next(INPUTSTREAM* i) {
	char c = *i->ptr;
	i->ptr++;
	if (c == '\n') {
		i->line++;
		i->col = 0;
	} else i->col++;
	return c;
}

char input_peek(INPUTSTREAM* i) {
	return *i->ptr;
}

bool input_eof(INPUTSTREAM* i) {
	return *i->ptr == 0;
}

void input_error(INPUTSTREAM* i, const char* msg, int line, int col) {
	if (line == -1) line = i->line;
	if (col  == -1) col  = i->col;
	printf("(%d,%d) %s", line, col, msg);
}