#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "allocwrap.h"
#include "fat12.h"
#include "fat12_string.h"

void preadDevice(uint8_t* buffer, uint64_t readBytes, int64_t offset, const char* loopDevicePath) {
	int deviceFileDescriptor = open(loopDevicePath, O_RDONLY);
	if (deviceFileDescriptor == -1) {
		perror("Error opening loop device file");
		exit(-1);
	}

	ssize_t bytesRead = pread(deviceFileDescriptor, buffer, readBytes, offset);
	if (bytesRead == -1) {
		perror("File failed to be read");
		close(deviceFileDescriptor);
		exit(-1);
	}
	if (bytesRead < readBytes) {
		fprintf(stderr, "pread: file short (%lu out of %lu bytes read)\n", bytesRead, readBytes);
		close(deviceFileDescriptor);
		exit(-1);
	}

	close(deviceFileDescriptor);
}

FAT12Header* loadFat12Header(FAT12Header* fat12Header, const char* loopDevicePath) {
	static char buffer[sizeof(FAT12Header)];

	preadDevice((uint8_t*)buffer, sizeof(FAT12Header), 0, loopDevicePath);
	memcpy(fat12Header, buffer, sizeof(FAT12Header));

	return fat12Header;
}

FAT12Info* loadFat12Info(FAT12Info* fat12Info, FAT12Header* fat12Header) {
	FAT12Info* info = fat12Info;
	uint32_t rootDirBytes = fat12Header->rootEntryCount * sizeof(FAT12DirectoryEntry);

	if (fat12Header->totalSectors16) {
		info->totalSectors = fat12Header->totalSectors16;
	} else {
		info->totalSectors = fat12Header->totalSectors32;
	}
	info->bytesPerSector = fat12Header->bytesPerSector;
	info->sectorsPerCluster = fat12Header->sectorsPerCluster;
	info->fatSectorSize = fat12Header->tableSize16;
	info->fatSectionSectorSize = fat12Header->tableCount * info->fatSectorSize;
	info->rootDirSectorsSize = bytesToSectorsRoundUp(rootDirBytes, fat12Header->bytesPerSector);
	info->dataSectorsSize = info->totalSectors - fat12Header->reservedSectorCount -
							info->fatSectionSectorSize - info->rootDirSectorsSize;
	info->clusterCount = info->dataSectorsSize / fat12Header->sectorsPerCluster;

	info->dataSectionSectorOffset =
		fat12Header->reservedSectorCount + info->fatSectionSectorSize + info->rootDirSectorsSize;
	info->fatSectionSectorOffset = fat12Header->reservedSectorCount;
	info->rootDirSectorOffset = info->dataSectionSectorOffset - info->rootDirSectorsSize;

	return info;
}

uint32_t countValidEntries(FAT12DirectoryEntry* dirEntries, uint32_t maxEntries,
						   bool includeNoneFileOrDirEntries) {
	uint32_t count = 0;
	for (uint32_t i = 0; i < maxEntries; i++) {
		if (isFinalDirectoryEntry(&dirEntries[i])) {
			break;	// End of the directory
		}
		if (includeNoneFileOrDirEntries) {
			count++;
			continue;
		}
		if (!isDeletedEntry(&dirEntries[i]) && !isVolumeLabelEntry(&dirEntries[i])) {
			count++;
		}
	}

	return count;
}

uint32_t getDirectoryEntries(FAT12DirectoryEntry** dirs, uint32_t sectorOffset,
							 uint32_t directorySectorSize, uint32_t bytesPerSector,
							 const char* loopDevicePath) {
	const uint32_t DIRECTORY_BYTES_SIZE = directorySectorSize * bytesPerSector;
	const uint32_t BYTES_OFFSET = sectorOffset * bytesPerSector;
	FAT12DirectoryEntry* dirEntries = xmalloc(DIRECTORY_BYTES_SIZE);
	preadDevice((uint8_t*)dirEntries, DIRECTORY_BYTES_SIZE, BYTES_OFFSET, loopDevicePath);

	uint32_t maxEntries = DIRECTORY_BYTES_SIZE / sizeof(FAT12DirectoryEntry);
	uint32_t entriesCount = countValidEntries(dirEntries, maxEntries, true);
	dirEntries = xrealloc(dirEntries, entriesCount * sizeof(FAT12DirectoryEntry));

	*dirs = dirEntries;
	return entriesCount;
}

