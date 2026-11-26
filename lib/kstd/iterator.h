#ifndef KSTD_ITERATOR_H
#define KSTD_ITERATOR_H

#include <lib/kstd/type_traits.h>

namespace kstd {

struct input_iterator_tag   {};
struct output_iterator_tag  {};
struct forward_iterator_tag   : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

template <typename Iterator>
struct iterator_traits {
    using difference_type   = typename Iterator::difference_type;
    using value_type        = typename Iterator::value_type;
    using pointer           = typename Iterator::pointer;
    using reference         = typename Iterator::reference;
    using iterator_category = typename Iterator::iterator_category;
};

template <typename T>
struct iterator_traits<T*> {
    using difference_type   = ptrdiff_t;
    using value_type        = T;
    using pointer           = T*;
    using reference         = T&;
    using iterator_category = random_access_iterator_tag;
};

template <typename T>
struct iterator_traits<const T*> {
    using difference_type   = ptrdiff_t;
    using value_type        = T;
    using pointer           = const T*;
    using reference         = const T&;
    using iterator_category = random_access_iterator_tag;
};

template <typename Iterator>
class reverse_iterator {
public:
    using iterator_type        = Iterator;
    using iterator_category    = typename iterator_traits<Iterator>::iterator_category;
    using value_type           = typename iterator_traits<Iterator>::value_type;
    using difference_type      = typename iterator_traits<Iterator>::difference_type;
    using pointer              = typename iterator_traits<Iterator>::pointer;
    using reference            = typename iterator_traits<Iterator>::reference;

    KSTD_CONSTEXPR reverse_iterator() : current() {}
    KSTD_CONSTEXPR explicit reverse_iterator(Iterator x) : current(x) {}

    KSTD_CONSTEXPR Iterator base() const { return current; }

    KSTD_CONSTEXPR reference operator*() const {
        Iterator tmp = current;
        --tmp;
        return *tmp;
    }

    KSTD_CONSTEXPR reverse_iterator& operator++() {
        --current;
        return *this;
    }

    KSTD_CONSTEXPR reverse_iterator operator++(int) {
        reverse_iterator tmp = *this;
        --current;
        return tmp;
    }

    KSTD_CONSTEXPR reverse_iterator& operator--() {
        ++current;
        return *this;
    }

    KSTD_CONSTEXPR reverse_iterator operator--(int) {
        reverse_iterator tmp = *this;
        ++current;
        return tmp;
    }

    KSTD_CONSTEXPR reverse_iterator operator+(difference_type n) const {
        return reverse_iterator(current - n);
    }

    KSTD_CONSTEXPR reverse_iterator& operator+=(difference_type n) {
        current -= n;
        return *this;
    }

    KSTD_CONSTEXPR reverse_iterator operator-(difference_type n) const {
        return reverse_iterator(current + n);
    }

    KSTD_CONSTEXPR reverse_iterator& operator-=(difference_type n) {
        current += n;
        return *this;
    }

    KSTD_CONSTEXPR pointer operator->() const {
        return &(operator*());
    }

    KSTD_CONSTEXPR reference operator[](difference_type n) const {
        return *(*this + n);
    }

private:
    Iterator current;
};

template <typename Iterator>
KSTD_CONSTEXPR bool operator==(const reverse_iterator<Iterator>& a,
                               const reverse_iterator<Iterator>& b) {
    return a.base() == b.base();
}
template <typename Iterator>
KSTD_CONSTEXPR bool operator!=(const reverse_iterator<Iterator>& a,
                               const reverse_iterator<Iterator>& b) {
    return a.base() != b.base();
}

}

#endif
