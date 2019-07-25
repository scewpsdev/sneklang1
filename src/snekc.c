#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

char* get_name_from_path(char* path) {
	char* c = strrchr(path, '/');
	char* filename = c ? c + 1 : path;
	char* module_name = malloc(strlen(filename) + 1);
	int last_fullstop = -1;
	for (int i = 0; i < strlen(filename); i++) if (filename[i] == '.') { last_fullstop = i; break; }
	int len = last_fullstop >= 0 ? last_fullstop : strlen(filename);
	memcpy(module_name, filename, len);
	module_name[len] = 0;
	return module_name;
}

int main(int argc, char** argv) {
	CODEGEN gen = gen_new();
	for (int i = 1; i < argc; i++) {
		char* filepath = argv[i];
		char* name = get_name_from_path(filepath);
		char* source = load_file(filepath);
		INPUTSTREAM input = input_new(source);

		LEXER lexer = lexer_new(&input);
		test_lexer(&lexer);
		input_reset(&input);

		PARSER parser = parser_new(&lexer);
		AST ast = parse_ast(&parser);
		test_parser(&ast);

		printf("### LLVM ###\n");
		gen_create_module(&gen, &ast, name);

		delete_ast(&ast);
		parser_delete(&parser);
		lexer_delete(&lexer);
		input_delete(&input);
	}
	gen_link(&gen);
	gen_delete(&gen);

	return 0;
}
