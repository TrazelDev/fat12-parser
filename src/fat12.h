#pragma once
#include <stdint.h>

typedef struct FAT12Header {
	// BPB:
	uint8_t bootjmp[3];
	uint8_t oemName[8];
	uint16_t bytesPerSector;
	uint8_t sectorsPerCluster;
	uint16_t reservedSectorCount;  // The number of reserved sectors
	uint8_t tableCount;
	uint16_t rootEntryCount;
	uint16_t totalSectors16;
	uint8_t mediaType;
	uint16_t tableSize16;  // The number of sectors per FAT
	uint16_t sectorsPerTrack;
	uint16_t headSideCount;
	uint32_t hiddenSectorCount;
	uint32_t totalSectors32;

	// fat12/fat16 specifics:
	uint8_t biosDriveNum;
	uint8_t reserved1;
	uint8_t bootSignature;
	uint32_t volumeId;
	uint8_t volumeLabel[11];
	uint8_t fatTypeLabel[8];
} __attribute__((packed)) FAT12Header;

typedef struct FAT12DirectoryEntry {
	char fileName[11];
	uint8_t attributes;
	uint8_t reserved;
	uint8_t creationTimeCentiseconds;
	uint16_t creationTime;
	uint16_t creationDate;
	uint16_t lastAccessDate;
	uint16_t zeroValue;	 // In fat12 and fat16 this is always zero
	uint16_t lastModifyTime;
	uint16_t lastModifyDate;
	uint16_t firstClusterNum;
	uint32_t fileSizeInBytes;
} __attribute__((packed)) FAT12DirectoryEntry;

typedef struct FAT12Info {
	uint32_t totalSectors;
	uint32_t fatSectorSize;
	uint32_t fatSectionSectorSize;
	uint32_t rootDirSectorsSize;
	uint32_t dataSectorsSize;
	uint32_t clusterCount;

	// the offset of sections:
	uint32_t dataSectionSectorOffset;
	uint32_t fatSectionSectorOffset;
	uint32_t rootDirSectorOffset;
} FAT12Info;

/** @brief gets an allocated fat12 header and loads the correct data for it from the loop device
 * @param fat12Header - allocated FAT12Header which the FAT12Header information will be loaded into
 * @param loopDevice - path to the loop device
 * @return NULL - failed to extract the data, fat12Header - in the case of success
 */
FAT12Header* loadFat12Header(FAT12Header* fat12Header, const char* loopDevicePath);

/** @brief loads fat12Info with all the correct values
 * @param fat12Header - loaded fat12Header with all the correct informatin
 * @param fat12Info - allocated FAT12Info which will be loaded the information of FAT12Info
 * @return NULL - failed to extract data, fat12Info in case of success
 */
FAT12Info* loadFat12Info(FAT12Header* fat12Header, FAT12Info* fat12Info);

// I let AI genrate this functions:
void printFat12Header(const FAT12Header* fat12Header);
void printFat12Info(const FAT12Info* fat12Info);
