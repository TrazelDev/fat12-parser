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

static FAT12DirectoryEntry* getEntryByName(const char* fileName, FAT12DirectoryEntry* entries,
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

static FAT12DirectoryEntry* getPathFinalDirectoryEntry(const char* path) {
	char* pathCopy = strdup(path);
	char* token = strtok(pathCopy, "/");
	FAT12DirectoryEntry* entry = NULL;
	FAT12DirectoryEntry* currentDirEntry = malloc(sizeof(FAT12DirectoryEntry));
	FAT12DirectoryEntry* dirEntries;
	uint32_t entriesCount = getRootDirectoryEntries(&dirEntries, &fat12Info, fat12LoopDevicePath);

	while (token) {
		entry = getEntryByName(token, dirEntries, entriesCount);
		if (!entry) {
			free(pathCopy);
			return NULL;
		}
		memcpy(currentDirEntry, entry, sizeof(FAT12DirectoryEntry));
		free(dirEntries);

		if (!isDirectoryEntryDirectory(currentDirEntry)) {
			if (strtok(NULL, "/")) {
				return NULL;
			}
			free(pathCopy);
			return currentDirEntry;
		}
		entriesCount =
			getDirectoryEntries(&dirEntries, currentDirEntry, &fat12Info, fat12LoopDevicePath);
		entriesCount = filterValidDirectoryEntries(&dirEntries, entriesCount);
		token = strtok(NULL, "/");
	}

	free(pathCopy);
	free(dirEntries);
	return currentDirEntry;
}

uint32_t getFileContentByPath(uint8_t** fileContent, const char* path) {
	FAT12DirectoryEntry* finalEntry = getPathFinalDirectoryEntry(path);
	if (!finalEntry) {
		(void)fprintf(stderr, "File does not exist: %s\n", path);
		free(finalEntry);
		return -1;
	}
	if (isDirectoryEntryDirectory(finalEntry)) {
		(void)fprintf(stderr, "Path is a directory, not a file: %s\n", path);
		free(finalEntry);
		return -1;
	}

	uint32_t fileSize = getFileContent(fileContent, finalEntry, &fat12Info, fat12LoopDevicePath);
	free(finalEntry);
	return fileSize;
}

uint32_t getFileNamesByPath(char*** filesNames, const char* path) {
	if (strlen(path) == 1 && strcmp(path, "/") == 0) {
		return getRootFileNames(filesNames, &fat12Info, fat12LoopDevicePath);
	}

	FAT12DirectoryEntry* finalEntry = getPathFinalDirectoryEntry(path);
	if (!finalEntry) {
		(void)fprintf(stderr, "Directory does not exist: %s\n", path);
		return -1;
	}
	if (!isDirectoryEntryDirectory(finalEntry)) {
		(void)fprintf(stderr, "Path is a file, not a directory: %s\n", path);
		return -1;
	}

	FAT12DirectoryEntry* dirEntries;
	uint32_t entriesCount =
		getDirectoryEntries(&dirEntries, finalEntry, &fat12Info, fat12LoopDevicePath);
	entriesCount = filterValidDirectoryEntries(&dirEntries, entriesCount);
	getEntriesFileNames(filesNames, dirEntries, entriesCount);

	free(finalEntry);
	free(dirEntries);
	return entriesCount;
}
