#include "../system_plikow/system_plikow.h"
#include "../include/printf/types.h"
#include "paging.h"
#include "syscall.h"
#include "userspace.h"
#include "../include/printf/vga.h"

#define ELF_MAGIC 0x464C457F
#define PT_LOAD 1

struct elf32_ehdr {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

struct elf32_phdr {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
};

static uint8_t elf_buf[65536];

int userspace_load_and_run(const char *path) {
    int size = read_file(path, (char *)elf_buf, sizeof(elf_buf));
    if (size <= 0) return -1;
    struct elf32_ehdr *eh = (struct elf32_ehdr *)elf_buf;
    if (*(uint32_t *)eh->e_ident != ELF_MAGIC) return -2;
    if (eh->e_phnum == 0) return -3;
    uint32_t max_addr = 0;
    for (uint16_t i = 0; i < eh->e_phnum; i++) {
        struct elf32_phdr *ph = (struct elf32_phdr *)(elf_buf + eh->e_phoff + i * eh->e_phentsize);
        if (ph->p_type != PT_LOAD) continue;
        if (ph->p_vaddr < USER_MEM_START || ph->p_vaddr + ph->p_memsz > USER_MEM_END) return -4;
        uint8_t *dst = (uint8_t *)(uint32_t)ph->p_vaddr;
        for (uint32_t j = 0; j < ph->p_filesz; j++) dst[j] = elf_buf[ph->p_offset + j];
        for (uint32_t j = ph->p_filesz; j < ph->p_memsz; j++) dst[j] = 0;
        uint32_t end = ph->p_vaddr + ph->p_memsz;
        if (end > max_addr) max_addr = end;
    }
    max_addr = (max_addr + 4095) & ~4095;
    if (max_addr > 0x003F0000) max_addr = 0x003F0000;
    userspace_break = max_addr;
    uint32_t stack = 0x003FF000;
    userspace_uruchom(eh->e_entry, stack);
    return 0;
}
