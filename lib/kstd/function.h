#ifndef KSTD_FUNCTION_H
#define KSTD_FUNCTION_H

#include <lib/kstd/config.h>
#include <lib/kstd/type_traits.h>
#include <lib/kstd/utility.h>

namespace kstd {

namespace detail {

template <typename T>
struct void_t_helper { using type = void; };
template <typename T> using void_t = typename void_t_helper<T>::type;

template <typename R, typename... Args>
struct function_vtable {
    R (*invoke)(void* buf, Args&&... args);
    void (*destroy)(void* buf);
    void (*copy)(void* dst, const void* src);
    void (*move)(void* dst, void* src);
};

template <typename Functor, typename R, typename... Args>
struct function_vtable_impl {
    static R invoke(void* buf, Args&&... args) {
        Functor& f = *reinterpret_cast<Functor*>(buf);
        return f(kstd::forward<Args>(args)...);
    }
    static void destroy(void* buf) {
        reinterpret_cast<Functor*>(buf)->~Functor();
    }
    static void copy(void* dst, const void* src) {
        new (dst) Functor(*reinterpret_cast<const Functor*>(src));
    }
    static void move(void* dst, void* src) {
        new (dst) Functor(kstd::move(*reinterpret_cast<Functor*>(src)));
        reinterpret_cast<Functor*>(src)->~Functor();
    }
};

template <typename R, typename... Args>
struct null_vtable {
    static R invoke(void*, Args&&...) { return R(); }
    static void destroy(void*) {}
    static void copy(void*, const void*) {}
    static void move(void*, void*) {}
};

template <typename T, typename = void>
struct has_call_operator : false_type {};

template <typename T>
struct has_call_operator<T, void_t<decltype(&T::operator())>> : true_type {};

}

template <typename> class function;

template <typename R, typename... Args>
class function<R(Args...)> {
public:
    using result_type = R;

    KSTD_CONSTEXPR function() noexcept :
        _vtable(&detail::null_vtable<R, Args...>::invoke,
                &detail::null_vtable<R, Args...>::destroy,
                &detail::null_vtable<R, Args...>::copy,
                &detail::null_vtable<R, Args...>::move),
        _has_value(false) {}

    KSTD_CONSTEXPR function(decltype(nullptr)) noexcept : function() {}

    template <typename Functor>
    function(Functor&& f) :
        _vtable(&detail::function_vtable_impl<typename remove_reference<Functor>::type, R, Args...>::invoke,
                &detail::function_vtable_impl<typename remove_reference<Functor>::type, R, Args...>::destroy,
                &detail::function_vtable_impl<typename remove_reference<Functor>::type, R, Args...>::copy,
                &detail::function_vtable_impl<typename remove_reference<Functor>::type, R, Args...>::move),
        _has_value(true)
    {
        static_assert(sizeof(Functor) <= KSTD_SFO_BUFFER_SIZE,
                      "kstd::function: callable zbyt duzy dla bufora SFO (zwieksz KSTD_SFO_BUFFER_SIZE)");
        static_assert(is_same<typename remove_cv<typename remove_reference<Functor>::type>::type,
                              function<R(Args...)>>::value == false,
                      "kstd::function: nie mozna skopiować samego siebie");
        new (_buf) typename remove_reference<Functor>::type(kstd::forward<Functor>(f));
    }

    function(const function& other) :
        _vtable(other._vtable), _has_value(other._has_value)
    {
        if (_has_value && _vtable->copy) {
            _vtable->copy(_buf, other._buf);
        }
    }

    function(function&& other) noexcept :
        _vtable(other._vtable), _has_value(other._has_value)
    {
        if (_has_value && _vtable->move) {
            _vtable->move(_buf, other._buf);
            other._has_value = false;
        }
    }

    ~function() {
        if (_has_value && _vtable->destroy) {
            _vtable->destroy(_buf);
        }
    }

    function& operator=(const function& other) {
        if (this != &other) {
            reset();
            _vtable = other._vtable;
            _has_value = other._has_value;
            if (_has_value && _vtable->copy) {
                _vtable->copy(_buf, other._buf);
            }
        }
        return *this;
    }

    function& operator=(function&& other) noexcept {
        if (this != &other) {
            reset();
            _vtable = other._vtable;
            _has_value = other._has_value;
            if (_has_value && _vtable->move) {
                _vtable->move(_buf, other._buf);
                other._has_value = false;
            }
        }
        return *this;
    }

    function& operator=(decltype(nullptr)) noexcept {
        reset();
        return *this;
    }

    R operator()(Args... args) const {
        if (!_has_value || !_vtable->invoke) {
            return R();
        }
        return _vtable->invoke(_buf, kstd::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept {
        return _has_value;
    }

    bool has_value() const noexcept { return _has_value; }

    void reset() noexcept {
        if (_has_value && _vtable->destroy) {
            _vtable->destroy(_buf);
        }
        _has_value = false;
        _vtable = &detail::null_vtable<R, Args...>;
    }

private:
    const detail::function_vtable<R, Args...>* _vtable;
    alignas(16) char _buf[KSTD_SFO_BUFFER_SIZE];
    bool _has_value;
};

}

#endif
