#pragma once
#include <stdint.h>

void initFat12Api(const char* loopDevicePath);
uint32_t getFileContentByPath(uint8_t** fileContent, const char* filePath);
uint32_t getFileNamesByPath(char*** filesNames, const char* dirPath);
