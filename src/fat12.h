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
	uint16_t firstClusterId;
	uint32_t fileSizeInBytes;
} __attribute__((packed)) FAT12DirectoryEntry;
#define FINAL_ENTRY 0x00
#define DELETED_ENTRY 0xE5
#define VOLUME_LABEL_ATTRIBUTE 0x08
static inline bool isFinalDirectoryEntry(FAT12DirectoryEntry* entry) {
	return entry->fileName[0] == FINAL_ENTRY;
}
static inline bool isVolumeLabelEntry(FAT12DirectoryEntry* entry) {
	return entry->attributes & VOLUME_LABEL_ATTRIBUTE;
}
static inline bool isDeletedEntry(FAT12DirectoryEntry* entry) {
	return ((uint8_t)entry->fileName[0] == DELETED_ENTRY);
}

#define FAT_LAST_CLUSTER_NUM 0xFFF
typedef struct FAT12Info {
	uint32_t bytesPerSector;
	uint32_t sectorsPerCluster;
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
 *
 * @param[out] names Address of char** variable. The function will allocate it.
 * @note Caller will free each string in the array then the array itself.
 * @param[in] fat12Info A FAT12Info that was created from the loopDevice.
 * @param[in] loopDevicePath.
 *
 * @return The count of file names in names variable.
 */
uint32_t getRootFileNames(char*** names, FAT12Info* fat12Info, const char* loopDevicePath);

/** Extracts fat12 directory entries of specific directory from the loopDevice provided.
 * This entries include:
 * directories, files, deleted entries, long file name entries and volume label entries.
 *
 * @param[out] dirs Pointer to an array of FAT12DirectoryEntry that will be allocated internally.
 * @note Caller will free the array.
 * @param[in] sectorOffset Starting sector number of directory that entries will be taken from.
 * @param[in] directorySectorSize.
 * @param[in] bytesPerSector bytes per sector for this specific fat12.
 * @param[in] loopDevicePath
 *
 * @return Amount of directory entries in variable dirs.
 */
uint32_t getDirectoryEntries(FAT12DirectoryEntry** dirs, uint32_t sectorOffset,
							 uint32_t directorySectorSize, uint32_t bytesPerSector,
							 const char* loopDevicePath);

/** @brief Filters a FAT12 directory array and keeps only file and directory entries.
 *
 * Function reads entries from *dirEntries and allocates a new array into *dirEntries after a
 * filter.
 *
 * @param[in,out] dirEntries Pointer to an array of FAT12DirectoryEntry that already contains
 * entries, on input holds pointer to an unfiltered array of FAT12DirectoryEntries, with
 * entriesCount elements, while on output holds pointer to a filtered array of
 * FAT12DirectoryEntries.
 * @note Input array freed by function and output array is then freed by caller.
 * @param[in] entriesCount Amuont of entries in *dirEntries.
 *
 * @return Number of entries in the new *dirEntries
 */
uint32_t filterValidDirectoryEntries(FAT12DirectoryEntry** dirEntries, uint32_t entriesCount);

/**
 * @brief Reads the contents of a FAT12 file into a newly allocated buffer.
 *
 * This function reads the file described by fileDirectoryEntry from the FAT12 filesystem
 * described by fat12Info located at loopDevicePath. The file data is returned via fileContent
 * and the size of it as return value. The caller owns the returned buffer.
 *
 * @param[out] fileContent On success, set to a newly allocated buffer containing the file's bytes.
 * @note The caller must free the buffer.
 * @param[in] fileDirectoryEntry Directory entry describing the file to read.
 * @param[in] fat12Info
 * @param[in] loopDevicePath Path to loop device.
 *
 * @return Number of bytes written to fileContent on success. Returns 0 on failure.
 */
uint32_t getFileContent(uint8_t** fileContent, FAT12DirectoryEntry* fileDirectoryEntry,
						FAT12Info* fat12Info, const char* loopDevicePath);

/** Loads the FAT (File Allocation Table) from a FAT12 loopDevice.
 * The function reads the first FAT in the FAT section of the filesystem located in loopDevice.
 * @param[in] fat12Info
 * @param[in] loopDevicePath Path to filesystem loop device.
 * @return Fat loaded into memory.
 * @note Caller will free the returned fat.
 */
uint8_t* getFat(FAT12Info* fat12Info, const char* loopDevicePath);

/**
 * @brief Reads a single FAT12 cluster into a newly allocated buffer.
 * This function read the cluster identified by clusterId from the fat12 filesystem located on the
 * loop device loopDevicePath points to.
 *
 * @param[out] data Set to a newly allocated buffer containing the clusters bytes.
 * @note Caller will free the memory
 * @param[in] clusterId Cluster id to read (clusterId=2 means the first cluster which internally
 * converted to cluster number).
 * @param[in] fat12Info
 * @param[in] loopDevicePath
 *
 * @return Number of bytes written to data (probably cluster size).
 */
uint32_t readCluster(char** data, uint16_t clusterId, FAT12Info* fat12Info,
					 const char* loopDevicePath);

/** Extracts FAT12 root directory entries from the loopDevice provided.
 * This directory entries only include: directories, files
 *
 * @param[out] dirs Pointer to an array of FAT12DirectoryEntry that will be allocated internally.
 * @note Caller will free the array.
 * @param[in] fat12Info A FAT12Info that was created from the loopDevice.
 * @param[in] loopDevicePath Path to filesystem loop device.
 *
 * @return Amount of root directory entries in variable dirs.
 */
uint32_t getRootDirectoryEntries(FAT12DirectoryEntry** dirEntries, FAT12Info* fat12Info,
								 const char* loopDevicePath);

/** Counts clusters for a specific cluster chain from the fat 12 table*/
uint32_t countFileClusters(uint16_t initialClusterId, const uint8_t* fat);
/** Gets a cluster id and fat 12 and returns next cluster in the chain.
 @note No error handling assumes values are correct.
 */
uint16_t getNextClusterId(uint16_t clusterId, const uint8_t* fat);
/** Converts clusterId to cluster number since the first id is 2 which points to cluster 0 */
static inline uint32_t clusterIdToClusterNum(uint16_t clusterId) { return clusterId - 2; }
void printFileAllocationTable(FAT12Info* fat12Info, const char* loopDevicePath);

// I let AI generate this functions:
// NOLINTBEGIN
void printFat12Header(const FAT12Header* fat12Header);
void printFat12Info(const FAT12Info* fat12Info);
void printFat12DirectoryEntry(const FAT12DirectoryEntry* e);
// NOLINTEND
