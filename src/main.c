#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define SECTOR_SIZE 512
typedef struct BPB
{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short		bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;
} __attribute__((packed)) BPB;


// I let AI genrate this function:
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


int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Invalid usage:\n");
		printf("Usage: FAT12Parser <loop_device_file>\n");
		exit(-1);
	}

	char* loopDevicePath = argv[1];
	int fat12FileDescriptor = open(loopDevicePath, O_RDONLY);
	if (fat12FileDescriptor == -1) {
		perror("Error opening loop device file");
	}

	char* buffer = malloc(SECTOR_SIZE);
	read(fat12FileDescriptor, buffer, SECTOR_SIZE);
	BPB* bpb = (BPB*)buffer;
	print_bpb(bpb);
	printf("bytes_per_sector: %d\n", bpb->bytes_per_sector);
}
