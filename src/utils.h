#pragma once

typedef struct DYNAMIC_STRING_t {
	char* buffer;
	long length;
	long bufsize;
} DYNAMIC_STRING;