#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "lexer.h"
#include "ast.h"

typedef struct PARSER_t {
	LEXER* input;
} PARSER;

PARSER parser_new(LEXER* lexer);
void parser_delete(PARSER* parser);

AST parse_ast(PARSER* p);