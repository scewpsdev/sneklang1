#pragma once

#include <stdint.h>

#include "ast.h"

typedef struct AST_PRINTER_t {
	uint8_t indentation;
} AST_PRINTER;

AST_PRINTER printer_new();
void printer_delete(AST_PRINTER* printer);

void print_ast(AST_PRINTER* p, AST* ast);