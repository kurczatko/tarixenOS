#ifndef KSTD_TYPE_TRAITS_H
#define KSTD_TYPE_TRAITS_H

#include <lib/kstd/config.h>
#include <lib/types.h>
#include <include/stdint.h>

#ifndef KSTD_PTRDIFF_T
#define KSTD_PTRDIFF_T
typedef long ptrdiff_t;
#endif

namespace kstd {

template <typename T, T V>
struct integral_constant {
    static constexpr T value = V;
    using value_type = T;
    using type = integral_constant;
    KSTD_CONSTEXPR operator T() const noexcept { return value; }
    KSTD_CONSTEXPR T operator()() const noexcept { return value; }
};

template <bool B> using bool_constant = integral_constant<bool, B>;
using true_type  = bool_constant<true>;
using false_type = bool_constant<false>;

template <typename T> struct is_void                 : false_type {};
template <> struct is_void<void>                   : true_type {};

template <typename T> struct is_null_pointer         : false_type {};
template <> struct is_null_pointer<decltype(nullptr)> : true_type {};

template <typename T> struct is_integral             : false_type {};
template <> struct is_integral<bool>               : true_type {};
template <> struct is_integral<char>               : true_type {};
template <> struct is_integral<signed char>        : true_type {};
template <> struct is_integral<unsigned char>      : true_type {};
template <> struct is_integral<wchar_t>            : true_type {};
template <> struct is_integral<short>              : true_type {};
template <> struct is_integral<unsigned short>     : true_type {};
template <> struct is_integral<int>                : true_type {};
template <> struct is_integral<unsigned int>       : true_type {};
template <> struct is_integral<long>               : true_type {};
template <> struct is_integral<unsigned long>      : true_type {};
template <> struct is_integral<long long>          : true_type {};
template <> struct is_integral<unsigned long long> : true_type {};

template <typename T> struct is_floating_point : false_type {};
template <> struct is_floating_point<float>      : true_type {};
template <> struct is_floating_point<double>     : true_type {};
template <> struct is_floating_point<long double>: true_type {};

template <typename T> struct is_arithmetic :
    bool_constant<is_integral<T>::value || is_floating_point<T>::value> {};

template <typename T> struct is_fundamental :
    bool_constant<is_arithmetic<T>::value || is_void<T>::value || is_null_pointer<T>::value> {};

template <typename T> struct is_pointer            : false_type {};
template <typename T> struct is_pointer<T*>        : true_type {};

template <typename T> struct is_lvalue_reference   : false_type {};
template <typename T> struct is_lvalue_reference<T&> : true_type {};

template <typename T> struct is_rvalue_reference   : false_type {};
template <typename T> struct is_rvalue_reference<T&&> : true_type {};

template <typename T> struct is_reference :
    bool_constant<is_lvalue_reference<T>::value || is_rvalue_reference<T>::value> {};

template <typename T> struct is_class :
    bool_constant<!is_arithmetic<T>::value && !is_fundamental<T>::value &&
                  !is_reference<T>::value && !is_pointer<T>::value && !is_void<T>::value> {};

template <typename T> struct is_nothrow_move_constructible : false_type {};
template <typename T> struct is_nothrow_move_assignable    : false_type {};
template <typename T> struct is_nothrow_default_constructible : false_type {};

template <typename T, typename U> struct is_same : false_type {};
template <typename T> struct is_same<T, T> : true_type {};

template <typename Base, typename Derived> struct is_base_of : false_type {};

template <typename T> struct remove_const             { typedef T type; };
template <typename T> struct remove_const<const T>    { typedef T type; };

template <typename T> struct remove_volatile          { typedef T type; };
template <typename T> struct remove_volatile<volatile T> { typedef T type; };

template <typename T> struct remove_cv {
    typedef typename remove_volatile<typename remove_const<T>::type>::type type;
};

template <typename T> struct remove_reference { typedef T type; };
template <typename T> struct remove_reference<T&>  { typedef T type; };
template <typename T> struct remove_reference<T&&> { typedef T type; };

template <typename T> struct add_lvalue_reference  { typedef T& type; };
template <typename T> struct add_lvalue_reference<T&>  { typedef T& type; };
template <typename T> struct add_lvalue_reference<T&&> { typedef T& type; };

template <typename T> struct add_rvalue_reference  { typedef T&& type; };
template <typename T> struct add_rvalue_reference<T&>  { typedef T& type; };
template <typename T> struct add_rvalue_reference<T&&> { typedef T&& type; };

template <typename T> struct remove_pointer { typedef T type; };
template <typename T> struct remove_pointer<T*> { typedef T type; };

template <typename T, bool = is_reference<T>::value || is_void<T>::value>
struct add_pointer_impl { typedef T type; };

template <typename T>
struct add_pointer_impl<T, false> { typedef typename remove_reference<T>::type * type; };

template <typename T> struct add_pointer {
    typedef typename add_pointer_impl<T>::type type;
};

template <typename...> struct conjunction : true_type {};
template <typename B1, typename... Bn>
struct conjunction<B1, Bn...> :
    bool_constant<B1::value && conjunction<Bn...>::value> {};

template <typename...> struct disjunction : false_type {};
template <typename B1, typename... Bn>
struct disjunction<B1, Bn...> :
    bool_constant<B1::value || disjunction<Bn...>::value> {};

template <typename B> struct negation : bool_constant<!B::value> {};

template <bool B, typename T = void> struct enable_if {};
template <typename T> struct enable_if<true, T> { typedef T type; };

template <bool B, typename T, typename F> struct conditional { typedef T type; };
template <typename T, typename F> struct conditional<false, T, F> { typedef F type; };

template <bool B, typename T = void> using enable_if_t = typename enable_if<B, T>::type;
template <typename T> using remove_const_t    = typename remove_const<T>::type;
template <typename T> using remove_volatile_t = typename remove_volatile<T>::type;
template <typename T> using remove_cv_t       = typename remove_cv<T>::type;
template <typename T> using remove_reference_t = typename remove_reference<T>::type;
template <typename T> using add_lvalue_reference_t  = typename add_lvalue_reference<T>::type;
template <typename T> using add_rvalue_reference_t  = typename add_rvalue_reference<T>::type;
template <typename T> using remove_pointer_t   = typename remove_pointer<T>::type;
template <typename T> using add_pointer_t      = typename add_pointer<T>::type;
template <bool B, typename T, typename F> using conditional_t = typename conditional<B, T, F>::type;

template <typename T>
KSTD_CONSTEXPR typename add_rvalue_reference<T>::type declval() noexcept;

}

#endif
