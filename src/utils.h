#pragma once

typedef struct STRING_t {
	char* buffer;
	long length;
	long bufsize;
} STRING;

STRING string_new(int size);
void string_delete(STRING str);
void string_resize(STRING* str, int size);
void string_append(STRING* str, char c);