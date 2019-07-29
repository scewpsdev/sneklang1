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
		strvec_push(&gen.module_name_vec, name);
		astvec_push(&gen.module_ast_vec, ast);
		test_parser(&ast);

		//parser_delete(&parser);
		//lexer_delete(&lexer);
		//input_delete(&input);
	}

	printf("### LLVM ###\n");
	for (int i = 0; i < gen.module_ast_vec.size; i++) {
		gen_create_module(&gen, &gen.module_ast_vec.buffer[i], gen.module_name_vec.buffer[i]);
	}
	gen_link(&gen);

	for (int i = 0; i < gen.module_ast_vec.size; i++) delete_ast(&gen.module_ast_vec.buffer[i]);
	gen_delete(&gen);

	return 0;
}
