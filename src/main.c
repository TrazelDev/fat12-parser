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
	if (!loadFat12Info(fat12Header, fat12Info)) {
		free(fat12Info);
		free(fat12Header);
		exit(-1);
	}
	printFat12Info(fat12Info);

	// TODO: refactor the function: getEntriesFileNames and also make it so the name will be
	// displated correctly
	char** ls = getRootFileNames(fat12Info, loopDevicePath);
	for (int i = 0; ls[i] != NULL; i++) {
		printf("%s\n", ls[i]);
	}
}
