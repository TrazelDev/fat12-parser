#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "fat12.h"

FAT12Header* load_fat12Header(FAT12Header* fat12Header, const char* loopDevicePath) {
	int fat12FileDescriptor = open(loopDevicePath, O_RDONLY);
	if (fat12FileDescriptor == -1) {
		perror("Error opening loop device file");
		return NULL;
	}

	char buffer[sizeof(FAT12Header)];
	read(fat12FileDescriptor, buffer, sizeof(FAT12Header));
	memcpy(fat12Header, buffer, sizeof(FAT12Header));
	return fat12Header;
}

void print_fat12Header(const FAT12Header* fat12Header) {
	if (fat12Header == NULL) {
		printf("Error: FAT12Header pointer is NULL\n");
		return;
	}

	printf("========== FAT12 Header ==========\n");

	// BPB Fields
	printf("Boot Jump:              %02X %02X %02X\n", fat12Header->bootjmp[0],
		   fat12Header->bootjmp[1], fat12Header->bootjmp[2]);

	printf("OEM Name:               %.8s\n", fat12Header->oem_name);
	printf("Bytes per Sector:       %u\n", fat12Header->bytes_per_sector);
	printf("Sectors per Cluster:    %u\n", fat12Header->sectors_per_cluster);
	printf("Reserved Sector Count:  %u\n", fat12Header->reserved_sector_count);
	printf("FAT Table Count:        %u\n", fat12Header->table_count);
	printf("Root Entry Count:       %u\n", fat12Header->root_entry_count);
	printf("Total Sectors (16-bit): %u\n", fat12Header->total_sectors_16);
	printf("Media Type:             0x%02X\n", fat12Header->media_type);
	printf("FAT Size (16-bit):      %u sectors\n", fat12Header->table_size_16);
	printf("Sectors per Track:      %u\n", fat12Header->sectors_per_track);
	printf("Number of Heads:        %u\n", fat12Header->head_side_count);
	printf("Hidden Sectors:         %u\n", fat12Header->hidden_sector_count);
	printf("Total Sectors (32-bit): %u\n", fat12Header->total_sectors_32);

	// FAT12/FAT16 Extended Fields
	printf("\n--- Extended Boot Record ---\n");
	printf("BIOS Drive Number:      0x%02X\n", fat12Header->bios_drive_num);
	printf("Reserved:               0x%02X\n", fat12Header->reserved1);
	printf("Boot Signature:         0x%02X\n", fat12Header->boot_signature);
	printf("Volume ID:              0x%08X\n", fat12Header->volume_id);
	printf("Volume Label:           %.11s\n", fat12Header->volume_label);
	printf("FAT Type Label:         %.8s\n", fat12Header->fat_type_label);

	printf("================================================\n");

	// Calculated values
	printf("\n--- Calculated Information ---\n");
	uint32_t total_sectors = fat12Header->total_sectors_16 ? fat12Header->total_sectors_16
														   : fat12Header->total_sectors_32;
	printf("Total Sectors:          %u\n", total_sectors);
	printf("Total Size:             %u bytes (%.2f KB)\n",
		   total_sectors * fat12Header->bytes_per_sector,
		   (total_sectors * fat12Header->bytes_per_sector) / 1024.0);
	printf("Cluster Size:           %u bytes\n",
		   fat12Header->sectors_per_cluster * fat12Header->bytes_per_sector);
	printf("Total FAT Size:         %u bytes\n",
		   fat12Header->table_count * fat12Header->table_size_16 * fat12Header->bytes_per_sector);
	printf("Root Directory Size:    %u bytes\n", fat12Header->root_entry_count * 32);
	printf("\n");
}
