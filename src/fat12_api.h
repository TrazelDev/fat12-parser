#pragma once
#include <stdint.h>

void initFat12Api(const char* loopDevicePath);
uint32_t getFileContentByPath(uint8_t** fileContent, const char* path);
uint32_t listFilesInDirectoryByPath(char** filesList, const char* path);
