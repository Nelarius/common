#pragma once

#include "aliases.h"
#include "nlrs_assert.h"
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

namespace nlrs
{
namespace detail
{

template<class T>
class reverse_iterator
{
public:
    explicit reverse_iterator(T* ptr)
        : ptr_(ptr)
    {}

    reverse_iterator() = delete;

    bool operator==(const reverse_iterator& rhs) const
    {
        return ptr_ == rhs.ptr_;
    }

    bool operator!=(const reverse_iterator& rhs) const
    {
        return ptr_ != rhs.ptr_;
    }

    reverse_iterator& operator++()
    {
        ptr_ = ptr_ - 1u;
        return *this;
    }

    reverse_iterator operator++(int)
    {
        reverse_iterator was(*this);
        ptr_ = ptr_ - 1u;
        return was;
    }

    T& operator*()
    {
        return *ptr_;
    }

    T* operator->()
    {
        return ptr_;
    }

private:
    T* ptr_;
};

}

// This implements the same functionality as std::array, but you
// can push_back and emplace_back elements into it
template<class T, usize N>
class resizable_array
{
public:
    resizable_array() = default;
    resizable_array(const resizable_array&) = default;
    resizable_array& operator=(const resizable_array&) = default;
    resizable_array(std::initializer_list<T>);
    ~resizable_array();

    resizable_array(resizable_array&&) = delete;
    resizable_array& operator=(resizable_array&&) = delete;

    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = detail::reverse_iterator<T>;
    using const_reverse_iterator = detail::reverse_iterator<const T>;

    T&          at(usize);
    const T&    at(usize) const;
    T&          operator[](usize);
    const T&    operator[](usize) const;
    iterator        begin();
    iterator        end();
    const_iterator   begin() const;
    const_iterator   end()   const;
    reverse_iterator       rbegin();
    reverse_iterator       rend();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend()   const;

    T* data()
    {
        return reinterpret_cast<T*>(&storage_[0]);
    }

    const T* data() const
    {
        return reinterpret_cast<const T*>(&storage_[0]);
    }

    // the element must be copy-constructible
    usize push_back(const T&);
    // the element must be move-constructible
    usize emplace_back(T&&);
    template<class... Args>
    usize emplace_back(Args&&... args);

    std::size_t size() const { return size_; }
    std::size_t maxSize() const { return N; }

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type storage_[N];
    std::size_t size_{ 0u };
};

template<class T, usize N>
resizable_array<T, N>::resizable_array(std::initializer_list<T> list)
    : storage_{ 0 }
{
    NLRS_ASSERT(list.size() <= N);
    for (auto& elem : list)
    {
        push_back(elem);
    }
}

template<class T, usize N>
resizable_array<T, N>::~resizable_array()
{
    for (usize i = 0u; i < size_; ++i)
    {
        at(i).~T();
    }
}

template<class T, usize N>
T& resizable_array<T, N>::at(usize i)
{
    return const_cast<T&>(static_cast<const resizable_array*>(this)->at(i));
}

template<class T, usize N>
const T& resizable_array<T, N>::at(usize i) const
{
    NLRS_ASSERT(i < size_);
    NLRS_ASSERT(size_ <= N);
    return *(data() + i);
}

template<class T, usize N>
T& resizable_array<T, N>::operator[](usize i)
{
    return at(i);
}

template<class T, usize N>
const T& resizable_array<T, N>::operator[](usize i) const
{
    return at(i);
}

template<class T, usize N>
typename resizable_array<T, N>::iterator resizable_array<T, N>::begin()
{
    NLRS_ASSERT(size_ <= N);
    return data();
}

template<class T, usize N>
typename resizable_array<T, N>::iterator resizable_array<T, N>::end()
{
    NLRS_ASSERT(size_ <= N);
    return (T*)(data() + size_);
}

template<class T, usize N>
typename resizable_array<T, N>::const_iterator resizable_array<T, N>::begin() const
{
    NLRS_ASSERT(size_ <= N);
    return data();
}

template<class T, usize N>
typename resizable_array<T, N>::const_iterator resizable_array<T, N>::end() const
{
    NLRS_ASSERT(size_ <= N);
    return (const T*)(data() + size_);
}

template<class T, usize N>
typename resizable_array<T, N>::reverse_iterator resizable_array<T, N>::rbegin()
{
    NLRS_ASSERT(size_ <= N);
    return reverse_iterator(reinterpret_cast<T*>(&storage_[size_ - 1u]));
}

template<class T, usize N>
typename resizable_array<T, N>::reverse_iterator resizable_array<T, N>::rend()
{
    NLRS_ASSERT(size_ <= N);
    return reverse_iterator(reinterpret_cast<T*>(&storage_[0u]) - 1u);
}

template<class T, usize N>
typename resizable_array<T, N>::const_reverse_iterator resizable_array<T, N>::rbegin() const
{
    NLRS_ASSERT(size_ <= N);
    return const_reverse_iterator(reinterpret_cast<const T*>(&storage_[size_ - 1u]));
}

template<class T, usize N>
typename resizable_array<T, N>::const_reverse_iterator resizable_array<T, N>::rend() const
{
    NLRS_ASSERT(size_ <= N);
    return const_reverse_iterator(data() - 1u);
}

template<class T, usize N>
usize resizable_array<T, N>::push_back(const T& elem)
{
    NLRS_ASSERT(size_ < N);
    new (data() + size_) T(elem);
    return size_++;
}

template<class T, usize N>
usize resizable_array<T, N>::emplace_back(T&& elem)
{
    NLRS_ASSERT(size_ < N);
    new (data() + size_) T(elem);
    return size_++;
}

template<class T, usize N>
template<class... Args>
usize resizable_array<T, N>::emplace_back(Args&&... args)
{
    NLRS_ASSERT(size_ < N);
    new (data() + size_) T{ std::forward<Args>(args)... };
    return size_++;
}

}
