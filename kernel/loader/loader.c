/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <fs/system_plikow.h>
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

#ifndef USER_STACK_TOP
#define USER_STACK_TOP 0x003FF000
#endif

#ifndef USER_HEAP_MAX
#define USER_HEAP_MAX  0x003F0000
#endif

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

static uint32_t elf_align_up(uint32_t addr, uint32_t align) {
    if (align == 0 || align == 1) return addr;
    return (addr + align - 1) & ~(align - 1);
}

static uint32_t elf_align_down(uint32_t addr, uint32_t align) {
    if (align == 0 || align == 1) return addr;
    return addr & ~(align - 1);
}

static int elf_check_header(const struct elf32_ehdr *eh) {
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

static int elf_segment_overlaps(uint32_t vaddr, uint32_t memsz, const uint32_t *loaded, uint32_t loaded_count) {
    uint32_t end = vaddr + memsz;
    if (end < vaddr) return 1; /* overflow */
    for (uint32_t i = 0; i < loaded_count; i++) {
        uint32_t s = loaded[i * 2];
        uint32_t e = loaded[i * 2 + 1];
        if (vaddr < e && s < end) return 1;
    }
    return 0;
}

/* Alokuje bufor na caly plik ELF (wielostronicowo, z obsluga bledu braku pamieci) */
static uint8_t *elf_alloc_file_buffer(uint32_t size, uint32_t *out_pages) {
    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    if (pages == 0) pages = 1;
    *out_pages = pages;

    /* Jesli plik miesci sie w 1 stronie, uzyj pojedynczej ramki */
    if (pages == 1) {
        uint8_t *buf = (uint8_t *)paging_alloc_frame();
        return buf;
    }

    /* Dla wiekszych plikow alokujemy strony i mapujemy w przestrzeni jadra */
    uint32_t *kdir = (uint32_t *)proces_get_kernel_page_dir();
    /* Tymczasowy zakres wirtualny poza typowym niskim zakresem user/kernel */
    uint32_t base_vaddr = 0x01000000;

    for (uint32_t p = 0; p < pages; p++) {
        uint32_t frame = (uint32_t)paging_alloc_frame();
        if (!frame) {
            for (uint32_t k = 0; k < p; k++) {
                paging_free_frame((void *)(base_vaddr + k * PAGE_SIZE));
            }
            return 0;
        }
        uint32_t vaddr = base_vaddr + p * PAGE_SIZE;
        uint32_t pde_idx = (vaddr >> 22) & 0x3FF;
        uint32_t *pt = (uint32_t *)(kdir[pde_idx] & ~0xFFF);
        if (!pt) {
            pt = (uint32_t *)paging_alloc_frame();
            if (!pt) {
                paging_free_frame((void *)frame);
                for (uint32_t k = 0; k < p; k++) {
                    paging_free_frame((void *)(base_vaddr + k * PAGE_SIZE));
                }
                return 0;
            }
            for (int i = 0; i < 1024; i++) pt[i] = 0;
            kdir[pde_idx] = ((uint32_t)pt & ~0xFFF) | PAGE_PRESENT | PAGE_READWRITE;
        }
        map_page(pt, vaddr, frame, PAGE_PRESENT | PAGE_READWRITE);
    }

    return (uint8_t *)base_vaddr;
}

static void elf_free_file_buffer(uint8_t *buf, uint32_t pages) {
    if (!buf) return;
    if (pages == 1) {
        paging_free_frame(buf);
        return;
    }
    for (uint32_t p = 0; p < pages; p++) {
        paging_free_frame(buf + p * PAGE_SIZE);
    }
}

/* Zwraca fizyczny/ramkowy adres zmapowany dla vaddr w danym page_dir lub 0 przy bledzie */
static uint32_t elf_get_mapped_frame(uint32_t *page_dir, uint32_t vaddr) {
    uint32_t pde_idx = (vaddr >> 22) & 0x3FF;
    uint32_t pte_idx = (vaddr >> 12) & 0x3FF;
    uint32_t pde = page_dir[pde_idx];
    if (!(pde & PAGE_PRESENT)) return 0;
    uint32_t *pt = (uint32_t *)(pde & ~0xFFF);
    uint32_t pte = pt[pte_idx];
    if (!(pte & PAGE_PRESENT)) return 0;
    return pte & ~0xFFF;
}

/* Mapuje strony segmentu w page_dir i zwraca 0 przy sukcesie, -1 przy braku pamieci */
static int elf_map_segment_pages(uint32_t *page_dir, uint32_t vaddr, uint32_t memsz, uint32_t flags) {
    uint32_t start = elf_align_down(vaddr, PAGE_SIZE);
    uint32_t end = elf_align_up(vaddr + memsz, PAGE_SIZE);
    if (end < start) return -1;

    uint32_t page_flags = PAGE_PRESENT | PAGE_USER;
    if (flags & PF_W) page_flags |= PAGE_READWRITE;

    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
        uint32_t pde_idx = (addr >> 22) & 0x3FF;
        uint32_t *page_table = (uint32_t *)(page_dir[pde_idx] & ~0xFFF);
        if (!(page_dir[pde_idx] & PAGE_PRESENT)) {
            page_table = (uint32_t *)paging_alloc_frame();
            if (!page_table) return -1;
            for (int i = 0; i < 1024; i++) page_table[i] = 0;
            page_dir[pde_idx] = ((uint32_t)page_table & ~0xFFF) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;
        }

        uint32_t existing = elf_get_mapped_frame(page_dir, addr);
        if (!existing) {
            uint32_t frame = (uint32_t)paging_alloc_frame();
            if (!frame) return -1;
            map_page(page_table, addr, frame, page_flags);
        }
    }
    return 0;
}

