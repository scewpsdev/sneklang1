#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct TOKEN_t {
	uint8_t type;
	char* value;
} TOKEN;

typedef struct LEXER_t {
	INPUTSTREAM* input;
} LEXER;

LEXER lexer_new(INPUTSTREAM* input);
void lexer_delete(LEXER lexer);

TOKEN read_next();
TOKEN next();
TOKEN peek();
bool eof();

void error(LEXER* l, const char* msg);
