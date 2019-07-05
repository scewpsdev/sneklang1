#include "utils.h"

#include <stdlib.h>
#include <memory.h>

DYNAMIC_STRING string_new(int size) {
	DYNAMIC_STRING str;
	str.buffer = malloc(size + 1);
	str.length = 0;
	str.bufsize = size + 1;
	return str;
}

void string_delete(DYNAMIC_STRING str) {
	free(str.buffer);
}

void string_resize(DYNAMIC_STRING* str, int size) {
	char* newbuffer = malloc(size + 1);
	memcpy(newbuffer, str->buffer, min(str->bufsize, size + 1));
	free(str->buffer);
	str->buffer = newbuffer;
	str->length = min(str->length, size);
	str->bufsize = size + 1;
}

void string_append(DYNAMIC_STRING* str, char c) {
	if (str->length == str->bufsize - 1) {
		string_resize(str, str->bufsize + 10);
	}
	str->buffer[str->length++] = c;
	str->buffer[str->length] = 0;
}