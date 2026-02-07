#pragma once
#include <stdint.h>

static inline uint32_t bytesToSectorsRoundUp(uint32_t bytes, uint16_t bytesPerSector) {
	return (bytes + bytesPerSector - 1) / bytesPerSector;
}

void* xmalloc(uint64_t size);
void* xrealloc(void* ptr, uint64_t size);
