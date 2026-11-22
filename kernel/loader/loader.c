/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 */

#include <fs/system_plikow.h> /*system plikow to takie szambo ze pewnie ladowanie jakiegos pliku to kwestia cudu*/
#include <include/printf/types.h>
#include <kernel/memory/paging.h>
#include <kernel/sys/syscall.h>
#include <kernel/core/userspace.h>
#include <kernel/core/proces.h>

#define ELF_MAGIC 0x464C457F

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7

#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4

#define EM_386    3

#define ELFCLASS32 1
#define ELFDATA2LSB 1
#define EV_CURRENT  1

#define EI_MAG0     0
#define EI_MAG1     1
#define EI_MAG2     2
#define EI_MAG3     3
#define EI_CLASS    4
#define EI_DATA     5
#define EI_VERSION  6

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

static int elf_check_header(struct elf32_ehdr *eh) {
    if (eh->e_ident[EI_MAG0] != 0x7F) return -1;
    if (eh->e_ident[EI_MAG1] != 'E') return -2;
    if (eh->e_ident[EI_MAG2] != 'L') return -3;
    if (eh->e_ident[EI_MAG3] != 'F') return -4;
    if (eh->e_ident[EI_CLASS] != ELFCLASS32) return -5;
    if (eh->e_ident[EI_DATA] != ELFDATA2LSB) return -6;
    if (eh->e_ident[EI_VERSION] != EV_CURRENT) return -7;
    if (eh->e_type != ET_EXEC) return -8;
    if (eh->e_machine != EM_386) return -9;
    if (eh->e_version != EV_CURRENT) return -10;
    return 0;
}

static int elf_segment_overlaps(uint32_t vaddr, uint32_t memsz, uint32_t *loaded, uint32_t loaded_count) {
    uint32_t end = vaddr + memsz;
    for (uint32_t i = 0; i < loaded_count; i++) {
        uint32_t s = loaded[i * 2];
        uint32_t e = loaded[i * 2 + 1];
        if (vaddr < e && s < end) return 1;
    }
    return 0;
}

static uint32_t elf_align_up(uint32_t addr, uint32_t align) {
    if (align == 0 || align == 1) return addr;
    return (addr + align - 1) & ~(align - 1);
}

static void elf_map_segment_pages(uint32_t *page_dir, uint32_t vaddr, uint32_t memsz, uint32_t flags) {
    uint32_t start = vaddr & ~(PAGE_SIZE - 1);
    uint32_t end = elf_align_up(vaddr + memsz, PAGE_SIZE);
    uint32_t page_flags = PAGE_PRESENT | PAGE_USER;
    if (flags & PF_W) page_flags |= PAGE_READWRITE;
    
    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
        uint32_t *page_table = (uint32_t *)(page_dir[(addr >> 22) & 0x3FF] & ~0xFFF);
        if (!page_table) {
            page_table = (uint32_t *)paging_alloc_frame();
            if (!page_table) continue;
            for (int i = 0; i < 1024; i++) page_table[i] = 0;
            page_dir[(addr >> 22) & 0x3FF] = ((uint32_t)page_table | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER);
        }
        uint32_t frame = (uint32_t)paging_alloc_frame();
        if (!frame) continue;
        map_page(page_table, addr, frame, page_flags);
    }
}