/* Kopiuje dane segmentu oraz zeruje czesc BSS po stronie page_dir bez polegania na biezacym CR3 */
static int elf_copy_segment_to_dir(
    uint32_t *target_dir,
    const uint8_t *elf_buf,
    uint32_t file_offset,
    uint32_t vaddr,
    uint32_t filesz,
    uint32_t memsz
) {
    uint32_t *kdir = (uint32_t *)proces_get_kernel_page_dir();
    uint32_t scratch_vaddr = 0x01800000;
    uint32_t scratch_pde_idx = (scratch_vaddr >> 22) & 0x3FF;
    uint32_t *scratch_pt = (uint32_t *)(kdir[scratch_pde_idx] & ~0xFFF);

    if (!scratch_pt) {
        scratch_pt = (uint32_t *)paging_alloc_frame();
        if (!scratch_pt) return -1;
        for (int i = 0; i < 1024; i++) scratch_pt[i] = 0;
        kdir[scratch_pde_idx] = ((uint32_t)scratch_pt & ~0xFFF) | PAGE_PRESENT | PAGE_READWRITE;
    }

    for (uint32_t offset = 0; offset < memsz; ) {
        uint32_t cur_vaddr = vaddr + offset;
        uint32_t page_vaddr = elf_align_down(cur_vaddr, PAGE_SIZE);
        uint32_t page_off = cur_vaddr - page_vaddr;
        uint32_t chunk = PAGE_SIZE - page_off;
        if (chunk > memsz - offset) {
            chunk = memsz - offset;
        }

        uint32_t frame = elf_get_mapped_frame(target_dir, page_vaddr);
        if (!frame) return -1;

        map_page(scratch_pt, scratch_vaddr, frame, PAGE_PRESENT | PAGE_READWRITE);
        uint8_t *dst_page = (uint8_t *)scratch_vaddr;

        for (uint32_t b = 0; b < chunk; b++) {
            uint32_t seg_pos = offset + b;
            if (seg_pos < filesz) {
                dst_page[page_off + b] = elf_buf[file_offset + seg_pos];
            } else {
                dst_page[page_off + b] = 0;
            }
        }

        offset += chunk;
    }

    return 0;
}

