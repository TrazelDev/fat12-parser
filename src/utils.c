#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

char* fatFileNameToStr(char* filenameFatFormat) {
	const uint32_t FILENAME_LENGTH = 8;
	const uint32_t EXTENSION_LENGTH = 3;

	// Copying:
	char* name = xmalloc(FILENAME_LENGTH + EXTENSION_LENGTH + 2);
	memcpy(name, filenameFatFormat, FILENAME_LENGTH);
	if (filenameFatFormat[FILENAME_LENGTH] != ' ') {
		name[FILENAME_LENGTH] = '.';
	}
	memcpy(name + FILENAME_LENGTH + 1, filenameFatFormat + FILENAME_LENGTH, EXTENSION_LENGTH);

	// Removing spaces:
	uint32_t j = 0;
	for (uint32_t i = 0; i < FILENAME_LENGTH + EXTENSION_LENGTH + 1; i++) {
		if (name[i] != ' ') {
			name[j] = tolower(name[i]);
			j++;
		}
	}
	name[j] = '\0';
	return name;
}

void* xmalloc(uint64_t size) {
	void* ret = malloc(size);
	if (!ret) {
		perror("");
		exit(-1);
	}
	return ret;
}

void* xrealloc(void* ptr, uint64_t size) {
	void* ret = realloc(ptr, size);
	if (!ret) {
		perror("");
		exit(-1);
	}
	return ret;
}