int userspace_load_elf_into_dir(uint32_t *page_dir, const char *path, uint32_t *entry_out, uint32_t *stack_out) {
    int size = read_file(path, 0, 0);
    if (size <= 0) return -1;
    
    char *elf_buf = (char *)paging_alloc_frame();
    if (!elf_buf) return -12;
    
    size = read_file(path, elf_buf, size);
    if (size <= 0) {
        paging_free_frame(elf_buf);
        return -1;
    }

    struct elf32_ehdr *eh = (struct elf32_ehdr *)elf_buf;
    int ret = elf_check_header(eh);
    if (ret != 0) {
        paging_free_frame(elf_buf);
        return ret;
    }

    if (eh->e_ehsize < sizeof(struct elf32_ehdr)) {
        paging_free_frame(elf_buf);
        return -11;
    }
    if (eh->e_phentsize < sizeof(struct elf32_phdr)) {
        paging_free_frame(elf_buf);
        return -12;
    }
    if (eh->e_phnum == 0) {
        paging_free_frame(elf_buf);
        return -13;
    }
    if (eh->e_phoff + (uint32_t)eh->e_phnum * (uint32_t)eh->e_phentsize > (uint32_t)size) {
        paging_free_frame(elf_buf);
        return -14;
    }
    if (eh->e_entry < USER_MEM_START || eh->e_entry >= USER_MEM_END) {
        paging_free_frame(elf_buf);
        return -15;
    }

    uint32_t loaded[32 * 2];
    uint32_t loaded_count = 0;
    uint32_t max_addr = 0;
    uint32_t entry_in_segment = 0;

    for (uint16_t i = 0; i < eh->e_phnum; i++) {
        struct elf32_phdr *ph = (struct elf32_phdr *)(elf_buf + eh->e_phoff + i * eh->e_phentsize);
        if (ph->p_type != PT_LOAD) continue;
        if (ph->p_vaddr < USER_MEM_START || ph->p_vaddr >= USER_MEM_END) {
            paging_free_frame(elf_buf);
            return -16;
        }
        if (ph->p_memsz == 0) continue;
        if (ph->p_vaddr + ph->p_memsz > USER_MEM_END) {
            paging_free_frame(elf_buf);
            return -16;
        }
        if (ph->p_offset + ph->p_filesz > (uint32_t)size) {
            paging_free_frame(elf_buf);
            return -17;
        }
        if (loaded_count < 32 && elf_segment_overlaps(ph->p_vaddr, ph->p_memsz, loaded, loaded_count)) {
            paging_free_frame(elf_buf);
            return -18;
        }
        if (loaded_count < 32) {
            loaded[loaded_count * 2] = ph->p_vaddr;
            loaded[loaded_count * 2 + 1] = ph->p_vaddr + ph->p_memsz;
            loaded_count++;
        }

        if (eh->e_entry >= ph->p_vaddr && eh->e_entry < ph->p_vaddr + ph->p_memsz) {
            entry_in_segment = 1;
        }

        uint32_t vaddr_aligned = ph->p_vaddr & ~(PAGE_SIZE - 1);
        uint32_t offset_in_page = ph->p_vaddr - vaddr_aligned;
        
        elf_map_segment_pages(page_dir, ph->p_vaddr, ph->p_memsz, ph->p_flags);

        uint8_t *dst = (uint8_t *)(uint32_t)ph->p_vaddr;
        for (uint32_t j = 0; j < ph->p_filesz; j++) {
            dst[j] = elf_buf[ph->p_offset + j];
        }
        for (uint32_t j = ph->p_filesz; j < ph->p_memsz; j++) {
            dst[j] = 0;
        }

        uint32_t end = ph->p_vaddr + ph->p_memsz;
        if (end > max_addr) max_addr = end;
    }

    if (!entry_in_segment) {
        paging_free_frame(elf_buf);
        return -19;
    }

    max_addr = elf_align_up(max_addr, PAGE_SIZE);
    if (max_addr > 0x003F0000) max_addr = 0x003F0000;

    *entry_out = eh->e_entry;
    *stack_out = 0x003FF000;

    paging_free_frame(elf_buf);
    return 0;
}

int userspace_load_elf(const char *path, uint32_t *entry_out) {
    uint32_t stack;
    return userspace_load_elf_into_dir((uint32_t *)proces_get_kernel_page_dir(), path, entry_out, &stack);
}

int userspace_load_and_run(const char *path) {
    uint32_t entry;
    uint32_t stack;
    int ret = userspace_load_elf_into_dir((uint32_t *)proces_get_kernel_page_dir(), path, &entry, &stack);
    if (ret != 0) return ret;
    userspace_uruchom(entry, stack);
    return 0;
}

int proces_zaladuj_elf(const char *path) {
    struct proces *p = proces_stworz(0, path, 0);
    if (!p) return -1;

    uint32_t entry;
    uint32_t stack;
    int ret = userspace_load_elf_into_dir(p->page_directory, path, &entry, &stack);
    if (ret != 0) {
        proces_zakoncz(p->pid);
        return ret;
    }

    p->entry_point = entry;
    proces_ustaw_stos(p, entry, 0);
    p->user_stack = stack;
    sched_enqueue(p);
    return p->pid;
}
