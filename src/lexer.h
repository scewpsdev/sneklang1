#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "input.h"
#include "utils.h"

#define TOKEN_NULL (TOKEN) {0}

enum TOKEN_TYPE {
	TOKEN_TYPE_NULL,
	TOKEN_TYPE_SEPARATOR,
	TOKEN_TYPE_INT,
	TOKEN_TYPE_CHAR,
	TOKEN_TYPE_FLOAT,
	TOKEN_TYPE_STRING,
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_KEYWORD,
	TOKEN_TYPE_PUNC,
	TOKEN_TYPE_OP
};

typedef struct TOKEN_t {
	uint8_t type;
	char* value;
} TOKEN;

typedef struct LEXER_t {
	INPUTSTREAM* input;
	TOKEN current;
	long line, col;

	STRING_VEC token_data;
} LEXER;

LEXER lexer_new(INPUTSTREAM* input);
void lexer_delete(LEXER* lexer);

TOKEN lexer_next(LEXER* l);
TOKEN lexer_peek(LEXER* l);
bool lexer_eof(LEXER* l);

void lexer_error(LEXER* l, const char* msg, ...);
