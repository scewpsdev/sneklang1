#include "lexer.h"

#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"
#include "keywords.h"

const char* KEYWORDS[] = {
	KEYWORD_TRUE,
	KEYWORD_FALSE,

	KEYWORD_FUNC_DECL,
	KEYWORD_RETURN,
	KEYWORD_IF,
	KEYWORD_ELSE,
	KEYWORD_LOOP,
	KEYWORD_WHILE,
	KEYWORD_BREAK,
	KEYWORD_CONTINUE,

	KEYWORD_IMPORT,

	""
};

LEXER lexer_new(INPUTSTREAM* input) {
	LEXER l;

	l.input = input;
	//l.current = TOKEN_NULL;
	//l.last = TOKEN_NULL;
	l.line = 1;
	l.col = 1;

	l.token_data = strvec_new(64);

	return l;
}

void lexer_delete(LEXER* l) {
	for (int i = 0; i < l->token_data.size; i++) {
		free(l->token_data.buffer[i]);
	}
	strvec_delete(&l->token_data);
}

bool is_whitespace(char c) {
	return c == ' ' || c == '\t' || c == '\r';
}

bool is_identifier(char c) {
	return isdigit(c) || isalpha(c) || c == '_';
}

bool is_keyword(char* str) {
	for (int i = 0; KEYWORDS[i][0] != 0; i++) {
		if (strcmp(str, KEYWORDS[i]) == 0) return true;
	}
	return false;
}

bool is_num(char c, char c2) {
	return isdigit(c) || c == '-' && isdigit(c2) || c == '.' && isdigit(c2);
}

bool is_punc(char c) {
	return c == '.' || c == ',' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']';
}

bool is_op(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' || c == '&' || c == '|' || c == '<' || c == '>' || c == '!';
}

bool is_digit(char c) {
	return isdigit(c) || c == '-' || c == '.';
}

bool not_newline(char c) {
	return c != '\n';
}

bool not_block_comment_end(char c, char c2) {
	return c != '*' || c2 != '/';
}

char* read_while(LEXER* l, bool(*parser)(char)) {
	DYNAMIC_STRING result = string_new(10);
	while (!input_eof(l->input) && parser(input_peek(l->input))) {
		string_push(&result, input_next(l->input));
	}
	strvec_push(&l->token_data, result.buffer);
	return result.buffer;
}

char* read_while2(LEXER* l, bool(*parser)(char, char)) {
	DYNAMIC_STRING result = string_new(10);
	while (!input_eof(l->input) && parser(input_peek(l->input), input_peek_n(l->input, 1))) {
		string_push(&result, input_next(l->input));
	}
	strvec_push(&l->token_data, result.buffer);
	return result.buffer;
}

char* read_escaped(LEXER* l, char end) {
	DYNAMIC_STRING result = string_new(100);
	bool esc = false;
	while (!input_eof(l->input)) {
		char c = input_next(l->input);
		if (esc) {
			esc = false;
			char escaped = 0;
			switch (c) {
			case 'n': escaped = '\n'; break;
			case 't': escaped = '\t'; break;
			case 'r': escaped = '\r'; break;
			case '0': escaped = '\0'; break;
			default: lexer_error(l, "Unknown escape character \\%c", c); continue;
			}
			string_push(&result, escaped);
		} else if (c == '\\') esc = true;
		else if (c == end) break;
		else string_push(&result, c);
	}
	strvec_push(&l->token_data, result.buffer);
	return result.buffer;
}

TOKEN read_number(LEXER* l) {
	char* str = read_while(l, is_digit);
	bool fpoint = false;
	for (int i = 0; str[i] != 0; i++) {
		if (str[i] == '.') {
			fpoint = true;
			break;
		}
	}
	return (TOKEN) { fpoint ? TOKEN_TYPE_FLOAT : TOKEN_TYPE_INT, str };
}

TOKEN read_char(LEXER* l) {
	input_next(l->input);
	return (TOKEN) { TOKEN_TYPE_CHAR, read_escaped(l, '\'') };
}

TOKEN read_string(LEXER* l) {
	input_next(l->input);
	return (TOKEN) { TOKEN_TYPE_STRING, read_escaped(l, '"') };
}

TOKEN read_identifier(LEXER* l) {
	char* value = read_while(l, is_identifier);
	return (TOKEN) { is_keyword(value) ? TOKEN_TYPE_KEYWORD : TOKEN_TYPE_IDENTIFIER, value };
}

void skip_line_comment(LEXER* l) {
	input_next(l->input);
	input_next(l->input);
	read_while(l, not_newline);
	input_next(l->input);
}

void skip_block_comment(LEXER* l) {
	input_next(l->input);
	input_next(l->input);
	read_while2(l, not_block_comment_end);
	input_next(l->input);
	input_next(l->input);
}

TOKEN read_next(LEXER* l) {
	read_while(l, is_whitespace);
	if (input_eof(l->input)) return TOKEN_NULL;

	char next = input_peek(l->input);
	char next2 = input_peek_n(l->input, 1);

	if (next == '/' && next2 == '/') {
		skip_line_comment(l);
		return read_next(l);
	} else if (next == '/' && next2 == '*') {
		skip_block_comment(l);
		return read_next(l);
	}

	if (next == '\n' || next == ';') {
		char* value = malloc(2);
		value[0] = input_next(l->input);
		value[1] = 0;
		strvec_push(&l->token_data, value);
		return (TOKEN) { TOKEN_TYPE_SEPARATOR, value };
	}
	if (next == '"') return read_string(l);
	if (next == '\'') return read_char(l);
	if (is_num(next, next2)) return read_number(l);
	if (is_identifier(next)) return read_identifier(l);
	if (is_punc(next)) {
		char* value = malloc(2);
		value[0] = input_next(l->input);
		value[1] = 0;
		strvec_push(&l->token_data, value);
		return (TOKEN) { TOKEN_TYPE_PUNC, value };
	}
	if (is_op(next)) return (TOKEN) { TOKEN_TYPE_OP, read_while(l, is_op) };

	lexer_error(l, "Can't handle character '%c'", next);
	return TOKEN_NULL;
}

TOKEN lexer_next(LEXER* l) {
	/*
	TOKEN tok = l->current;
	l->last = tok;
	l->current = TOKEN_NULL;
	l->line = l->input->line;
	l->col = l->input->col;
	*/
	//return tok.type != TOKEN_TYPE_NULL ? tok : read_next(l);
	l->line = l->input->line;
	l->col = l->input->col;
	return read_next(l);
}

TOKEN lexer_peek(LEXER* l) {
	char* ptr = l->input->ptr;
	TOKEN tok = read_next(l);
	input_rewind(l->input, ptr);
	return tok;
}

bool lexer_eof(LEXER* l) {
	return lexer_peek(l).type == TOKEN_TYPE_NULL;
}

void lexer_error(LEXER* l, const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	input_error(l->input, msg, l->line, l->col, args);
	va_end(args);
}
