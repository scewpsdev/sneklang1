#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "input.h"

#define TOKEN_NULL (TOKEN) {0}

#define TOKEN_TYPE_NULL 0x00

#define TOKEN_TYPE_IDENTIFIER 0x10
#define TOKEN_TYPE_KEYWORD 0x11
#define TOKEN_TYPE_STRING 0x12
#define TOKEN_TYPE_CHAR 0x13
#define TOKEN_TYPE_INT 0x14
#define TOKEN_TYPE_PUNC 0x15
#define TOKEN_TYPE_OP 0x16

typedef struct TOKEN_t {
	uint8_t type;
	char* value;
} TOKEN;

typedef struct LEXER_t {
	INPUTSTREAM* input;
	TOKEN current;
} LEXER;

LEXER lexer_new(INPUTSTREAM* input);
void lexer_delete(LEXER lexer);

TOKEN lexer_next(LEXER* l);
TOKEN lexer_peek(LEXER* l);
bool lexer_eof(LEXER* l);

void lexer_error(LEXER* l, const char* msg, ...);
