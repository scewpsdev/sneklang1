#include "utils.h"

#include <stdlib.h>
#include <memory.h>
#include <string.h>

DEF_DYNAMIC_VECTOR_TERMINATED(char, DYNAMIC_STRING, string, 0)
DEF_DYNAMIC_VECTOR(char*, STRING_VEC, strvec)
DEF_DYNAMIC_VECTOR(VAR_DECL, VAR_DECL_VEC, vdvec)
DEF_DYNAMIC_VECTOR(EXPRESSION, EXPR_VEC, evec)
DEF_DYNAMIC_VECTOR(LLVMModuleRef, MODULE_VEC, mdvec)
DEF_DYNAMIC_VECTOR(LLVMValueRef, VALUE_VEC, valvec)

void string_push_s(DYNAMIC_STRING* str, char* s) {
	int len = strlen(s);
	for (int i = 0; i < len; i++) string_push(str, s[i]);
}

/*
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
		string_resize(str, (str->bufsize - 1) * 2 + 1);
	}
	str->buffer[str->length++] = c;
	str->buffer[str->length] = 0;
}

STATEMENT_VEC statement_vec_new(int size) {

}

void statement_vec_delete(STATEMENT_VEC str) {

}

void statement_vec_resize(STATEMENT_VEC* str, int size) {

}

void statement_vec_append(STATEMENT_VEC* str, char c) {

}
*/