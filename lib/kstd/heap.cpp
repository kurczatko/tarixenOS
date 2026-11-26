#include <lib/kstd/memory.h>
#include <lib/types.h>
#include <include/stdint.h>

namespace kstd {

namespace detail {

constexpr size_t HEAP_BLOCK_ALIGN = 8U;
constexpr size_t HEAP_DEFAULT_SIZE = 65536U;

struct heap_block {
    size_t   size;
    bool     free;
    heap_block* next;
};

static inline size_t align_up(size_t v, size_t a) {
    return (v + a - 1U) & ~(a - 1U);
}

static uint8_t  g_heap_memory[HEAP_DEFAULT_SIZE];
static size_t   g_heap_size = 0U;
static size_t   g_heap_used = 0U;
static heap_block* g_heap_first = nullptr;
static heap_block* g_heap_free_list = nullptr;

static volatile uint32_t g_heap_lock = 0U;

static inline void heap_lock(void) {
    uint32_t expected = 0U;
    while (!__atomic_compare_exchange_n(&g_heap_lock, &expected, 1U,
                                        0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
        expected = 0U;
        __builtin_ia32_pause();
    }
}

static inline void heap_unlock(void) {
    __atomic_store_n(&g_heap_lock, 0U, __ATOMIC_RELEASE);
}

void heap_init_internal(void* base, size_t size) {
    heap_lock();
    g_heap_size   = size;
    g_heap_used   = 0U;
    g_heap_first  = reinterpret_cast<heap_block*>(base);
    g_heap_free_list = nullptr;

    heap_block* first = g_heap_first;
    first->size   = size - sizeof(heap_block);
    first->free   = true;
    first->next   = nullptr;

    heap_unlock();
}

}

using namespace detail;

extern "C" void kstd_heap_init(void* base, size_t size) {
    detail::heap_init_internal(base, size);
}

extern "C" void* kstd_malloc(size_t size) {
    if (size == 0) size = 1U;
    size = detail::align_up(size, detail::HEAP_BLOCK_ALIGN);

    detail::heap_lock();

    heap_block* prev = nullptr;
    heap_block* cur = detail::g_heap_free_list;
    while (cur) {
        if (cur->size >= size) {
            size_t leftover = cur->size - size;
            if (leftover > sizeof(heap_block) + detail::HEAP_BLOCK_ALIGN) {
                heap_block* remainder = reinterpret_cast<heap_block*>(
                    reinterpret_cast<uint8_t*>(cur) + sizeof(heap_block) + size);
                remainder->size   = leftover - sizeof(heap_block);
                remainder->free   = true;
                remainder->next   = cur->next;

                if (prev) prev->next = cur->next;
                else      detail::g_heap_free_list = cur->next;

                cur->size = size;
                cur->free = false;
                remainder->next = detail::g_heap_free_list;
                detail::g_heap_free_list = remainder;
            } else {
                if (prev) prev->next = cur->next;
                else      detail::g_heap_free_list = cur->next;
                cur->free = false;
            }
            detail::g_heap_used += size + sizeof(heap_block);
            detail::heap_unlock();
            return reinterpret_cast<uint8_t*>(cur) + sizeof(heap_block);
        }
        prev = cur;
        cur = cur->next;
    }

    detail::heap_unlock();
    return nullptr;
}

extern "C" void kstd_free(void* ptr) {
    if (!ptr) return;

    heap_block* block = reinterpret_cast<heap_block*>(
        reinterpret_cast<uint8_t*>(ptr) - sizeof(heap_block));

    detail::heap_lock();

    block->free = true;
    detail::g_heap_used -= block->size + sizeof(heap_block);

    block->next = detail::g_heap_free_list;
    detail::g_heap_free_list = block;

    detail::heap_unlock();
}

extern "C" size_t kstd_malloc_usable_size(void* ptr) {
    if (!ptr) return 0U;
    heap_block* block = reinterpret_cast<heap_block*>(
        reinterpret_cast<uint8_t*>(ptr) - sizeof(heap_block));
    return block->size;
}

inline void default_heap_init() {
    kstd_heap_init(detail::g_heap_memory, sizeof(detail::g_heap_memory));
}

}
