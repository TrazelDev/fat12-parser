#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fat12.h"
#include "utils.h"

FAT12Header* loadFat12Header(FAT12Header* fat12Header, const char* loopDevicePath) {
	int fat12FileDescriptor = open(loopDevicePath, O_RDONLY);
	if (fat12FileDescriptor == -1) {
		perror("Error opening loop device file");
		exit(-1);
	}

	char buffer[sizeof(FAT12Header)];
	if (read(fat12FileDescriptor, buffer, sizeof(FAT12Header)) == -1) {
		perror("File failed to be read");
		exit(-1);
	}
	memcpy(fat12Header, buffer, sizeof(FAT12Header));

	close(fat12FileDescriptor);
	return fat12Header;
}

FAT12Info* loadFat12Info(FAT12Header* fat12Header, FAT12Info* fat12Info) {
	FAT12Info* info = fat12Info;
	uint32_t rootDirBytes = fat12Header->rootEntryCount * sizeof(FAT12DirectoryEntry);

	if (fat12Header->totalSectors16) {
		info->totalSectors = fat12Header->totalSectors16;
	} else {
		info->totalSectors = fat12Header->totalSectors32;
	}
	info->bytesPerSector = fat12Header->bytesPerSector;
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

uint32_t countValidEntries(FAT12DirectoryEntry* dirEntries, int maxEntries,
						   bool includeDeltedEntries) {
	int count = 0;
	for (int i = 0; i < maxEntries; i++) {
		if (dirEntries[i].fileName[0] == FINAL_ENTRY) break;  // End of directory
		if (includeDeltedEntries | ((uint8_t)dirEntries[i].fileName[0] == UNUSED_ENTRY)) count++;
	}
	return count;
}

FAT12DirectoryEntry* getDirectoryEntries(uint32_t sectorOffset, uint32_t directorySectorSize,
										 const char* loopDevicePath, FAT12Info* fat12Info) {
	int fat12FileDescriptor = open(loopDevicePath, O_RDONLY);
	if (fat12FileDescriptor == -1) {
		perror("Error opening loop device file");
		exit(-1);
	}

	uint32_t directoryBytesSize = directorySectorSize * fat12Info->bytesPerSector;
	uint32_t bytesOffset = sectorOffset * fat12Info->bytesPerSector;
	FAT12DirectoryEntry* dirEntries = xmalloc(directoryBytesSize);

	if (pread(fat12FileDescriptor, dirEntries, directoryBytesSize, bytesOffset) <
		directoryBytesSize) {
		perror("File failed to be read");
		exit(-1);
	}

	uint32_t maxEntries = directoryBytesSize / sizeof(FAT12DirectoryEntry);
	uint32_t entriesCount = countValidEntries(dirEntries, maxEntries, true);
	dirEntries = xrealloc(dirEntries, (entriesCount + 1) * sizeof(FAT12DirectoryEntry));

	close(fat12FileDescriptor);
	return dirEntries;
}

char** getEntriesFileNames(FAT12DirectoryEntry* dirEntries, uint32_t maxEntries) {
	int count = 0;
	for (uint32_t i = 0; i < maxEntries; i++) {
		if (dirEntries[i].fileName[0] == FINAL_ENTRY) break;
		if ((uint8_t)dirEntries[i].fileName[0] == UNUSED_ENTRY) continue;
		if (dirEntries[i].attributes & 0x08) continue;	// skipping volume labels
		count++;
	}

	char** fileNames = xmalloc((count + 1) * sizeof(char*));
	int nameIndex = 0;
	for (uint32_t i = 0; i < maxEntries; i++) {
		if (dirEntries[i].fileName[0] == FINAL_ENTRY) break;
		if ((uint8_t)dirEntries[i].fileName[0] == UNUSED_ENTRY) continue;
		if (dirEntries[i].attributes & 0x08) continue;	// skipping volume labels

		fileNames[nameIndex] = xmalloc(sizeof(dirEntries[i].fileName) + 2);
		memcpy(fileNames[nameIndex], dirEntries[i].fileName, 8);
		fileNames[nameIndex][8] = '.';
		memcpy(fileNames[nameIndex] + 9, dirEntries[i].fileName + 8, 3);
		fileNames[nameIndex][12] = '\0';

		nameIndex++;
	}

	fileNames[nameIndex] = NULL;
	return fileNames;
}

char** getRootFileNames(FAT12Info* fat12Info, const char* loopDevice) {
	FAT12DirectoryEntry* dirEntries = getDirectoryEntries(
		fat12Info->rootDirSectorOffset, fat12Info->rootDirSectorsSize, loopDevice, fat12Info);

	uint32_t rootDirBytesSize = fat12Info->rootDirSectorsSize * fat12Info->bytesPerSector;
	uint32_t maxEntries = rootDirBytesSize / sizeof(FAT12DirectoryEntry);
	char** fileNames = getEntriesFileNames(dirEntries, maxEntries);

	free(dirEntries);
	return fileNames;
}

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
