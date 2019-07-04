#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "input.h"

#define TOKEN_NULL 0x00

typedef struct TOKEN_t {
	uint8_t type;
	char* value;
} TOKEN;

typedef struct LEXER_t {
	INPUTSTREAM* input;
} LEXER;

LEXER lexer_new(INPUTSTREAM* input);
void lexer_delete(LEXER lexer);

TOKEN lexer_next(LEXER* l);
TOKEN lexer_peek(LEXER* l);
bool lexer_eof(LEXER* l);

void lexer_error(LEXER* l, const char* msg);
