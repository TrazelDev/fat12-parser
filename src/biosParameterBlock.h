#pragma once
#include <stdint.h>

typedef struct BPB
{
	uint8_t  bootjmp[3];
	uint8_t  oem_name[8];
	uint16_t bytes_per_sector;
	uint8_t  sectors_per_cluster;
	uint16_t reserved_sector_count;
	uint8_t  table_count;
	uint16_t root_entry_count;
	uint16_t total_sectors_16;
	uint8_t  media_type;
	uint16_t table_size_16;
	uint16_t sectors_per_track;
	uint16_t head_side_count;
	uint32_t hidden_sector_count;
	uint32_t total_sectors_32;
} __attribute__((packed)) BPB;

/** @brief gets an allocated Bios block parameter and loads the correct data
  * for it from the loop device
  * @return NULL - failed to extract the data, bpb - in the case of success
*/
BPB* load_bpb(BPB* bpb, const char* loopDevicePath);

// I let AI genrate this function:
void print_bpb(const BPB* bpb);
