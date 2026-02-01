all: run

SRCS_DIR := src
BINS_DIR := bin

FAT12_BIN := fat12_fs.bin
FAT12_MOUNT_DIR := _temp_dir/
TARGET := $(BINS_DIR)/FAT12Parser

SRCS = $(shell find ./$(SRCS_DIR) -type f -name *.c)
HEADERS = $(shell find ./$(SRCS_DIR) -type f -name *.h)
OBJS = $(patsubst ./$(SRCS_DIR)/%.c,./$(BINS_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)


CC := gcc
CFLAGS := -MMD -MP
LDFLAGS := -lm

run: build
	./$(TARGET)
build: $(TARGET)

create_loop_device: $(FAT12_BIN)
clean:
	rm -rf $(BINS_DIR)/

# depend:
# 	$(CC) -MM $(SRCS) > $(DEPS)
# 	# fixing multipe dirs in DEPS
# 	@sed -i -E "s/^(.+?).o: ([^ ]+?)\1/\2\1.o: \2\1/g" $(DEPS)

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) -o $@ $(OBJS)

./$(BINS_DIR)/%.o: ./$(SRCS_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

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



-include $(DEPS)