uint32_t getEntriesFileNames(char*** fileNames, FAT12DirectoryEntry* dirEntries,
							 uint32_t dirEntriesCount) {
	// Directories names are included in this count:
	uint32_t fileTypeEntriesCount = countValidEntries(dirEntries, dirEntriesCount, false);

	*fileNames = (char**)xmalloc(fileTypeEntriesCount * sizeof(char*));
	int nameIndex = 0;
	for (uint32_t i = 0; i < dirEntriesCount; i++) {
		if (isFinalDirectoryEntry(&dirEntries[i])) {
			break;
		}
		if (isDeletedEntry(&dirEntries[i])) {
			continue;
		}
		if (isVolumeLabelEntry(&dirEntries[i])) {
			continue;
		}

		char* val = fatFileNameToStr(dirEntries[i].fileName);
		(*fileNames)[nameIndex] = val;
		nameIndex++;
	}

	return fileTypeEntriesCount;
}

uint32_t getRootFileNames(char*** names, FAT12Info* fat12Info, const char* loopDevicePath) {
	FAT12DirectoryEntry* dirEntries;
	uint32_t entriesCount = getDirectoryEntries(&dirEntries, fat12Info->rootDirSectorOffset,
												fat12Info->rootDirSectorsSize,
												fat12Info->bytesPerSector, loopDevicePath);

	uint32_t fileNamesCount = getEntriesFileNames(names, dirEntries, entriesCount);
	free(dirEntries);
	return fileNamesCount;
}

uint32_t filterValidDirectoryEntries(FAT12DirectoryEntry** dirEntries, uint32_t entriesCount) {
	// Directories names are included in this count:
	uint32_t fileTypeEntriesCount = countValidEntries(*dirEntries, entriesCount, false);

	FAT12DirectoryEntry* filteredEntries =
		xmalloc(fileTypeEntriesCount * sizeof(FAT12DirectoryEntry));
	int validIndex = 0;
	FAT12DirectoryEntry* currElement;
	for (uint32_t i = 0; i < entriesCount; i++) {
		currElement = &(*dirEntries)[i];
		if (isFinalDirectoryEntry(currElement)) {
			break;
		}
		if (isDeletedEntry(currElement)) {
			continue;
		}
		if (isVolumeLabelEntry(currElement)) {
			continue;
		}

		memcpy(&filteredEntries[validIndex], currElement, sizeof(FAT12DirectoryEntry));
		validIndex++;
	}
	free(*dirEntries);
	*dirEntries = filteredEntries;
	return fileTypeEntriesCount;
}

uint32_t getFileContent(uint8_t** fileContent, FAT12DirectoryEntry* fileDirectoryEntry,
						FAT12Info* fat12Info, const char* loopDevicePath) {
	assert(fileDirectoryEntry->fileSizeInBytes != 0);
	const uint32_t BYTES_PER_CLUSTER = fat12Info->bytesPerSector * fat12Info->sectorsPerCluster;

	uint8_t* fat = getFat(fat12Info, loopDevicePath);
	uint32_t fileClusterCount = countFileClusters(fileDirectoryEntry->firstClusterId, fat);
	*fileContent = xmalloc((uint64_t)fileClusterCount * BYTES_PER_CLUSTER);
	uint8_t* currFileContentPtr = *fileContent;

	uint32_t currClusterId = fileDirectoryEntry->firstClusterId;
	char* clusterData;
	for (uint32_t i = 0; i < fileClusterCount; i++) {
		readCluster(&clusterData, currClusterId, fat12Info, loopDevicePath);
		memcpy(currFileContentPtr, clusterData, BYTES_PER_CLUSTER);
		free(clusterData);

		currFileContentPtr += BYTES_PER_CLUSTER;
		currClusterId = getNextClusterId(currClusterId, fat);
	}

	if (isDirectoryEntryDirectory(fileDirectoryEntry)) {
		return BYTES_PER_CLUSTER * fileClusterCount;
	}
	return fileDirectoryEntry->fileSizeInBytes;
}

uint8_t* getFat(FAT12Info* fat12Info, const char* loopDevicePath) {
	const uint32_t FAT12_TABLE_SIZE = fat12Info->fatSectorSize * fat12Info->bytesPerSector;
	const uint32_t FAT_BYTE_OFFSET = fat12Info->bytesPerSector * fat12Info->fatSectionSectorOffset;

	uint8_t* fat = xmalloc(FAT12_TABLE_SIZE);
	preadDevice(fat, FAT12_TABLE_SIZE, FAT_BYTE_OFFSET, loopDevicePath);

	return fat;
}

uint16_t getNextClusterId(uint16_t clusterId, const uint8_t* fat) {
	uint32_t offset = clusterId + (clusterId / 2);
	int packed = fat[offset] | (fat[offset + 1] << 8);

	if (clusterId % 2) {
		return packed >> 4;
	}
	return packed & 0x0FFF;
}

uint32_t countFileClusters(uint16_t initialClusterId, const uint8_t* fat) {
	uint32_t clusterCount = 0;
	uint16_t currClusterId = initialClusterId;
	while (currClusterId != FAT_LAST_CLUSTER_NUM) {
		clusterCount++;
		currClusterId = getNextClusterId(currClusterId, fat);
	}

	return clusterCount;
}

