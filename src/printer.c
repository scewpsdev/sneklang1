#include "printer.h"

AST_PRINTER printer_new() {
	AST_PRINTER p;
	p.indentation = 0;
	return p;
}

void printer_delete(AST_PRINTER* printer) {
}

void indent(AST_PRINTER* p) {
	for (int i = 0; i < p->indentation; i++) putchar(' ');
}

void print_int_literal(AST_PRINTER* p, INT* i) {
	printf("%d", (int)i->value);
}

void print_char_literal(AST_PRINTER* p, CHAR* i) {
	putchar('\'');
	putchar(i->value);
	putchar('\'');
}

void print_bool_literal(AST_PRINTER* p, BOOL* b) {
	printf(b->value ? "true" : "false");
}

void print_float_literal(AST_PRINTER* p, FLOAT* f) {
	printf("%f", f->value);
}

void print_string_literal(AST_PRINTER* p, STRING* str) {
	putchar('"');
	printf(str->value);
	putchar('"');
}

void print_identifier(AST_PRINTER* p, IDENTIFIER* identifier) {
	printf(identifier->name);
}

void print_expr(AST_PRINTER* p, EXPRESSION* expr) {
	switch (expr->type) {
	case EXPR_TYPE_INT: print_int_literal(p, &expr->int_literal); break;
	case EXPR_TYPE_CHAR: print_char_literal(p, &expr->char_literal); break;
	case EXPR_TYPE_BOOL: print_bool_literal(p, &expr->bool_literal); break;
	case EXPR_TYPE_FLOAT: print_float_literal(p, &expr->float_literal); break;
	case EXPR_TYPE_STRING: print_string_literal(p, &expr->string_literal); break;
	case EXPR_TYPE_IDENTIFIER: print_identifier(p, &expr->identifier); break;
	default: break;
	}
}

void print_expr_statement(AST_PRINTER* p, EXPR_STATEMENT* expr) {
	print_expr(p, &expr->expr);
	putchar(';');
}

void print_statement(AST_PRINTER* p, STATEMENT* statement) {
	switch (statement->type) {
	case STATEMENT_TYPE_EXPR: print_expr_statement(p, &statement->expr); break;
	default: break;
	}
}

void print_ast(AST_PRINTER* p, AST* ast) {
	for (int i = 0; i < ast->num_statements; i++) {
		indent(p);
		print_statement(p, &ast->statements[i]);
		putchar('\n');
	}
}