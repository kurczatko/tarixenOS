#ifndef KSTD_UTILITY_H
#define KSTD_UTILITY_H

#include <lib/kstd/type_traits.h>

namespace kstd {

template <typename T>
KSTD_CONSTEXPR typename remove_reference<T>::type&& move(T&& t) noexcept {
    return static_cast<typename remove_reference<T>::type&&>(t);
}

template <typename T>
KSTD_CONSTEXPR T&& forward(typename remove_reference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}

template <typename T>
KSTD_CONSTEXPR T&& forward(typename remove_reference<T>::type&& t) noexcept {
    static_assert(!is_lvalue_reference<T>::value, "forward z lvalue_reference jest zabronione");
    return static_cast<T&&>(t);
}

template <typename T>
KSTD_CONSTEXPR void swap(T& a, T& b) noexcept(
    is_nothrow_move_constructible<T>::value && is_nothrow_move_assignable<T>::value)
{
    T tmp = kstd::move(a);
    a = kstd::move(b);
    b = kstd::move(tmp);
}

template <typename T1, typename T2>
struct pair {
    using first_type  = T1;
    using second_type = T2;

    T1 first;
    T2 second;

    KSTD_CONSTEXPR pair() : first(), second() {}
    KSTD_CONSTEXPR pair(const T1& x, const T2& y) : first(x), second(y) {}
    KSTD_CONSTEXPR pair(T1&& x, T2&& y) : first(kstd::move(x)), second(kstd::move(y)) {}

    template <typename U1, typename U2>
    KSTD_CONSTEXPR pair(U1&& x, U2&& y) : first(kstd::forward<U1>(x)), second(kstd::forward<U2>(y)) {}

    KSTD_CONSTEXPR pair& operator=(const pair& other) {
        first  = other.first;
        second = other.second;
        return *this;
    }
};

template <typename T1, typename T2>
KSTD_CONSTEXPR pair<typename remove_cv<T1>::type, typename remove_cv<T2>::type>
make_pair(T1&& x, T2&& y) {
    return pair<typename remove_cv<T1>::type, typename remove_cv<T2>::type>(
        kstd::forward<T1>(x), kstd::forward<T2>(y));
}

template <typename T1, typename T2>
KSTD_CONSTEXPR bool operator==(const pair<T1,T2>& a, const pair<T1,T2>& b) {
    return a.first == b.first && a.second == b.second;
}
template <typename T1, typename T2>
KSTD_CONSTEXPR bool operator!=(const pair<T1,T2>& a, const pair<T1,T2>& b) {
    return !(a == b);
}

}

#endif