int userspace_load_elf_into_dir(uint32_t *page_dir, const char *path, uint32_t *entry_out, uint32_t *stack_out) {
    if (!page_dir || !path || !entry_out || !stack_out) return -20;

    int size = read_file(path, 0, 0);
    if (size <= 0 || (uint32_t)size < sizeof(struct elf32_ehdr)) return -1;

    uint32_t buf_pages = 0;
    uint8_t *elf_buf = elf_alloc_file_buffer((uint32_t)size, &buf_pages);
    if (!elf_buf) return -12;

    int read_bytes = read_file(path, (char *)elf_buf, size);
    if (read_bytes <= 0 || (uint32_t)read_bytes < sizeof(struct elf32_ehdr)) {
        elf_free_file_buffer(elf_buf, buf_pages);
        return -1;
    }
    uint32_t file_size = (uint32_t)read_bytes;

    struct elf32_ehdr *eh = (struct elf32_ehdr *)elf_buf;
    int ret = elf_check_header(eh);
    if (ret != 0) {
        elf_free_file_buffer(elf_buf, buf_pages);
        return ret;
    }

    if (eh->e_ehsize < sizeof(struct elf32_ehdr)) {
        elf_free_file_buffer(elf_buf, buf_pages);
        return -11;
    }
    if (eh->e_phentsize < sizeof(struct elf32_phdr)) {
        elf_free_file_buffer(elf_buf, buf_pages);
        return -12;
    }
    if (eh->e_phnum == 0) {
        elf_free_file_buffer(elf_buf, buf_pages);
        return -13;
    }
    if ((uint64_t)eh->e_phoff + (uint64_t)eh->e_phnum * (uint64_t)eh->e_phentsize > (uint64_t)file_size) {
        elf_free_file_buffer(elf_buf, buf_pages);
        return -14;
    }
    if (eh->e_entry < USER_MEM_START || eh->e_entry >= USER_MEM_END) {
        elf_free_file_buffer(elf_buf, buf_pages);
        return -15;
    }

    uint32_t loaded[32 * 2];
    uint32_t loaded_count = 0;
    uint32_t max_addr = 0;
    uint32_t entry_in_segment = 0;

    for (uint16_t i = 0; i < eh->e_phnum; i++) {
        uint32_t ph_off = eh->e_phoff + (uint32_t)i * (uint32_t)eh->e_phentsize;
        struct elf32_phdr *ph = (struct elf32_phdr *)(elf_buf + ph_off);

        if (ph->p_type != PT_LOAD) continue;
        if (ph->p_memsz == 0) continue;

        if (ph->p_vaddr < USER_MEM_START || ph->p_vaddr >= USER_MEM_END) {
            elf_free_file_buffer(elf_buf, buf_pages);
            return -16;
        }
        if (ph->p_vaddr + ph->p_memsz < ph->p_vaddr || ph->p_vaddr + ph->p_memsz > USER_MEM_END) {
            elf_free_file_buffer(elf_buf, buf_pages);
            return -16;
        }
        if ((uint64_t)ph->p_offset + (uint64_t)ph->p_filesz > (uint64_t)file_size) {
            elf_free_file_buffer(elf_buf, buf_pages);
            return -17;
        }
        if (ph->p_filesz > ph->p_memsz) {
            elf_free_file_buffer(elf_buf, buf_pages);
            return -17;
        }

        if (loaded_count < 32 && elf_segment_overlaps(ph->p_vaddr, ph->p_memsz, loaded, loaded_count)) {
            elf_free_file_buffer(elf_buf, buf_pages);
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

        if (elf_map_segment_pages(page_dir, ph->p_vaddr, ph->p_memsz, ph->p_flags) != 0) {
            elf_free_file_buffer(elf_buf, buf_pages);
            return -12;
        }

        if (elf_copy_segment_to_dir(page_dir, elf_buf, ph->p_offset, ph->p_vaddr, ph->p_filesz, ph->p_memsz) != 0) {
            elf_free_file_buffer(elf_buf, buf_pages);
            return -12;
        }

        uint32_t end = ph->p_vaddr + ph->p_memsz;
        if (end > max_addr) max_addr = end;
    }

    if (!entry_in_segment) {
        elf_free_file_buffer(elf_buf, buf_pages);
        return -19;
    }

    max_addr = elf_align_up(max_addr, PAGE_SIZE);
    if (max_addr > USER_HEAP_MAX) max_addr = USER_HEAP_MAX;

    *entry_out = eh->e_entry;
    *stack_out = USER_STACK_TOP;

    elf_free_file_buffer(elf_buf, buf_pages);
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
    p->user_stack = stack;
    proces_ustaw_stos(p, entry, stack);
    sched_enqueue(p);
    return p->pid;
}