#pragma once
#include <stdint.h>

void* xmalloc(uint64_t size);
void* xrealloc(void* ptr, uint64_t size);
