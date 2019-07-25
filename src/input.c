#include "input.h"

INPUTSTREAM input_new(char* buffer) {
	INPUTSTREAM i;
	i.buffer = buffer;
	i.ptr = buffer;
	i.line = 1;
	i.col = 1;
	return i;
}

void input_delete(INPUTSTREAM* i) {
	free(i->buffer);
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

char input_peek_n(INPUTSTREAM* i, int offset) {
	return *(i->ptr + offset);
}

bool input_eof(INPUTSTREAM* i) {
	return *i->ptr == 0;
}

void input_rewind(INPUTSTREAM* i, char* ptr) {
	i->ptr = ptr;
}

void input_reset(INPUTSTREAM* i) {
	input_rewind(i, i->buffer);
	i->line = 1;
	i->col = 1;
}

void input_error(INPUTSTREAM* i, const char* msg, int line, int col, va_list args) {
	if (line == -1) line = i->line;
	if (col == -1) col = i->col;
	printf("(%d,%d) ", line, col);
	vprintf(msg, args);
	putchar('\n');
}
