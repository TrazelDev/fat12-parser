#include <stdio.h>
#include <stdlib.h>

#include "fat12.h"

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Invalid usage:\n");
		printf("Usage: FAT12Parser <loop_device_file>\n");
		exit(-1);
	}
	char* loopDevicePath = argv[1];

	FAT12Header* fat12Header = malloc(sizeof(FAT12Header));
	if (!loadFat12Header(fat12Header, loopDevicePath)) {
		free(fat12Header);
		exit(-1);
	}
	printFat12Header(fat12Header);

	FAT12Info* fat12Info = malloc(sizeof(FAT12Info));
	if (!loadFat12Info(fat12Header, fat12Info)) {
		free(fat12Info);
		free(fat12Header);
		exit(-1);
	}
	printFat12Info(fat12Info);
}
