#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fat12.h"
#include "fat12_api.h"
#include "fat12_string.h"

static const char* fat12LoopDevicePath;
static FAT12Header fat12Header;
static FAT12Info fat12Info;

void initFat12Api(const char* loopDevicePath) {
	fat12LoopDevicePath = loopDevicePath;
	loadFat12Header(&fat12Header, loopDevicePath);
	loadFat12Info(&fat12Info, &fat12Header);
}

FAT12DirectoryEntry* getEntryByName(const char* fileName, FAT12DirectoryEntry* entries,
									uint32_t entriesCount) {
	char* entryName;
	for (uint32_t i = 0; i < entriesCount; i++) {
		entryName = fatFileNameToStr(entries[i].fileName);
		if (strlen(entryName) != strlen(fileName)) {
			continue;
		}
		if (strcmp(entryName, fileName) == 0) {
			free(entryName);
			return &entries[i];
		}
	}

	return NULL;
}

uint32_t getFileContentByPath(uint8_t** fileContent, const char* path) {
	FAT12DirectoryEntry* dirEntries = NULL;
	FAT12DirectoryEntry* entry = NULL;
	FAT12DirectoryEntry currentDirEntry;
	char* pathCopy = strdup(path);
	char* token = NULL;
	uint32_t entriesCount = 0;

	entriesCount = getRootDirectoryEntries(&dirEntries, &fat12Info, fat12LoopDevicePath);
	token = strtok(pathCopy, "/");

	while (token) {
		entry = getEntryByName(token, dirEntries, entriesCount);
		if (!entry) {
			free(pathCopy);
			fprintf(stderr, "File or directory does not exist: %s\n", path);
			exit(-1);
		}
		memcpy(&currentDirEntry, entry, sizeof(FAT12DirectoryEntry));

		if (!isDirectoryEntryDirectory(&currentDirEntry)) {
			free(pathCopy);
			return getFileContent(fileContent, &currentDirEntry, &fat12Info, fat12LoopDevicePath);
		}

		entriesCount =
			getDirectoryEntries(&dirEntries, &currentDirEntry, &fat12Info, fat12LoopDevicePath);
		entriesCount = filterValidDirectoryEntries(&dirEntries, entriesCount);
		token = strtok(NULL, "/");
	}

	free(pathCopy);
	fprintf(stderr, "File does not exist: %s\n", path);
	exit(-1);
	return 0;
}
