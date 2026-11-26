#ifndef KSTD_ALGORITHM_H
#define KSTD_ALGORITHM_H

#include <lib/kstd/type_traits.h>
#include <lib/kstd/iterator.h>
#include <lib/kstd/utility.h>

namespace kstd {

template <typename InputIt, typename OutputIt>
KSTD_CONSTEXPR OutputIt copy(InputIt first, InputIt last, OutputIt d_first) {
    while (first != last) {
        *d_first = *first;
        ++first;
        ++d_first;
    }
    return d_first;
}

template <typename BidirIt1, typename BidirIt2>
KSTD_CONSTEXPR BidirIt2 copy_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) {
    while (first != last) {
        *--d_last = *--last;
    }
    return d_last;
}

template <typename ForwardIt, typename T>
KSTD_CONSTEXPR void fill(ForwardIt first, ForwardIt last, const T& value) {
    while (first != last) {
        *first = value;
        ++first;
    }
}

template <typename InputIt, typename T>
KSTD_CONSTEXPR InputIt find(InputIt first, InputIt last, const T& value) {
    while (first != last) {
        if (*first == value) return first;
        ++first;
    }
    return last;
}

template <typename InputIt1, typename InputIt2>
KSTD_CONSTEXPR bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2) {
    while (first1 != last1) {
        if (!(*first1 == *first2)) return false;
        ++first1;
        ++first2;
    }
    return true;
}

template <typename InputIt1, typename InputIt2>
KSTD_CONSTEXPR bool lexicographical_compare(InputIt1 first1, InputIt1 last1,
                                            InputIt2 first2, InputIt2 last2) {
    while (first1 != last1) {
        if (first2 == last2 || *first2 < *first1) return false;
        if (*first1 < *first2) return true;
        ++first1;
        ++first2;
    }
    return first2 != last2;
}

template <typename T>
KSTD_CONSTEXPR const T& min(const T& a, const T& b) {
    return b < a ? b : a;
}

template <typename T>
KSTD_CONSTEXPR const T& max(const T& a, const T& b) {
    return a < b ? b : a;
}

template <typename ForwardIt1, typename ForwardIt2>
KSTD_CONSTEXPR ForwardIt2 swap_ranges(ForwardIt1 first1, ForwardIt1 last1,
                                      ForwardIt2 first2) {
    while (first1 != last1) {
        kstd::swap(*first1, *first2);
        ++first1;
        ++first2;
    }
    return first2;
}

template <typename ForwardIt>
KSTD_CONSTEXPR ForwardIt rotate(ForwardIt first, ForwardIt middle, ForwardIt last) {
    if (first == middle) return last;
    if (middle == last) return first;
    ForwardIt next = middle;
    while (first != next) {
        kstd::swap(*first, *next);
        ++first;
        ++next;
        if (next == last) next = middle;
        else if (first == middle) middle = next;
    }
    return first;
}

template <typename InputIt, typename T>
KSTD_CONSTEXPR size_t count(InputIt first, InputIt last, const T& value) {
    size_t n = 0;
    while (first != last) {
        if (*first == value) ++n;
        ++first;
    }
    return n;
}

}

#endif
