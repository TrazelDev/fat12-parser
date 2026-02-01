CXX := g++

FAT12_BIN := fat12_fs.bin
FAT12_MOUNT_DIR := _temp_dir/

create_loop_device: $(FAT12_BIN)
clean:
	sudo rm $(FAT12_BIN)


$(FAT12_BIN):
	dd if=/dev/zero of=$(FAT12_BIN) bs=1024 count=2000
	mkfs.fat -F 12 $(FAT12_BIN)
	mkdir -p $(FAT12_MOUNT_DIR)
	sudo mount -o loop $(FAT12_BIN) $(FAT12_MOUNT_DIR)
	# loop is used because this is a virtual filing system within a file
	# opening a shell for you to create files inside of dir:
	sudo bash -c "cd $(FAT12_MOUNT_DIR) ; exec bash"
	sudo umount $(FAT12_MOUNT_DIR)
	sudo rm -r $(FAT12_MOUNT_DIR)
	# Info: Untill the loop devices is mounted it is not gurnteed that the new
	# information will be written in the file



