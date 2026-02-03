#include "biosParameterBlock.h"
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

BPB* load_bpb(BPB* bpb, const char* loopDevicePath) {
	int fat12FileDescriptor = open(loopDevicePath, O_RDONLY);
	if (fat12FileDescriptor == -1) {
		perror("Error opening loop device file");
		return NULL;
	}

	char buffer[sizeof(BPB)];
	read(fat12FileDescriptor, buffer, sizeof(BPB));
	memcpy(bpb, buffer, sizeof(BPB));
	return bpb;
}


void print_bpb(const BPB* bpb)
{	
	if (bpb == NULL) {
		printf("Error: BPB pointer is NULL\n");
		return;
	}

	printf("========== BIOS Parameter Block (BPB) ==========\n");
	
	printf("Boot Jump:              %02X %02X %02X\n", 
		bpb->bootjmp[0], bpb->bootjmp[1], bpb->bootjmp[2]);
	
	printf("OEM Name:               %.8s\n", bpb->oem_name);
	
	printf("Bytes per Sector:       %u\n", bpb->bytes_per_sector);
	printf("Sectors per Cluster:    %u\n", bpb->sectors_per_cluster);
	printf("Reserved Sector Count:  %u\n", bpb->reserved_sector_count);
	printf("FAT Table Count:        %u\n", bpb->table_count);
	printf("Root Entry Count:       %u\n", bpb->root_entry_count);
	printf("Total Sectors (16-bit): %u\n", bpb->total_sectors_16);
	printf("Media Type:             0x%02X\n", bpb->media_type);
	printf("FAT Size (16-bit):      %u sectors\n", bpb->table_size_16);
	printf("Sectors per Track:      %u\n", bpb->sectors_per_track);
	printf("Number of Heads:        %u\n", bpb->head_side_count);
	printf("Hidden Sectors:         %u\n", bpb->hidden_sector_count);
	printf("Total Sectors (32-bit): %u\n", bpb->total_sectors_32);
	
	printf("================================================\n");
	
	// Calculated values
	printf("\n--- Calculated Information ---\n");
	unsigned int total_sectors = bpb->total_sectors_16 ? 
		bpb->total_sectors_16 : bpb->total_sectors_32;
	printf("Total Sectors:          %u\n", total_sectors);
	printf("Total Size:             %u bytes (%.2f KB)\n", 
		total_sectors * bpb->bytes_per_sector,
		(total_sectors * bpb->bytes_per_sector) / 1024.0);
	printf("Cluster Size:           %u bytes\n", 
		bpb->sectors_per_cluster * bpb->bytes_per_sector);
	printf("Total FAT Size:         %u bytes\n", 
		bpb->table_count * bpb->table_size_16 * bpb->bytes_per_sector);
	printf("Root Directory Size:    %u bytes\n", 
		bpb->root_entry_count * 32);
	printf("\n");
}

