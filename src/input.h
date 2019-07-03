#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct INPUTSTREAM_t {
	char* buffer;
	char* ptr;
	int line, col;
} INPUTSTREAM;

INPUTSTREAM input_new(char* buffer);
void input_delete(INPUTSTREAM i);

char input_next(INPUTSTREAM* i);
char input_peek(INPUTSTREAM* i);
bool eof();

void error(INPUTSTREAM* i, const char* msg, int line, int col);
