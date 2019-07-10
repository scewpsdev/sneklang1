#include "printer.h"

AST_PRINTER printer_new() {
	AST_PRINTER p;
	p.indentation = 0;
	return p;
}

void printer_delete(AST_PRINTER* printer) {
}

void print_expr(AST_PRINTER* p, EXPRESSION* expr);

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

void print_func_call(AST_PRINTER* p, FUNC_CALL* func_call) {
	print_expr(p, func_call->callee);
	putchar('(');
	for (int i = 0; i < func_call->num_args; i++) {
		print_expr(p, &func_call->args[i]);
		if (i < func_call->num_args - 1) printf(", ");
	}
	putchar(')');
}

void print_type(TYPE t) {
	if (t.cpy) putchar('*');
	printf(t.name);
}

void print_arg_list(AST_PRINTER* p, VAR_DECL* args, int num_args) {
	for (int i = 0; i < num_args; i++) {
		print_type(args[i].type);
		if (args[i].name) printf(" %s", args[i].name);
		if (i < num_args - 1) printf(", ");
	}
}

void print_func_decl(AST_PRINTER* p, FUNC_DECL* func_decl) {
	printf("decl %s(", func_decl->funcname);
	print_arg_list(p, func_decl->args, func_decl->num_args);
	printf(");");
}

void print_expr(AST_PRINTER* p, EXPRESSION* expr) {
	switch (expr->type) {
	case EXPR_TYPE_INT_LITERAL: print_int_literal(p, &expr->int_literal); break;
	case EXPR_TYPE_CHAR_LITERAL: print_char_literal(p, &expr->char_literal); break;
	case EXPR_TYPE_BOOL_LITERAL: print_bool_literal(p, &expr->bool_literal); break;
	case EXPR_TYPE_FLOAT_LITERAL: print_float_literal(p, &expr->float_literal); break;
	case EXPR_TYPE_STRING_LITERAL: print_string_literal(p, &expr->string_literal); break;
	case EXPR_TYPE_IDENTIFIER: print_identifier(p, &expr->identifier); break;
	case EXPR_TYPE_FUNC_CALL: print_func_call(p, &expr->func_call); break;
	case EXPR_TYPE_FUNC_DECL: print_func_decl(p, &expr->func_decl); break;
	default: break;
	}
}

void print_ast(AST_PRINTER* p, AST* ast) {
	for (int i = 0; i < ast->num_expressions; i++) {
		indent(p);
		print_expr(p, &ast->expressions[i]);
		putchar('\n');
	}
}