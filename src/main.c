#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fat12_api.h"

void smallTest(const char* loopDevicePath) {
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

void catFile(const char* loopDevicePath, const char* filePath) {
	char* fileContent;
	initFat12Api(loopDevicePath);
	getFileContentByPath((uint8_t**)&fileContent, filePath);
	printf("%s", fileContent);
	free(fileContent);
}

void lsPath(const char* loopDevicePath, const char* filePath) {
	initFat12Api(loopDevicePath);

	char** names;
	uint32_t nameCount = getFileNamesByPath(&names, filePath);
	for (uint32_t i = 0; i < nameCount; i++) {
		printf("%s\n", names[i]);
		free(names[i]);
	}
	free((void*)names);
}
void printHelpMenu() {
	printf("Invalid usage:\n");
	printf("Usage: FAT12Parser <loop_device_file> <command>\n\n");
	printf("Supported commands:\n");
	printf("1. ls <dir_path>\n");
	printf("2. cat <file_path>\n");
}

int main(int argc, char** argv) {
	if (argc != 4) {
		printHelpMenu();
		exit(-1);
	}

	const char LS_COMMAND[] = "ls";
	const char CAT_COMMAND[] = "cat";
	char* loopDevicePath = argv[1];
	char* command = argv[2];
	char* path = argv[3];

	if (strlen(command) == strlen(LS_COMMAND) && strcmp(command, LS_COMMAND) == 0) {
		lsPath(loopDevicePath, path);
	} else if (strlen(command) == strlen(CAT_COMMAND) && strcmp(command, CAT_COMMAND) == 0) {
		catFile(loopDevicePath, path);
	} else {
		printHelpMenu();
		exit(-1);
	}
}
