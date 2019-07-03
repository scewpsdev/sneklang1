#include <stdlib.h>
#include <stdio.h>

char* load_file(const char* filepath) {
	FILE* file = fopen(filepath, "rb");
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* buffer = malloc(length + 1);
	fread(buffer, 0, length, file);
	buffer[length] = 0;
	fclose(file);
	return buffer;
}
