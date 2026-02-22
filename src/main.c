#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocwrap.h"
#include "fat12.h"

void test(FAT12Info* fat12Info, const char* loopDevicePath) {
	// printFileAllocationTable(fat12Info, loopDevicePath);
	FAT12DirectoryEntry* dirEntries;
	FAT12DirectoryEntry entry;
	char* data;
	uint32_t entriesCount;
	entriesCount = getRootDirectoryEntries(&dirEntries, fat12Info, loopDevicePath);
	memcpy(&entry, dirEntries, sizeof(FAT12DirectoryEntry));

	entriesCount = getDirectoryEntries(&dirEntries, &entry, fat12Info, loopDevicePath);
	entriesCount = filterValidDirectoryEntries(&dirEntries, entriesCount);
	printFat12DirectoryEntry(dirEntries);
	printFat12DirectoryEntry(dirEntries + 1);
	printFat12DirectoryEntry(dirEntries + 2);

	getFileContent((uint8_t**)&data, dirEntries + 2, fat12Info, loopDevicePath);
	printf("%s", data);
}

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
		printf("%d %s\n", i, fileNames[i]);
	}

	// printFileAllocationTable(fat12Info, loopDevicePath);
	test(fat12Info, loopDevicePath);
}
