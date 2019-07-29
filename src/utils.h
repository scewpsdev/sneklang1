#pragma once

#include <llvm-c/Core.h>

#include "ast.h"

#define DECL_DYNAMIC_VECTOR(element, name, prefix) \
typedef struct name##_t {\
	element* buffer;\
	long size;\
	long capacity;\
} name;\
name prefix##_new(long size);\
void prefix##_delete(name* v);\
void prefix##_resize(name* v, long size);\
void prefix##_push(name* v, element e);

#define DEF_DYNAMIC_VECTOR(element, name, prefix) \
name prefix##_new(long size) {\
	return (name) { malloc(size * sizeof(element)), 0, size };\
}\
void prefix##_delete(name* v) {\
	free(v->buffer);\
}\
void prefix##_resize(name* v, long size) {\
	element* newbuf = realloc(v->buffer, size * sizeof(element));\
	v->buffer = newbuf;\
	v->size = min(v->size, size);\
	v->capacity = size;\
}\
void prefix##_push(name* v, element e) {\
	if (v->size == v->capacity) prefix##_resize(v, v->capacity * 2);\
	v->buffer[v->size++] = e;\
}

#define DEF_DYNAMIC_VECTOR_TERMINATED(element, name, prefix, terminator) \
name prefix##_new(long size) {\
	name n = (name) { malloc((size + 1) * sizeof(element)), 0, size };\
	n.buffer[0] = terminator;\
	return n;\
}\
void prefix##_delete(name* v) {\
	free(v->buffer);\
}\
void prefix##_resize(name* v, long size) {\
	element* newbuf = realloc(v->buffer, (size + 1) * sizeof(element));\
	v->buffer = newbuf;\
	v->size = min(v->size, size);\
	v->capacity = size;\
	v->buffer[v->size] = terminator;\
}\
void prefix##_push(name* v, element e) {\
	if (v->size == v->capacity) prefix##_resize(v, v->capacity * 2);\
	v->buffer[v->size++] = e;\
	v->buffer[v->size] = terminator;\
}

DECL_DYNAMIC_VECTOR(char, DYNAMIC_STRING, string)
DECL_DYNAMIC_VECTOR(char*, STRING_VEC, strvec)
DECL_DYNAMIC_VECTOR(VAR_DECL, VAR_DECL_VEC, vdvec)
DECL_DYNAMIC_VECTOR(EXPRESSION, EXPR_VEC, evec)
DECL_DYNAMIC_VECTOR(LLVMModuleRef, MODULE_VEC, mdvec)
DECL_DYNAMIC_VECTOR(AST, AST_VEC, astvec)
DECL_DYNAMIC_VECTOR(LLVMValueRef, VALUE_VEC, valvec)

void string_push_s(DYNAMIC_STRING* str, char* s);

char* copy_str(char* str);

/*
typedef struct DYNAMIC_STRING_t {
	char* buffer;
	long length;
	long bufsize;
} DYNAMIC_STRING;

DYNAMIC_STRING string_new(int size);
void string_delete(DYNAMIC_STRING str);
void string_resize(DYNAMIC_STRING* str, int size);
void string_append(DYNAMIC_STRING* str, char c);

typedef struct STATEMENT_VEC_t {
	STATEMENT* buffer;
	long length;
	long bufsize;
} STATEMENT_VEC;

STATEMENT_VEC statement_vec_new(int size);
void statement_vec_delete(STATEMENT_VEC str);
void statement_vec_resize(STATEMENT_VEC* str, int size);
void statement_vec_append(STATEMENT_VEC* str, char c);
*/