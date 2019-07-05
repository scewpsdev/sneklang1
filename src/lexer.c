#include "lexer.h"

#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"

const char* KEYWORDS[] = {
	"world",
	""
};

LEXER lexer_new(INPUTSTREAM* input) {
	LEXER l;
	l.input = input;
	l.current = TOKEN_NULL;
	return l;
}

void lexer_delete(LEXER l) {
}

bool is_whitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool is_identifier(char c) {
	return isdigit(c) || isalpha(c);
}

bool is_keyword(char* str) {
	for (int i = 0; KEYWORDS[i][0] != 0; i++) {
		if (strcmp(str, KEYWORDS[i]) == 0) return true;
	}
	return false;
}

bool is_int(char c, char c2) {
	return isdigit(c) || c == '-' && isdigit(c2);
}

bool is_punc(char c) {
	return c == '.' || c == ',' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']';
}

bool is_op(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == 
}

bool not_newline(char c) {
	return c != '\n';
}

bool not_block_comment_end(char c, char c2) {
	return c != '*' || c2 != '/';
}

char* read_while(LEXER* l, bool(*parser)(char)) {
	STRING result = string_new(10);
	while (!input_eof(l->input) && parser(input_peek(l->input))) {
		string_append(&result, input_next(l->input));
	}
	return result.buffer;
}

char* read_while2(LEXER* l, bool(*parser)(char, char)) {
	STRING result = string_new(10);
	while(!input_eof(l->input) && parser(input_peek(l->input), input_peek_n(l->input, 1))) {
		string_append(&result, input_next(l->input));
	}
	return result.buffer;
}

char* read_escaped(char end) {
	STRING result = string_new(100);
	bool esc = false;
	while(!input_eof(l->input)) {
		char c = input_next(l->input);
		if (esc) {
			// TODO
			string_append(&result, c);
			esc = false;
		} else if (c == '\\') esc = true;
		else if (c == end) break;
		else string_append(&result, c);
	}
	return result.buffer;
}

TOKEN read_identifier(LEXER* l) {
	char* value = read_while(l, is_identifier);
	return (TOKEN) { is_keyword(value) ? TOKEN_TYPE_KEYWORD : TOKEN_TYPE_IDENTIFIER, value };
}

TOKEN read_string(LEXER* l) {
	input_next(l);
	return (TOKEN) { TOKEN_TYPE_STRING, read_escaped('"') };
}

TOKEN read_char(LEXER* l) {
	return (TOKEN) { TOKEN_TYPE_CHAR, read_escaped('\'') };
}

void skip_line_comment(LEXER* l) {
	input_next(l->input);
	input_next(l->input);
	free(read_while2(l, not_newline));
	input_next(l->input);
}

void skip_block_comment(LEXER* l) {
	input_next(l->input);
	input_next(l->input);
	free(read_while(l, not_block_comment_end));
	input_next(l->input);
	input_next(l->input);
}

TOKEN read_next(LEXER* l) {
	free(read_while(l, is_whitespace));
	if (input_eof(l->input)) return TOKEN_NULL;

	char next = input_peek(l->input);
	char next2 = input_peek_n(l->input, 2);

	if (next == '/' && next2 == '/') {
		skip_line_comment(l);
		return read_next(l);
	} else if (next == '/' && next2 == '*') {
		skip_block_comment(l);
		return read_next();
	}

	if (c == '"') return read_string();
	if (c == '\'') return read_char();
	if (is_identifier(next)) return read_identifier(l);
	if (is_int(next, next2)) return read_int(l);
	if (is_punc(next)) {
		char* value = malloc(2);
		value[0] = input_next(l->input);
		value[1] = 0;
		return (TOKEN) { TOKEN_TYPE_PUNC, value };
	}
	if (is_op(next)) return (TOKEN) { TOKEN_TYPE_OP, read_while(is_op) };

	lexer_error(l, "Can't handle character '%c'", next);
	return TOKEN_NULL;
}

TOKEN lexer_next(LEXER* l) {
	TOKEN tok = l->current;
	l->current = TOKEN_NULL;
	return tok.type != TOKEN_TYPE_NULL ? tok : read_next(l);
}

TOKEN lexer_peek(LEXER* l) {
	return l->current.type != TOKEN_TYPE_NULL ? l->current : (l->current = read_next(l));
}

bool lexer_eof(LEXER* l) {
	return lexer_peek(l).type == TOKEN_TYPE_NULL;
}

void lexer_error(LEXER* l, const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	input_error(l->input, msg, 0, 0, args);
	va_end(args);
}
