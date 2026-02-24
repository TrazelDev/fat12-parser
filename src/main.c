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
	free(fileContent);

	char** names;
	uint32_t nameCount = getFileNamesByPath(&names, "/temp");
	for (uint32_t i = 0; i < nameCount; i++) {
		printf("%s\n", names[i]);
		free(names[i]);
	}

	free((void*)names);
}
