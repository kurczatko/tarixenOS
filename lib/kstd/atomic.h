#ifndef KSTD_ATOMIC_H
#define KSTD_ATOMIC_H

#include <lib/kstd/type_traits.h>

namespace kstd {

enum memory_order {
    memory_order_relaxed   = __ATOMIC_RELAXED,
    memory_order_consume   = __ATOMIC_CONSUME,
    memory_order_acquire   = __ATOMIC_ACQUIRE,
    memory_order_release   = __ATOMIC_RELEASE,
    memory_order_acq_rel   = __ATOMIC_ACQ_REL,
    memory_order_seq_cst   = __ATOMIC_SEQ_CST
};

KSTD_STATIC_INLINE void atomic_thread_fence(memory_order order = memory_order_seq_cst) noexcept {
    __atomic_thread_fence(order);
}

KSTD_STATIC_INLINE void atomic_signal_fence(memory_order order = memory_order_seq_cst) noexcept {
    __atomic_signal_fence(order);
}

KSTD_STATIC_INLINE uint8_t atomic_load_8(const volatile uint8_t* obj, memory_order order = memory_order_seq_cst) noexcept {
    return __atomic_load_n(obj, order);
}
KSTD_STATIC_INLINE void atomic_store_8(volatile uint8_t* obj, uint8_t val,
                                       memory_order order = memory_order_seq_cst) noexcept {
    __atomic_store_n(obj, val, order);
}

KSTD_STATIC_INLINE uint16_t atomic_load_16(const volatile uint16_t* obj, memory_order order = memory_order_seq_cst) noexcept {
    return __atomic_load_n(obj, order);
}
KSTD_STATIC_INLINE void atomic_store_16(volatile uint16_t* obj, uint16_t val,
                                        memory_order order = memory_order_seq_cst) noexcept {
    __atomic_store_n(obj, val, order);
}

KSTD_STATIC_INLINE uint32_t atomic_load_32(const volatile uint32_t* obj, memory_order order = memory_order_seq_cst) noexcept {
    return __atomic_load_n(obj, order);
}
KSTD_STATIC_INLINE void atomic_store_32(volatile uint32_t* obj, uint32_t val,
                                        memory_order order = memory_order_seq_cst) noexcept {
    __atomic_store_n(obj, val, order);
}

KSTD_STATIC_INLINE uint64_t atomic_load_64(const volatile uint64_t* obj, memory_order order = memory_order_seq_cst) noexcept {
    return __atomic_load_n(obj, order);
}
KSTD_STATIC_INLINE void atomic_store_64(volatile uint64_t* obj, uint64_t val,
                                        memory_order order = memory_order_seq_cst) noexcept {
    __atomic_store_n(obj, val, order);
}

KSTD_STATIC_INLINE uint32_t atomic_fetch_add_32(volatile uint32_t* obj, uint32_t arg,
                                                memory_order order = memory_order_seq_cst) noexcept {
    return __atomic_fetch_add(obj, arg, order);
}
KSTD_STATIC_INLINE uint32_t atomic_fetch_sub_32(volatile uint32_t* obj, uint32_t arg,
                                                memory_order order = memory_order_seq_cst) noexcept {
    return __atomic_fetch_sub(obj, arg, order);
}

KSTD_STATIC_INLINE uint64_t atomic_fetch_add_64(volatile uint64_t* obj, uint64_t arg,
                                                memory_order order = memory_order_seq_cst) noexcept {
    return __atomic_fetch_add(obj, arg, order);
}
KSTD_STATIC_INLINE uint64_t atomic_fetch_sub_64(volatile uint64_t* obj, uint64_t arg,
                                                memory_order order = memory_order_seq_cst) noexcept {
    return __atomic_fetch_sub(obj, arg, order);
}

KSTD_STATIC_INLINE bool atomic_compare_exchange_32(volatile uint32_t* obj,
                                                    uint32_t* expected, uint32_t desired,
                                                    memory_order success_order = memory_order_seq_cst,
                                                    memory_order failure_order = memory_order_seq_cst) noexcept {
    return __atomic_compare_exchange_n(obj, expected, desired, 0,
                                       success_order,
                                       failure_order);
}

KSTD_STATIC_INLINE bool atomic_compare_exchange_64(volatile uint64_t* obj,
                                                    uint64_t* expected, uint64_t desired,
                                                    memory_order success_order = memory_order_seq_cst,
                                                    memory_order failure_order = memory_order_seq_cst) noexcept {
    return __atomic_compare_exchange_n(obj, expected, desired, 0,
                                       success_order,
                                       failure_order);
}

template <typename T>
struct atomic {
    static_assert(is_integral<T>::value || is_pointer<T>::value,
                  "kstd::atomic obsługuje tylko typy całkowite i wskaźniki");

    KSTD_CONSTEXPR atomic() noexcept : _v(0) {}
    KSTD_CONSTEXPR atomic(T desired) noexcept : _v(desired) {}

    atomic(const atomic&) = delete;
    atomic& operator=(const atomic&) = delete;

