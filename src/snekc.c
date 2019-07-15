#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "file.h"
#include "input.h"
#include "lexer.h"
#include "parser.h"
#include "printer.h"
#include "gen.h"

void test_lexer(LEXER* l) {
	printf("### TOKENS ###\n");
	while (!lexer_eof(l)) {
		TOKEN token = lexer_next(l);
		printf("%d: %s\n", token.type, token.value);
	}
	putchar('\n');
}

void test_parser(AST* ast) {
	printf("### AST ###\n");
	AST_PRINTER printer = printer_new();
	print_ast(&printer, ast);
	printer_delete(&printer);
	putchar('\n');
}

int main(int argc, char** argv) {
	CODEGEN gen = gen_new();
	for (int i = 1; i < argc; i++) {
		char* filepath = argv[i];
		char* source = load_file(filepath);
		INPUTSTREAM input = input_new(source);

		LEXER lexer = lexer_new(&input);
		test_lexer(&lexer);
		input_reset(&input);

		PARSER parser = parser_new(&lexer);
		AST ast = parse_ast(&parser);
		test_parser(&ast);

		printf("### LLVM ###\n");
		gen_create_module(&gen, &ast);

		delete_ast(&ast);
		parser_delete(&parser);
		lexer_delete(&lexer);
		input_delete(&input);
	}
	gen_link(&gen);
	gen_delete(&gen);

	return 0;
}
