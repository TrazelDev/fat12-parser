#include <stdio.h>
#include <stdlib.h>

#include "fat12.h"
#include "utils.h"

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Invalid usage:\n");
		printf("Usage: FAT12Parser <loop_device_file>\n");
		exit(-1);
	}
	char* loopDevicePath = argv[1];

	FAT12Header* fat12Header = xmalloc(sizeof(FAT12Header));
	if (!loadFat12Header(fat12Header, loopDevicePath)) {
		free(fat12Header);
		exit(-1);
	}
	printFat12Header(fat12Header);

	FAT12Info* fat12Info = xmalloc(sizeof(FAT12Info));
	if (!loadFat12Info(fat12Info, fat12Header)) {
		free(fat12Info);
		free(fat12Header);
		exit(-1);
	}
	printFat12Info(fat12Info);

	char** fileNames = NULL;
	uint32_t namesCount = getRootFileNames(&fileNames, fat12Info, loopDevicePath);
	for (uint32_t i = 0; i < namesCount; i++) {
		printf("%s\n", fileNames[i]);
	}
}
