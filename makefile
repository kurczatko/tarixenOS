BUILD_DIR := build
ISO_DIR := $(BUILD_DIR)/iso

# BIOS32
BOOT_BIN32 := $(BUILD_DIR)/boot.bin
KERNEL_ELF32 := $(BUILD_DIR)/kernel32.elf
KERNEL_BIN32 := $(BUILD_DIR)/kernel32.bin
OS_IMAGE := $(BUILD_DIR)/tarixenios.img
KERNEL_MAX_SECTORS := 20
FLOPPY_SIZE := 1474560

UEFI_BIN := $(BUILD_DIR)/uefi64.bin
KERNEL_ELF64 := $(BUILD_DIR)/kernel64.elf
ISO_IMAGE := $(BUILD_DIR)/tarixenios.iso

CC32 := cc
CC64 := cc
LD32 := ld
LD64 := ld
OBJCOPY := objcopy
NASM := nasm

CFLAGS32 := -m32 -ffreestanding -fno-pic -fno-pie -nostdlib -nostdinc -Wall -Wextra -I. -Iinclude -Iinclude/printf -Ikernel -Igraficzny_tryb -fno-stack-protector
CFLAGS64 := -m64 -ffreestanding -fno-pic -fno-pie -nostdlib -nostdinc -Wall -Wextra -Iinclude -Iinclude/printf -Ikernel -Igraficzny_tryb -fno-stack-protector -mcmodel=kernel -mno-red-zone
LDFLAGS32 := -m elf_i386 -T linker.ld
LDFLAGS64 := -T linker64.ld

KERNEL_SOURCES := $(wildcard kernel/*.c) $(wildcard kernel/apps/*.c) $(wildcard kernel/apps/*/*.c) $(wildcard include/printf/*.c) include/blue.c $(wildcard drivers/*.c) $(wildcard graficzny_tryb/*.c) $(wildcard system_plikow/*.c)
KERNEL_OBJECTS32 := $(patsubst %.c,$(BUILD_DIR)/%.o,$(KERNEL_SOURCES))
KERNEL_OBJECTS64 := $(patsubst %.c,$(BUILD_DIR)/%64.o,$(KERNEL_SOURCES))

.PHONY: all image iso clean

all: image

image: $(OS_IMAGE)

iso: $(ISO_IMAGE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/kernel $(BUILD_DIR)/kernel/apps $(BUILD_DIR)/kernel/apps/appdownload $(BUILD_DIR)/drivers $(BUILD_DIR)/include/printf
	mkdir -p $(ISO_DIR)

$(BOOT_BIN32): boot.asm | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

$(UEFI_BIN): uefi64.asm | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC32) $(CFLAGS32) -c $< -o $@

$(BUILD_DIR)/%64.o: %.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	@if command -v x86_64-elf-gcc >/dev/null 2>&1; then \
		x86_64-elf-gcc $(CFLAGS64) -c $< -o $@; \
	else \
		echo "Pomijam x86_64 (brak toolchain, użyj make image)"; \
		touch $@; \
	fi

$(KERNEL_ELF32): $(KERNEL_OBJECTS32) linker.ld | $(BUILD_DIR)
	$(LD32) $(LDFLAGS32) -o $@ $(KERNEL_OBJECTS32)

$(KERNEL_ELF64): $(KERNEL_OBJECTS64) linker64.ld | $(BUILD_DIR)
	@if command -v x86_64-elf-ld >/dev/null 2>&1; then \
		x86_64-elf-ld $(LDFLAGS64) -o $@ $(KERNEL_OBJECTS64); \
	else \
		echo "Pomijam UEFI64 kernel (brak toolchain)"; \
		$(CC64) $(CFLAGS64) -nostdlib -nodefaultlibs -T linker64.ld -o $@ $(KERNEL_OBJECTS64) || touch $@; \
	fi

$(KERNEL_BIN32): $(KERNEL_ELF32)
	$(OBJCOPY) -O binary $< $@

$(OS_IMAGE): $(BOOT_BIN32) $(KERNEL_BIN32) | $(BUILD_DIR)
	dd if=/dev/zero of=$@ bs=$(FLOPPY_SIZE) count=1 status=none
	dd if=$(BOOT_BIN32) of=$@ conv=notrunc status=none
	dd if=$(KERNEL_BIN32) of=$@ bs=512 seek=1 conv=notrunc status=none
	@if [ -f $(UEFI_BIN) ]; then \
		echo "UEFI stub gotowy (img BIOS32)"; \
	fi

$(ISO_IMAGE): $(UEFI_BIN) $(KERNEL_ELF64) | $(ISO_DIR)
	mkdir -p $(ISO_DIR)
	cp -f $(KERNEL_ELF64) $(ISO_DIR)/kernel64.elf || cp -f build/kernel64.elf $(ISO_DIR)/kernel64.elf || touch $(ISO_DIR)/kernel64.elf
	cp $(UEFI_BIN) $(ISO_DIR)/bootx64.efi
	@if command -v xorriso >/dev/null 2>&1; then \
		xorriso -as mkisofs -o $@ \
			-b bootx64.efi \
			-no-emul-boot -boot-load-size 4 -boot-info-table \
			--efi-boot bootx64.efi \
			$(ISO_DIR); \
	else \
		echo "xorriso brak - użyj mkisofs lub genisoimage lub ręcznie"; \
	fi

clean:
	rm -rf $(BUILD_DIR)

