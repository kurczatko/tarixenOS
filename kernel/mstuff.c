/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <include/tarixen/mstuff.h>
#include <libc/syscall.h>

#define M_BLOCK_OFFSET 0
#define M_HEAD_SIZE m_align_up(sizeof(struct m_block))

struct m_block {
    uint32_t size;
    uint8_t free;
    struct m_block *next;
};

static struct m_block *m_heap_base = 0;
static uint32_t m_heap_end = 0;
static uint8_t m_heap_initialized = 0;

static void m_heap_grow(uint32_t delta)
{
    if (delta == 0) return;
    uint32_t old_end = m_heap_end;
    uint32_t new_end = old_end + delta;
    if (libc_syscall(SYSCALL_BRK, new_end, 0, 0) == 0) return;
    m_heap_end = new_end;
}

static struct m_block *m_find_block(uint32_t size)
{
    struct m_block *curr = m_heap_base;
    while (curr) {
        if (curr->free && curr->size >= size) return curr;
        curr = curr->next;
    }
    return 0;
}

static void m_split_block(struct m_block *block, uint32_t size)
{
    if (block->size - size < M_HEAD_SIZE + 16) return;
    struct m_block *rem = (struct m_block *)((char *)block + M_HEAD_SIZE + size);
    rem->size = block->size - size - M_HEAD_SIZE;
    rem->free = 1;
    rem->next = block->next;
    block->size = size;
    block->free = 0;
    block->next = rem;
}

static struct m_block *m_extend_heap(uint32_t size)
{
    uint32_t total = M_HEAD_SIZE + size;
    m_heap_grow(total);
    if (m_heap_end < total) return 0;
    struct m_block *block = (struct m_block *)((char *)m_heap_base + (m_heap_end - total));
    block->size = size;
    block->free = 0;
    block->next = 0;
    return block;
}

void *m_malloc(uint32_t size)
{
    if (size == 0) size = 1;
    size = m_align_up(size);

    if (!m_heap_initialized) {
        m_heap_initialized = 1;
        if (!m_heap_end) m_heap_end = libc_syscall(SYSCALL_BRK, 0, 0, 0);
        if (!m_heap_end) m_heap_end = 0x01000000;
        m_heap_base = (struct m_block *)m_heap_end;
        m_heap_end += M_HEAD_SIZE;
        m_heap_base->size = 0;
        m_heap_base->free = 1;
        m_heap_base->next = 0;
    }

    struct m_block *block = m_find_block(size);
    if (block) {
        block->free = 0;
        m_split_block(block, size);
        return (void *)((char *)block + M_HEAD_SIZE);
    }

    block = m_extend_heap(size);
    if (!block) return 0;
    return (void *)((char *)block + M_HEAD_SIZE);
}

void m_free(void *ptr)
{
    if (!ptr) return;
    struct m_block *block = (struct m_block *)((char *)ptr - M_HEAD_SIZE);
    block->free = 1;

    struct m_block *curr = m_heap_base;
    while (curr) {
        if (curr->free && curr->next && curr->next->free) {
            curr->size += M_HEAD_SIZE + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

void *m_realloc(void *ptr, uint32_t size)
{
    if (!ptr) return m_malloc(size);
    if (size == 0) { m_free(ptr); return 0; }

    struct m_block *block = (struct m_block *)((char *)ptr - M_HEAD_SIZE);
    if (block->size >= size) {
        m_split_block(block, size);
        return ptr;
    }

    void *new_ptr = m_malloc(size);
    if (!new_ptr) return 0;
    m_memcpy(new_ptr, ptr, block->size);
    m_free(ptr);
    return new_ptr;
}

void *m_calloc(uint32_t count, uint32_t size)
{
    uint32_t total = count * size;
    void *ptr = m_malloc(total);
    if (!ptr) return 0;
    m_memset(ptr, 0, total);
    return ptr;
}
