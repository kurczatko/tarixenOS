BUILD_DIR := build
ISO_DIR := $(BUILD_DIR)/iso

BOOT_BIN32 := $(BUILD_DIR)/boot.bin
KERNEL_ELF32 := $(BUILD_DIR)/kernel32.elf
KERNEL_BIN32 := $(BUILD_DIR)/kernel32.bin
OS_IMAGE := $(BUILD_DIR)/tarixenOS.img
KERNEL_MAX_SECTORS := 512
FLOPPY_SIZE := 67108864

UEFI_BIN := $(BUILD_DIR)/uefi64.bin
KERNEL_ELF64 := $(BUILD_DIR)/kernel64.elf
ISO_IMAGE := $(BUILD_DIR)/tarixenOS.iso

CC32 := cc
CC64 := cc
CXX := g++
LD32 := ld
LD64 := ld
OBJCOPY := objcopy
NASM := nasm

CFLAGS32 := -m32 -Os -ffreestanding -fno-pic -fno-pie -nostdlib -nostdinc -Wall -Wextra -I. -Iinclude -Iinclude/printf -Ilibc -Ikernel -Igraficzny_tryb -fno-stack-protector -Wno-incompatible-pointer-types
CFLAGS64 := -m64 -ffreestanding -fno-pic -fno-pie -nostdlib -nostdinc -Wall -Wextra -Iinclude -Iinclude/printf -Ikernel -Igraficzny_tryb -fno-stack-protector -mcmodel=kernel -mno-red-zone
CXXFLAGS := -m32 -Os -ffreestanding -fno-exceptions -fno-rtti -nostdlib -nostdinc -Wall -Wextra -I. -Iinclude -Iinclude/printf -Ikernel -Igraficzny_tryb -fno-stack-protector
LDFLAGS32 := -m elf_i386 -T linker.ld
LDFLAGS64 := -T linker64.ld

