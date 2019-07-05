#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

typedef struct INPUTSTREAM_t {
	char* buffer;
	char* ptr;
	int line, col;
} INPUTSTREAM;

INPUTSTREAM input_new(char* buffer);
void input_delete(INPUTSTREAM i);

char input_next(INPUTSTREAM* i);
char input_peek(INPUTSTREAM* i);
char input_peek_n(INPUTSTREAM* i, int offset);
bool input_eof(INPUTSTREAM* i);

void input_error(INPUTSTREAM* i, const char* msg, int line, int col, va_list args);