    KSTD_CONSTEXPR T load(memory_order order = memory_order_seq_cst) const noexcept {
        return static_cast<T>(__atomic_load_n(reinterpret_cast<const volatile uint32_t*>(&_v),
                                              order));
    }

    KSTD_CONSTEXPR void store(T desired, memory_order order = memory_order_seq_cst) noexcept {
        __atomic_store_n(reinterpret_cast<volatile uint32_t*>(&_v),
                         static_cast<uint32_t>(desired),
                         order);
    }

    KSTD_CONSTEXPR T operator=(T desired) noexcept {
        store(desired);
        return desired;
    }

    KSTD_CONSTEXPR T exchange(T desired,
                              memory_order order = memory_order_seq_cst) noexcept {
        return static_cast<T>(__atomic_exchange_n(reinterpret_cast<volatile uint32_t*>(&_v),
                                                  static_cast<uint32_t>(desired),
                                                  order));
    }

    KSTD_CONSTEXPR bool compare_exchange_weak(T& expected, T desired,
        memory_order success = memory_order_seq_cst,
        memory_order failure = memory_order_seq_cst) noexcept {
        uint32_t exp = static_cast<uint32_t>(expected);
        bool ok = __atomic_compare_exchange_n(reinterpret_cast<volatile uint32_t*>(&_v),
                                              &exp, static_cast<uint32_t>(desired),
                                              0,
                                              success,
                                              failure);
        expected = static_cast<T>(exp);
        return ok;
    }

    KSTD_CONSTEXPR bool compare_exchange_strong(T& expected, T desired,
        memory_order success = memory_order_seq_cst,
        memory_order failure = memory_order_seq_cst) noexcept {
        return compare_exchange_weak(expected, desired, success, failure);
    }

    KSTD_CONSTEXPR T fetch_add(T arg, memory_order order = memory_order_seq_cst) noexcept {
        return static_cast<T>(__atomic_fetch_add(reinterpret_cast<volatile uint32_t*>(&_v),
                                                 static_cast<uint32_t>(arg),
                                                 order));
    }

    KSTD_CONSTEXPR T fetch_sub(T arg, memory_order order = memory_order_seq_cst) noexcept {
        return static_cast<T>(__atomic_fetch_sub(reinterpret_cast<volatile uint32_t*>(&_v),
                                                 static_cast<uint32_t>(arg),
                                                 order));
    }

    KSTD_CONSTEXPR T fetch_or(T arg, memory_order order = memory_order_seq_cst) noexcept {
        return static_cast<T>(__atomic_fetch_or(reinterpret_cast<volatile uint32_t*>(&_v),
                                                static_cast<uint32_t>(arg),
                                                order));
    }

    KSTD_CONSTEXPR T fetch_and(T arg, memory_order order = memory_order_seq_cst) noexcept {
        return static_cast<T>(__atomic_fetch_and(reinterpret_cast<volatile uint32_t*>(&_v),
                                                 static_cast<uint32_t>(arg),
                                                 order));
    }

    KSTD_CONSTEXPR T fetch_xor(T arg, memory_order order = memory_order_seq_cst) noexcept {
        return static_cast<T>(__atomic_fetch_xor(reinterpret_cast<volatile uint32_t*>(&_v),
                                                 static_cast<uint32_t>(arg),
                                                 order));
    }

    KSTD_CONSTEXPR T operator++() noexcept { return fetch_add(1) + 1; }
    KSTD_CONSTEXPR T operator++(int) noexcept { return fetch_add(1); }
    KSTD_CONSTEXPR T operator--() noexcept { return fetch_sub(1) - 1; }
    KSTD_CONSTEXPR T operator--(int) noexcept { return fetch_sub(1); }

    KSTD_CONSTEXPR T operator+=(T arg) noexcept { return fetch_add(arg) + arg; }
    KSTD_CONSTEXPR T operator-=(T arg) noexcept { return fetch_sub(arg) - arg; }

private:
    volatile uint32_t _v;
};

template <>
struct atomic<uint64_t> {
    KSTD_CONSTEXPR atomic() noexcept : _v(0) {}
    KSTD_CONSTEXPR atomic(uint64_t desired) noexcept : _v(desired) {}
    atomic(const atomic&) = delete;
    atomic& operator=(const atomic&) = delete;

