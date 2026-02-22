#include <stdio.h>
#include <stdlib.h>

#include "fat12_api.h"

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Invalid usage:\n");
		printf("Usage: FAT12Parser <loop_device_file>\n");
		exit(-1);
	}
	char* loopDevicePath = argv[1];

	char* fileContent;
	initFat12Api(loopDevicePath);
	getFileContentByPath((uint8_t**)&fileContent, "/temp/files/file.txt");
	printf("%s", fileContent);
}
