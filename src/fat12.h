#pragma once
#include <stdbool.h>
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

#define FINAL_ENTRY 0x00
#define DELETED_ENTRY 0xE5
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
	uint32_t bytesPerSector;
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

/** Loads FAT12Header with information from a loop device.
 * @param[out] fat12Header Pointer to the allocated structure to load information to.
 * @param[in] loopDevicePath
 * @return A pointer to fat12Header.
 */
FAT12Header* loadFat12Header(FAT12Header* fat12Header, const char* loopDevicePath);

/** Loads FAT12Info from FAT12Header.
 * @param[out] fat12Info Pointer to the allocated structure to load information to.
 * @param[in] fat12Header A FAT12Header structure with correct information of some loop device.
 * @return A pointer fat12Info.
 */
FAT12Info* loadFat12Info(FAT12Info* fat12Info, FAT12Header* fat12Header);

/** Gets the root folder file names in an array from the loopDevice using FAT12Info
 * @param[out] names Address of char** variable. The function will allocate it.
 * @note Caller will free each string in the array then the array itself.
 * @param[in] fat12Info A FAT12Info that was created from the loopDevice.
 * @param[in] loopDevicePath.
 * @return The count of file names in names variable.
 */
uint32_t getRootFileNames(char*** names, FAT12Info* fat12Info, const char* loopDevicePath);

/** Extracts fat12 directory entries of specific directory from the loopDevice provided.
 * @param[out] dirs Pointer to an array of FAT12DirectoryEntry that will be allocated internally.
 * @note Caller will free the array.
 * @param[in] sectorOffset Starting sector number of directory that entries will be taken from.
 * @param[in] directorySectorSize.
 * @param[in] bytesPerSector bytes per sector for this specific fat12.
 * @param[in] loopDevicePath
 * @return Amount of directory entries in variable dirs.
 */
uint32_t getDirectoryEntries(FAT12DirectoryEntry** dirs, uint32_t sectorOffset,
							 uint32_t directorySectorSize, uint32_t bytesPerSector,
							 const char* loopDevicePath);

// I let AI genrate this functions:
void printFat12Header(const FAT12Header* fat12Header);
void printFat12Info(const FAT12Info* fat12Info);
