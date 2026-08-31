CC = i686-elf-gcc
AS = i686-elf-as 
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -mno-sse -mno-mmx -Iinclude
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc
SRC_DIR = src
BUILD_DIR = build

C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
S_SOURCES = $(wildcard $(SRC_DIR)/*.s)

OBJS = $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(S_SOURCES)) \
       $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))

all: myos.iso

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s 
	@mkdir -p $(BUILD_DIR)
	$(AS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c 
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

myos.bin: $(OBJS) linker.ld 
	$(CC) -T linker.ld -o myos.bin $(OBJS) $(LDFLAGS)

myos.iso: myos.bin grub.cfg 
	mkdir -p isodir/boot/grub 
	cp myos.bin isodir/boot/myos.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

run: myos.iso   
	qemu-system-i386 -cdrom myos.iso

run-kernel: myos.bin
	qemu-system-i386 -kernel myos.bin -no-reboot -d int -D qemu.log

clean:
	rm -rf $(BUILD_DIR) myos.bin myos.iso isodir qemu.log