    KSTD_CONSTEXPR uint64_t load(memory_order order = memory_order_seq_cst) const noexcept {
        return __atomic_load_n(reinterpret_cast<const volatile uint64_t*>(&_v),
                               order);
    }
    KSTD_CONSTEXPR void store(uint64_t desired, memory_order order = memory_order_seq_cst) noexcept {
        __atomic_store_n(reinterpret_cast<volatile uint64_t*>(&_v), desired,
                         order);
    }
    KSTD_CONSTEXPR uint64_t operator=(uint64_t desired) noexcept { store(desired); return desired; }
    KSTD_CONSTEXPR uint64_t exchange(uint64_t desired,
                                     memory_order order = memory_order_seq_cst) noexcept {
        return __atomic_exchange_n(reinterpret_cast<volatile uint64_t*>(&_v), desired,
                                   order);
    }
    KSTD_CONSTEXPR bool compare_exchange_weak(uint64_t& expected, uint64_t desired,
        memory_order success = memory_order_seq_cst,
        memory_order failure = memory_order_seq_cst) noexcept {
        return __atomic_compare_exchange_n(reinterpret_cast<volatile uint64_t*>(&_v),
                                           &expected, desired,
                                           0,
                                           success,
                                           failure);
    }
    KSTD_CONSTEXPR bool compare_exchange_strong(uint64_t& expected, uint64_t desired,
        memory_order success = memory_order_seq_cst,
        memory_order failure = memory_order_seq_cst) noexcept {
        return compare_exchange_weak(expected, desired, success, failure);
    }
    KSTD_CONSTEXPR uint64_t fetch_add(uint64_t arg,
                                      memory_order order = memory_order_seq_cst) noexcept {
        return __atomic_fetch_add(reinterpret_cast<volatile uint64_t*>(&_v), arg,
                                  order);
    }
    KSTD_CONSTEXPR uint64_t fetch_sub(uint64_t arg,
                                      memory_order order = memory_order_seq_cst) noexcept {
        return __atomic_fetch_sub(reinterpret_cast<volatile uint64_t*>(&_v), arg,
                                  order);
    }
    KSTD_CONSTEXPR uint64_t fetch_or(uint64_t arg,
                                     memory_order order = memory_order_seq_cst) noexcept {
        return __atomic_fetch_or(reinterpret_cast<volatile uint64_t*>(&_v), arg,
                                 order);
    }
    KSTD_CONSTEXPR uint64_t fetch_and(uint64_t arg,
                                      memory_order order = memory_order_seq_cst) noexcept {
        return __atomic_fetch_and(reinterpret_cast<volatile uint64_t*>(&_v), arg,
                                  order);
    }
    KSTD_CONSTEXPR uint64_t fetch_xor(uint64_t arg,
                                      memory_order order = memory_order_seq_cst) noexcept {
        return __atomic_fetch_xor(reinterpret_cast<volatile uint64_t*>(&_v), arg,
                                  order);
    }

private:
    volatile uint64_t _v;
};

template <typename T>
struct atomic<T*> {
    KSTD_CONSTEXPR atomic() noexcept : _v(nullptr) {}
    KSTD_CONSTEXPR atomic(T* desired) noexcept : _v(desired) {}
    atomic(const atomic&) = delete;
    atomic& operator=(const atomic&) = delete;

    KSTD_CONSTEXPR T* load(memory_order order = memory_order_seq_cst) const noexcept {
        return reinterpret_cast<T*>(__atomic_load_n(
            reinterpret_cast<const volatile uint32_t*>(&_v),
            order));
    }
    KSTD_CONSTEXPR void store(T* desired, memory_order order = memory_order_seq_cst) noexcept {
        __atomic_store_n(reinterpret_cast<volatile uint32_t*>(&_v),
                         reinterpret_cast<uint32_t>(desired),
                         order);
    }
    KSTD_CONSTEXPR T* operator=(T* desired) noexcept { store(desired); return desired; }
    KSTD_CONSTEXPR T* exchange(T* desired, memory_order order = memory_order_seq_cst) noexcept {
        return reinterpret_cast<T*>(__atomic_exchange_n(
            reinterpret_cast<volatile uint32_t*>(&_v), reinterpret_cast<uint32_t>(desired),
            order));
    }
    KSTD_CONSTEXPR bool compare_exchange_weak(T*& expected, T* desired,
        memory_order success = memory_order_seq_cst,
        memory_order failure = memory_order_seq_cst) noexcept {
        uint32_t exp = reinterpret_cast<uint32_t>(expected);
        bool ok = __atomic_compare_exchange_n(
            reinterpret_cast<volatile uint32_t*>(&_v), &exp, reinterpret_cast<uint32_t>(desired),
            0,
            success, failure);
        expected = reinterpret_cast<T*>(exp);
        return ok;
    }
    KSTD_CONSTEXPR bool compare_exchange_strong(T*& expected, T* desired,
        memory_order success = memory_order_seq_cst,
        memory_order failure = memory_order_seq_cst) noexcept {
        return compare_exchange_weak(expected, desired, success, failure);
    }
    KSTD_CONSTEXPR T* fetch_add(ptrdiff_t arg,
                                memory_order order = memory_order_seq_cst) noexcept {
        return reinterpret_cast<T*>(__atomic_fetch_add(
            reinterpret_cast<volatile uint32_t*>(&_v),
            static_cast<uint32_t>(arg * sizeof(T)),
            order));
    }
    KSTD_CONSTEXPR T* fetch_sub(ptrdiff_t arg,
                                memory_order order = memory_order_seq_cst) noexcept {
        return reinterpret_cast<T*>(__atomic_fetch_sub(
            reinterpret_cast<volatile uint32_t*>(&_v),
            static_cast<uint32_t>(arg * sizeof(T)),
            order));
    }

private:
    volatile uint32_t _v;
};

}

#endif
