#include <kernel/paging.h>

__attribute__((aligned(4096))) static uint32_t page_directory[1024];
__attribute__((aligned(4096))) static uint32_t page_table_low[1024];
__attribute__((aligned(4096))) static uint32_t page_table_low2[1024];
__attribute__((aligned(4096))) static uint32_t page_table_low3[1024];
__attribute__((aligned(4096))) static uint32_t page_table_fb[1024];

#define FRAME_ALLOC_START 0x00200000
#define FRAME_ALLOC_END   0x01000000
#define FRAME_ALLOC_COUNT ((FRAME_ALLOC_END - FRAME_ALLOC_START) / PAGE_SIZE)
static uint32_t frame_alloc_bits[(FRAME_ALLOC_COUNT + 31) / 32];

static inline int frame_alloc_test(uint32_t i) {
    return (frame_alloc_bits[i >> 5] >> (i & 31)) & 1U;
}
static inline void frame_alloc_set(uint32_t i) {
    frame_alloc_bits[i >> 5] |= (1U << (i & 31));
}
static inline void frame_alloc_clear(uint32_t i) {
    frame_alloc_bits[i >> 5] &= ~(1U << (i & 31));
}

void *paging_alloc_frame(void) {
    uint32_t i = 0;
    while (i < FRAME_ALLOC_COUNT) {
        if (!frame_alloc_test(i)) {
            frame_alloc_set(i);
            return (void *)(FRAME_ALLOC_START + i * PAGE_SIZE);
        }
        i++;
    }
    return 0;
}

void paging_free_frame(void *addr) {
    uint32_t a = (uint32_t)addr;
    if (a < FRAME_ALLOC_START || a >= FRAME_ALLOC_END) return;
    if ((a & (PAGE_SIZE - 1)) != 0) return;
    uint32_t i = (a - FRAME_ALLOC_START) / PAGE_SIZE;
    frame_alloc_clear(i);
}

void map_page(uint32_t *table, uint32_t vaddr, uint32_t paddr, uint32_t flags)
{
    table[(vaddr / PAGE_SIZE) & 0x3FF] = (paddr & ~(PAGE_SIZE - 1)) | flags;
}

void paging_init(void)
{
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
        page_table_low[i] = 0;
        page_table_low2[i] = 0;
        page_table_low3[i] = 0;
        page_table_fb[i] = 0;
    }

    for (uint32_t addr = 0; addr < 4u * 1024u * 1024u; addr += PAGE_SIZE) {
        uint32_t flags = PAGE_PRESENT | PAGE_READWRITE;
        if (addr >= USER_MEM_START && addr < USER_MEM_END) {
            flags |= PAGE_USER;
        }
        map_page(page_table_low, addr, addr, flags);
    }

    for (uint32_t addr = 4u * 1024u * 1024u; addr < 8u * 1024u * 1024u; addr += PAGE_SIZE) {
        map_page(page_table_low2, addr, addr, PAGE_PRESENT | PAGE_READWRITE);
    }

    for (uint32_t addr = 8u * 1024u * 1024u; addr < 12u * 1024u * 1024u; addr += PAGE_SIZE) {
        map_page(page_table_low3, addr, addr, PAGE_PRESENT | PAGE_READWRITE);
    }

    page_directory[0] = ((uint32_t)page_table_low | PAGE_PRESENT | PAGE_READWRITE);
    page_directory[1] = ((uint32_t)page_table_low2 | PAGE_PRESENT | PAGE_READWRITE);
    page_directory[2] = ((uint32_t)page_table_low3 | PAGE_PRESENT | PAGE_READWRITE);

    page_directory[0xFD000000 >> 22] = ((uint32_t)page_table_fb | PAGE_PRESENT | PAGE_READWRITE);
    for (int i = 0; i < 1024; i++) {
        map_page(page_table_fb, 0xFD000000 + i * PAGE_SIZE, 0xFD000000 + i * PAGE_SIZE, PAGE_PRESENT | PAGE_READWRITE);
    }

    asm volatile ("mov %0, %%cr3" :: "r"(page_directory));

    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));
}
