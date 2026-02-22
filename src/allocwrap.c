#include <stdio.h>
#include <stdlib.h>

#include "allocwrap.h"

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