void printFileAllocationTable(FAT12Info* fat12Info, const char* loopDevicePath) {
	const uint32_t FAT12_TABLE_BYTES = fat12Info->fatSectorSize * fat12Info->bytesPerSector;
	uint8_t* fat = getFat(fat12Info, loopDevicePath);

	uint32_t maxEntries = (FAT12_TABLE_BYTES * 2) / 3;
	for (int i = 0; i < maxEntries; i++) {
		uint16_t pointerIndex = getNextClusterId(i, fat);

		printf("%x -> %x\n", i, pointerIndex);
	}
}

uint32_t readCluster(char** data, uint16_t clusterId, FAT12Info* fat12Info,
					 const char* loopDevicePath) {
	uint32_t clusterNum = clusterIdToClusterNum(clusterId);
	uint32_t bytesPerCluster = fat12Info->sectorsPerCluster * fat12Info->bytesPerSector;
	uint32_t dataSectionSectorOffset = clusterNum * fat12Info->sectorsPerCluster;
	uint32_t deviceSectorOffset = fat12Info->dataSectionSectorOffset + dataSectionSectorOffset;
	uint32_t deviceBytesOffset = deviceSectorOffset * fat12Info->bytesPerSector;

	*data = xmalloc(bytesPerCluster);
	preadDevice((uint8_t*)*data, bytesPerCluster, deviceBytesOffset, loopDevicePath);
	return bytesPerCluster;
}
uint32_t getRootDirectoryEntries(FAT12DirectoryEntry** dirEntries, FAT12Info* fat12Info,
								 const char* loopDevicePath) {
	uint32_t entriesCount = getDirectoryEntries(dirEntries, fat12Info->rootDirSectorOffset,
												fat12Info->rootDirSectorsSize,
												fat12Info->bytesPerSector, loopDevicePath);
	return filterValidDirectoryEntries(dirEntries, entriesCount);
}

// NOLINTBEGIN
void printFat12Header(const FAT12Header* fat12Header) {
	if (fat12Header == NULL) {
		printf("Error: FAT12Header pointer is NULL\n");
		return;
	}

	printf("========== FAT12 Header ==========\n");

	// BPB Fields
	printf("Boot Jump:              %02X %02X %02X\n", fat12Header->bootjmp[0],
		   fat12Header->bootjmp[1], fat12Header->bootjmp[2]);

	printf("OEM Name:               %.8s\n", fat12Header->oemName);
	printf("Bytes per Sector:       %u\n", fat12Header->bytesPerSector);
	printf("Sectors per Cluster:    %u\n", fat12Header->sectorsPerCluster);
	printf("Reserved Sector Count:  %u\n", fat12Header->reservedSectorCount);
	printf("FAT Table Count:        %u\n", fat12Header->tableCount);
	printf("Root Entry Count:       %u\n", fat12Header->rootEntryCount);
	printf("Total Sectors (16-bit): %u\n", fat12Header->totalSectors16);
	printf("Media Type:             0x%02X\n", fat12Header->mediaType);
	printf("FAT Size (16-bit):      %u sectors\n", fat12Header->tableSize16);
	printf("Sectors per Track:      %u\n", fat12Header->sectorsPerTrack);
	printf("Number of Heads:        %u\n", fat12Header->headSideCount);
	printf("Hidden Sectors:         %u\n", fat12Header->hiddenSectorCount);
	printf("Total Sectors (32-bit): %u\n", fat12Header->totalSectors32);

	// FAT12/FAT16 Extended Fields
	printf("\n--- Extended Boot Record ---\n");
	printf("BIOS Drive Number:      0x%02X\n", fat12Header->biosDriveNum);
	printf("Reserved:               0x%02X\n", fat12Header->reserved1);
	printf("Boot Signature:         0x%02X\n", fat12Header->bootSignature);
	printf("Volume ID:              0x%08X\n", fat12Header->volumeId);
	printf("Volume Label:           %.11s\n", fat12Header->volumeLabel);
	printf("FAT Type Label:         %.8s\n", fat12Header->fatTypeLabel);

	printf("================================================\n");

	// Calculated values
	printf("\n--- Calculated Information ---\n");
	uint32_t totalSectors =
		fat12Header->totalSectors16 ? fat12Header->totalSectors16 : fat12Header->totalSectors32;
	printf("Total Sectors:          %u\n", totalSectors);
	printf("Total Size:             %u bytes (%.2f KB)\n",
		   totalSectors * fat12Header->bytesPerSector,
		   (totalSectors * fat12Header->bytesPerSector) / 1024.0);
	printf("Cluster Size:           %u bytes\n",
		   fat12Header->sectorsPerCluster * fat12Header->bytesPerSector);
	printf("Total FAT Size:         %u bytes\n",
		   fat12Header->tableCount * fat12Header->tableSize16 * fat12Header->bytesPerSector);
	printf("Root Directory Size:    %u bytes\n", fat12Header->rootEntryCount * 32);
	printf("\n");
}

