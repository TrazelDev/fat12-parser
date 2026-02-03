#include <stdio.h>
#include <stdlib.h>
#include "biosParameterBlock.h"


int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Invalid usage:\n");
		printf("Usage: FAT12Parser <loop_device_file>\n");
		exit(-1);
	}

	char* loopDevicePath = argv[1];
	BPB* bpb = malloc(sizeof(BPB));

	if (!load_bpb(bpb, loopDevicePath)) {
		free(bpb);
		exit(-1);
	}
	print_bpb(bpb);
}
