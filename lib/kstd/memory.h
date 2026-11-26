#ifndef KSTD_MEMORY_H
#define KSTD_MEMORY_H

#include <lib/kstd/type_traits.h>
#include <lib/kstd/utility.h>
#include <lib/kstd/atomic.h>

namespace kstd {

extern "C" void kstd_heap_init(void* base, size_t size);
extern "C" void* kstd_malloc(size_t size);
extern "C" void  kstd_free(void* ptr);
extern "C" size_t kstd_malloc_usable_size(void* ptr);

template <typename T>
struct allocator {
    using value_type = T;
    using pointer = T*;
    using size_type = size_t;

    allocator() noexcept {}
    template <typename U> allocator(const allocator<U>&) noexcept {}

    pointer allocate(size_type n) {
        if (n == 0) return nullptr;
        return static_cast<pointer>(kstd_malloc(n * sizeof(T)));
    }

    void deallocate(pointer p, size_type) noexcept {
        kstd_free(static_cast<void*>(p));
    }

    template <typename... Args>
    void construct(pointer p, Args&&... args) {
        ::new (static_cast<void*>(p)) T(kstd::forward<Args>(args)...);
    }

    void destroy(pointer p) noexcept {
        p->~T();
    }
};

template <typename T>
struct default_delete {
    void operator()(T* p) const noexcept {
        kstd_free(static_cast<void*>(p));
    }
};

template <typename T, typename Deleter = default_delete<T>>
class unique_ptr {
public:
    using pointer = T*;
    using deleter_type = Deleter;

    KSTD_CONSTEXPR unique_ptr() noexcept : _ptr(nullptr) {}
    KSTD_CONSTEXPR unique_ptr(decltype(nullptr)) noexcept : _ptr(nullptr) {}
    explicit KSTD_CONSTEXPR unique_ptr(pointer p) noexcept : _ptr(p) {}

    unique_ptr(unique_ptr&& other) noexcept : _ptr(other.release()) {}
    unique_ptr& operator=(unique_ptr&& other) noexcept {
        reset(other.release());
        return *this;
    }

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    ~unique_ptr() { reset(); }

    pointer release() noexcept {
        pointer p = _ptr;
        _ptr = nullptr;
        return p;
    }

    void reset(pointer p = nullptr) noexcept {
        if (_ptr) {
            Deleter{}(_ptr);
        }
        _ptr = p;
    }

    pointer get() const noexcept { return _ptr; }

    T& operator*() const noexcept { return *_ptr; }
    T* operator->() const noexcept { return _ptr; }

    explicit operator bool() const noexcept { return _ptr != nullptr; }

    friend bool operator==(const unique_ptr& a, decltype(nullptr)) noexcept {
        return a.get() == nullptr;
    }
    friend bool operator==(decltype(nullptr), const unique_ptr& b) noexcept {
        return b.get() == nullptr;
    }
    friend bool operator!=(const unique_ptr& a, decltype(nullptr)) noexcept {
        return a.get() != nullptr;
    }
    friend bool operator!=(decltype(nullptr), const unique_ptr& b) noexcept {
        return b.get() != nullptr;
    }

private:
    pointer _ptr;
};

template <typename T, typename... Args>
KSTD_CONSTEXPR unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(static_cast<T*>(kstd_malloc(sizeof(T))));
}

}

#endif
