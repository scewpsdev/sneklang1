#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "lexer.h"

typedef struct PARSER_t {
	LEXER* input;
} PARSER;

PARSER parser_new(LEXER* lexer);
void parser_delete(PARSER parser);

LEXER lexer_new(INPUTSTREAM* input);
void lexer_delete(LEXER lexer);

TOKEN read_next();
TOKEN next();
TOKEN peek();
bool eof();

void error(LEXER* l, const char* msg);
