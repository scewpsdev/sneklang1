#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "file.h"
#include "input.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		char* filepath = argv[i];
		char* source = load_file(filepath);
		INPUTSTREAM input = input_new(source);
		LEXER lexer = lexer_new(&input);
		PARSER parser = parser_new(&lexer);
		
		parse_toplevel(&parser);

		delete_parser(parser);
		delete_lexer(lexer);
		delete_input(input);
		free(source);
	}
	return 0;
}
