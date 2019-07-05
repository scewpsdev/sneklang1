#pragma once

typedef struct DYNAMIC_STRING_t {
	char* buffer;
	long length;
	long bufsize;
} DYNAMIC_STRING;

DYNAMIC_STRING string_new(int size);
void string_delete(DYNAMIC_STRING str);
void string_resize(DYNAMIC_STRING* str, int size);
void string_append(DYNAMIC_STRING* str, char c);