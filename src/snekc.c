#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "file.h"
#include "input.h"
#include "lexer.h"
#include "parser.h"

void test_lexer(LEXER* l) {
	while (!lexer_eof(l)) {
		TOKEN token = lexer_next(l);
		printf("%d: %s\n", token.type, token.value);
	}
}

int main(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		char* filepath = argv[i];
		char* source = load_file(filepath);
		INPUTSTREAM input = input_new(source);
		LEXER lexer = lexer_new(&input);
		test_lexer(&lexer);
		PARSER parser = parser_new(&lexer);

		parse_toplevel(&parser);

		parser_delete(parser);
		lexer_delete(lexer);
		input_delete(input);
	}
	return 0;
}