KERNEL_SOURCES := $(wildcard kernel/*.c) kernel/syscall_asm.S kernel/default_irq.S kernel/wyjatki_entry.S kernel/timer_entry.S $(wildcard kernel/apps/*.c) $(wildcard kernel/apps/*/*.c) $(wildcard kernel/syscals/*.c) $(wildcard include/printf/*.c) include/blue.c $(wildcard drivers/*.c) $(wildcard drivers/*.cpp) $(wildcard drivers/usb/*.c) $(wildcard drivers/usb/*/*.c) $(wildcard drivers/*.S) $(wildcard drivers/usb/core/*.S) $(wildcard drivers/sound/*.c) $(wildcard drivers/sound/*.cpp) $(wildcard graficzny/*.c) $(wildcard graficzny_tryb/*.c) $(wildcard system_plikow/*.c) $(wildcard scripts/*.c) kompilator/cc_os.c kompilator/lexer_os.c kompilator/codegen_os.c kernel/browser_css.cpp kernel/browser_js.cpp user_land/konta.c user_land/tapeta.c
KERNEL_OBJECTS32 := $(patsubst %.c,$(BUILD_DIR)/%.o,$(filter %.c,$(KERNEL_SOURCES))) $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(filter %.cpp,$(KERNEL_SOURCES))) $(patsubst %.S,$(BUILD_DIR)/%.o,$(filter %.S,$(KERNEL_SOURCES)))
KERNEL_OBJECTS64 := $(patsubst %.c,$(BUILD_DIR)/%64.o,$(filter %.c,$(KERNEL_SOURCES))) $(patsubst %.cpp,$(BUILD_DIR)/%64.o,$(filter %.cpp,$(KERNEL_SOURCES))) $(patsubst %.S,$(BUILD_DIR)/%64.o,$(filter %.S,$(KERNEL_SOURCES)))

USER_APPS := NOTATNIK.ELF EKSPLORATOR.ELF BROWSER.ELF KALKULATOR.ELF GRA.ELF KURSOR.ELF
USER_APPS_LOWER := notatnik eksploartor browser kalkulator gra kursor

.PHONY: all image iso clean user_apps run

all: image

image: $(OS_IMAGE)

iso: $(ISO_IMAGE)

user_apps: $(addprefix $(BUILD_DIR)/,$(USER_APPS))

$(BUILD_DIR)/NOTATNIK.ELF: user_land/apps/notatnik_user.c user_land/okno_user.c libc/string.c libc/stdio.c libc/stdlib.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC32) $(CFLAGS32) -c user_land/okno_user.c -o $(BUILD_DIR)/okno_user.o
	$(CC32) $(CFLAGS32) -c libc/string.c -o $(BUILD_DIR)/string.o
	$(CC32) $(CFLAGS32) -c libc/stdio.c -o $(BUILD_DIR)/stdio.o
	$(CC32) $(CFLAGS32) -c libc/stdlib.c -o $(BUILD_DIR)/stdlib.o
	$(CC32) $(CFLAGS32) -c $< -o $(BUILD_DIR)/notatnik_user.o
	$(LD32) -m elf_i386 -nostdlib -Ttext=0x00100000 -e main -o $@ $(BUILD_DIR)/notatnik_user.o $(BUILD_DIR)/okno_user.o $(BUILD_DIR)/string.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/stdlib.o

$(BUILD_DIR)/EKSPLORATOR.ELF: user_land/apps/eksplorator_user.c user_land/okno_user.c libc/string.c libc/stdio.c libc/stdlib.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC32) $(CFLAGS32) -c user_land/okno_user.c -o $(BUILD_DIR)/okno_user.o
	$(CC32) $(CFLAGS32) -c libc/string.c -o $(BUILD_DIR)/string.o
	$(CC32) $(CFLAGS32) -c libc/stdio.c -o $(BUILD_DIR)/stdio.o
	$(CC32) $(CFLAGS32) -c libc/stdlib.c -o $(BUILD_DIR)/stdlib.o
	$(CC32) $(CFLAGS32) -c $< -o $(BUILD_DIR)/eksplorator_user.o
	$(LD32) -m elf_i386 -nostdlib -Ttext=0x00100000 -e main -o $@ $(BUILD_DIR)/eksplorator_user.o $(BUILD_DIR)/okno_user.o $(BUILD_DIR)/string.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/stdlib.o

$(BUILD_DIR)/BROWSER.ELF: user_land/apps/browser_user.c user_land/okno_user.c libc/string.c libc/stdio.c libc/stdlib.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC32) $(CFLAGS32) -c user_land/okno_user.c -o $(BUILD_DIR)/okno_user.o
	$(CC32) $(CFLAGS32) -c libc/string.c -o $(BUILD_DIR)/string.o
	$(CC32) $(CFLAGS32) -c libc/stdio.c -o $(BUILD_DIR)/stdio.o
	$(CC32) $(CFLAGS32) -c libc/stdlib.c -o $(BUILD_DIR)/stdlib.o
	$(CC32) $(CFLAGS32) -c $< -o $(BUILD_DIR)/browser_user.o
	$(LD32) -m elf_i386 -nostdlib -Ttext=0x00100000 -e main -o $@ $(BUILD_DIR)/browser_user.o $(BUILD_DIR)/okno_user.o $(BUILD_DIR)/string.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/stdlib.o

$(BUILD_DIR)/KALKULATOR.ELF: user_land/apps/kalkulator_user.c user_land/okno_user.c libc/string.c libc/stdio.c libc/stdlib.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC32) $(CFLAGS32) -c user_land/okno_user.c -o $(BUILD_DIR)/okno_user.o
	$(CC32) $(CFLAGS32) -c libc/string.c -o $(BUILD_DIR)/string.o
	$(CC32) $(CFLAGS32) -c libc/stdio.c -o $(BUILD_DIR)/stdio.o
	$(CC32) $(CFLAGS32) -c libc/stdlib.c -o $(BUILD_DIR)/stdlib.o
	$(CC32) $(CFLAGS32) -c $< -o $(BUILD_DIR)/kalkulator_user.o
	$(LD32) -m elf_i386 -nostdlib -Ttext=0x00100000 -e main -o $@ $(BUILD_DIR)/kalkulator_user.o $(BUILD_DIR)/okno_user.o $(BUILD_DIR)/string.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/stdlib.o

$(BUILD_DIR)/GRA.ELF: user_land/apps/gra_user.c user_land/okno_user.c libc/string.c libc/stdio.c libc/stdlib.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC32) $(CFLAGS32) -c user_land/okno_user.c -o $(BUILD_DIR)/okno_user.o
	$(CC32) $(CFLAGS32) -c libc/string.c -o $(BUILD_DIR)/string.o
	$(CC32) $(CFLAGS32) -c libc/stdio.c -o $(BUILD_DIR)/stdio.o
	$(CC32) $(CFLAGS32) -c libc/stdlib.c -o $(BUILD_DIR)/stdlib.o
	$(CC32) $(CFLAGS32) -c $< -o $(BUILD_DIR)/gra_user.o
	$(LD32) -m elf_i386 -nostdlib -Ttext=0x00100000 -e main -o $@ $(BUILD_DIR)/gra_user.o $(BUILD_DIR)/okno_user.o $(BUILD_DIR)/string.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/stdlib.o

$(BUILD_DIR)/KURSOR.ELF: user_land/apps/kursor_user.c user_land/okno_user.c libc/string.c libc/stdio.c libc/stdlib.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC32) $(CFLAGS32) -c user_land/okno_user.c -o $(BUILD_DIR)/okno_user.o
	$(CC32) $(CFLAGS32) -c libc/string.c -o $(BUILD_DIR)/string.o
	$(CC32) $(CFLAGS32) -c libc/stdio.c -o $(BUILD_DIR)/stdio.o
	$(CC32) $(CFLAGS32) -c libc/stdlib.c -o $(BUILD_DIR)/stdlib.o
	$(CC32) $(CFLAGS32) -c $< -o $(BUILD_DIR)/kursor_user.o
	$(LD32) -m elf_i386 -nostdlib -Ttext=0x00100000 -e main -o $@ $(BUILD_DIR)/kursor_user.o $(BUILD_DIR)/okno_user.o $(BUILD_DIR)/string.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/stdlib.o

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

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC32) $(CFLAGS32) -c $< -o $@

$(BUILD_DIR)/%64.o: %.S | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC64) $(CFLAGS64) -c $< -o $@ || touch $@

$(BUILD_DIR)/%64.o: %.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	@if command -v x86_64-elf-gcc >/dev/null 2>&1; then \
		x86_64-elf-gcc $(CFLAGS64) -c $< -o $@; \
	else \
		echo "Pomijam x86_64 (brak toolchain, użyj make image)"; \
		touch $@; \
	fi

$(BUILD_DIR)/%64.o: %.cpp | $(BUILD_DIR)
	@if command -v x86_64-elf-g++ >/dev/null 2>&1; then \
		mkdir -p $(dir $@); \
		x86_64-elf-g++ $(CFLAGS64) -fno-exceptions -fno-rtti -c $< -o $@ || touch $@; \
	else \
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
	$(OBJCOPY) --change-addresses=-0x1000 -O binary $< $@

$(OS_IMAGE): $(BOOT_BIN32) $(KERNEL_BIN32) | $(BUILD_DIR)
	dd if=/dev/zero of=$@ bs=$(FLOPPY_SIZE) count=1 status=none
	@if command -v mkfs.fat >/dev/null 2>&1; then \
		mkfs.fat -F 32 -S 512 -s 1 -R 32 -n TARIXENOS $@ >/dev/null; \
	else \
		echo "Brak mkfs.fat - obraz nie zostanie sformatowany jako FAT32"; exit 1; \
	fi
	dd if=$(BOOT_BIN32) of=$@ bs=1 count=3 conv=notrunc status=none
	dd if=$(BOOT_BIN32) of=$@ bs=1 skip=90 seek=90 count=420 conv=notrunc status=none
	@kernel_size=$$(stat -c %s $(KERNEL_BIN32)); \
	if [ $$kernel_size -gt $$((512 * 512)) ]; then \
		echo "Kernel jest za duzy: $$kernel_size bajtow"; exit 1; \
	fi
	dd if=$(KERNEL_BIN32) of=$@ bs=512 seek=4096 conv=notrunc status=none
	@for app in $(USER_APPS); do \
		if [ -f $(BUILD_DIR)/$$app ]; then \
			mcopy -i $@ $(BUILD_DIR)/$$app ::/$$app; \
		fi; \
	done
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
		echo "nie ma xorriso sam se rob"; \
	fi

clean:

run: image
	qemu-system-x86_64 -m 2G -boot c -drive file=$(OS_IMAGE),format=raw -machine pc 
