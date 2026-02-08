#pragma once
#include <stdint.h>

static inline uint32_t bytesToSectorsRoundUp(uint32_t bytes, uint16_t bytesPerSector) {
	return (bytes + bytesPerSector - 1) / bytesPerSector;
}

/** converts filename in the format of fat12 to a regular file name, handles errors internally and
 * terminates the program on fail
 * @param a file name in the format of fat12(11 chars long [8 for name][3 for extention])
 * @return an allocated string containg the name
 */
char* fatFileNameToStr(char* filenameFatFormat);

void* xmalloc(uint64_t size);
void* xrealloc(void* ptr, uint64_t size);