void printFat12Info(const FAT12Info* fat12Info) {
	if (fat12Info == NULL) {
		printf("Error: FAT12Info pointer is NULL\n");
		return;
	}

	printf("========== FAT12 Info ==========\n");

	// Sector counts
	printf("Total Sectors:          %u\n", fat12Info->totalSectors);
	printf("FAT Sector Size:        %u sectors\n", fat12Info->fatSectorSize);
	printf("FAT Section Size:       %u sectors\n", fat12Info->fatSectionSectorSize);
	printf("Root Dir Size:          %u sectors\n", fat12Info->rootDirSectorsSize);
	printf("Data Section Size:      %u sectors\n", fat12Info->dataSectorsSize);
	printf("Cluster Count:          %u clusters\n", fat12Info->clusterCount);

	// Section offsets
	printf("\n--- Section Offsets (in sectors) ---\n");
	printf("FAT Section Offset:     %u\n", fat12Info->fatSectionSectorOffset);
	printf("Root Dir Offset:        %u\n", fat12Info->rootDirSectorOffset);
	printf("Data Section Offset:    %u\n", fat12Info->dataSectionSectorOffset);

	printf("================================================\n");
	printf("\n");
}

void printFat12DirectoryEntry(const FAT12DirectoryEntry* e) {
	if (!e) {
		printf("Error: FAT12DirectoryEntry pointer is NULL\n");
		return;
	}

	printf("========== FAT12 Directory Entry ==========\n");

	printf("Raw 8.3 Name:           '%.11s'\n", e->fileName);
	printf("Attributes:             0x%02X\n", e->attributes);
	printf("Reserved:               0x%02X\n", e->reserved);

	// Decode FAT time/date (optional but useful)
	{
		uint16_t t = e->creationTime;
		uint16_t d = e->creationDate;
		unsigned sec = (t & 0x1F) * 2;
		unsigned min = (t >> 5) & 0x3F;
		unsigned hour = (t >> 11) & 0x1F;
		unsigned day = d & 0x1F;
		unsigned mon = (d >> 5) & 0x0F;
		unsigned year = 1980 + ((d >> 9) & 0x7F);

		printf("Creation Time (raw):    0x%04X\n", e->creationTime);
		printf("Creation Date (raw):    0x%04X\n", e->creationDate);
		printf("Created (decoded):      %04u-%02u-%02u %02u:%02u:%02u +%u cs\n", year, mon, day,
			   hour, min, sec, (unsigned)e->creationTimeCentiseconds);
	}

	{
		uint16_t d = e->lastAccessDate;
		unsigned day = d & 0x1F;
		unsigned mon = (d >> 5) & 0x0F;
		unsigned year = 1980 + ((d >> 9) & 0x7F);

		printf("Last Access Date (raw): 0x%04X\n", e->lastAccessDate);
		printf("Last Access (decoded):  %04u-%02u-%02u\n", year, mon, day);
	}

	{
		uint16_t t = e->lastModifyTime;
		uint16_t d = e->lastModifyDate;
		unsigned sec = (t & 0x1F) * 2;
		unsigned min = (t >> 5) & 0x3F;
		unsigned hour = (t >> 11) & 0x1F;
		unsigned day = d & 0x1F;
		unsigned mon = (d >> 5) & 0x0F;
		unsigned year = 1980 + ((d >> 9) & 0x7F);

		printf("Last Modify Time (raw): 0x%04X\n", e->lastModifyTime);
		printf("Last Modify Date (raw): 0x%04X\n", e->lastModifyDate);
		printf("Last Modified(decoded): %04u-%02u-%02u %02u:%02u:%02u\n", year, mon, day, hour, min,
			   sec);
	}

	printf("zeroValue:              0x%04X\n", e->zeroValue);
	printf("First Cluster:          %u (0x%04X)\n", e->firstClusterId, e->firstClusterId);
	printf("File Size:              %u bytes (0x%08X)\n", e->fileSizeInBytes, e->fileSizeInBytes);

	// A couple common entry markers to help debugging dumps
	printf("Entry marker:           0x%02X (fileName[0])\n", (unsigned char)e->fileName[0]);
	if ((unsigned char)e->fileName[0] == 0x00) printf("  -> End of directory\n");
	if ((unsigned char)e->fileName[0] == 0xE5) printf("  -> Deleted entry\n");
	if (e->attributes == 0x0F) printf("  -> Long File Name (LFN) entry\n");

	printf("================================================\n\n");
}
// NOLINTEND
