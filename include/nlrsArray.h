#pragma once

#include "nlrsAliases.h"
#include "nlrsAllocator.h"
#include "nlrsAssert.h"
#include "nlrsBuffer.h"
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

namespace nlrs
{

template<typename T>
class ReverseIterator
{
public:
    explicit ReverseIterator(T* ptr)
        : ptr_(ptr)
    {}

    ReverseIterator() = delete;

    bool operator==(const ReverseIterator& rhs) const
    {
        return ptr_ == rhs.ptr_;
    }

    bool operator!=(const ReverseIterator& rhs) const
    {
        return ptr_ != rhs.ptr_;
    }

    ReverseIterator& operator++()
    {
        ptr_ = ptr_ - 1u;
        return *this;
    }

    ReverseIterator operator++(int)
    {
        ReverseIterator was(*this);
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

/*
 * An array whose elements are stored in contiguous blocks in memory.
 * Implements a subset of std::vector features.
 */
template<typename T>
class Array
{
public:
    explicit Array(IAllocator& allocator, usize capacity = 8u);
    explicit Array(IAllocator& allocator, std::initializer_list<T> list);
    Array() = delete;
    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;
    Array(Array&&);
    Array& operator=(Array&&);
    ~Array();

    using Iterator = T*;
    using ConstIterator = const T*;
    using RIterator = ReverseIterator<T>;
    using ConstRIterator = ReverseIterator<const T>;

    ConstIterator   begin() const;
    Iterator        begin();
    ConstIterator   end() const;
    Iterator        end();
    RIterator       rbegin();
    RIterator       rend();
    ConstRIterator  rbegin() const;
    ConstRIterator  rend() const;
    T&              at(usize);
    const T&        at(usize) const;
    T&              operator[](usize);
    const T&        operator[](usize) const;
    T&              front();
    const T&        front() const;
    T&              back();
    const T&        back() const;
    T*              data();
    const T*        data() const;

    // T must be copy-constructible
    usize pushBack(const T&);

    // T must be move-constructible
    usize pushBack(T&&);

    template<typename... Args>
    usize emplaceBack(Args&&... args);

    void popBack();
    // Resize the array to contain newSize elements. If the current size is less than
    // newSize, then the array will contain the first newSize elements.
    void resize(usize newSize);
    // Remove an element at index by swapping it with the last element and calling popBack().
    // In order to be removable, the elements need to be copy-constructible
    void swapAndRemove(usize);
    void swapAndRemove(Iterator);
    void swapAndRemove(RIterator);
    void clear();

    usize size() const { return size_; }
    usize capacity() const { return storage_.capacity(); }
    bool  empty() const { return size_ == 0u; }

private:
    void ensureCapacity_();

    Buffer<T>   storage_;
    usize       size_;
};

template<typename T>
Array<T>::Array(IAllocator& allocator, usize capacity)
    : storage_(allocator, capacity),
    size_{ 0u }
{}

template<typename T>
Array<T>::Array(IAllocator& allocator, std::initializer_list<T> list)
    : storage_(allocator),
    size_(0u)
{
    for (auto& elem : list)
    {
        pushBack(elem);
    }
}

template<typename T>
Array<T>::~Array()
{
    if (storage_.capacity())
    {
        clear();
    }
}

template<typename T>
Array<T>::Array(Array<T>&& other)
    : storage_(std::move(other.storage_)),
    size_(other.size_)
{
    other.size_ = 0u;
}

template<typename T>
Array<T>& Array<T>::operator=(Array<T>&& rhs)
{
    clear();
    storage_ = std::move(rhs.storage_);
    size_ = rhs.size_;
    rhs.size_ = 0u;
    return *this;
}

template<typename T>
T& Array<T>::at(usize i)
{
    NLRS_ASSERT(i < size_);
    return ((T*)storage_.at(0))[i];
}

template<typename T>
const T& Array<T>::at(usize i) const
{
    NLRS_ASSERT(i < size_);
    return ((const T*)storage_.at(0))[i];
}

template<typename T>
T& Array<T>::operator[](usize i)
{
    return at(i);
}

template<typename T>
const T& Array<T>::operator[](usize i) const
{
    return at(i);
}

template<typename T>
T& Array<T>::front()
{
    NLRS_ASSERT(size_ != 0u);
    return at(0u);
}

template<typename T>
const T& Array<T>::front() const
{
    NLRS_ASSERT(size_ != 0u);
    return at(0u);
}

template<typename T>
T& Array<T>::back()
{
    NLRS_ASSERT(size_ != 0u);
    return at(size_ - 1u);
}

template<typename T>
const T& Array<T>::back() const
{
    NLRS_ASSERT(size_ != 0u);
    return at(size_ - 1u);
}

template<typename T>
typename Array<T>::Iterator Array<T>::begin()
{
    NLRS_ASSERT(storage_.capacity() != 0u);
    NLRS_ASSERT((T*)(storage_.at(0)) != nullptr);
    return (T*)(storage_.at(0));
}

template<typename T>
typename Array<T>::Iterator Array<T>::end()
{
    NLRS_ASSERT(storage_.capacity() != 0u);
    NLRS_ASSERT((T*)(storage_.at(0)) != nullptr);
    return (T*)(storage_.at(0)) + size_;
}

template<typename T>
typename Array<T>::ConstIterator Array<T>::begin() const
{
    NLRS_ASSERT(storage_.capacity() != 0u);
    NLRS_ASSERT((const T*)(storage_.at(0)) != nullptr);
    return (const T*)(storage_.at(0));
}

template<typename T>
typename Array<T>::ConstIterator Array<T>::end() const
{
    NLRS_ASSERT(storage_.capacity() != 0u);
    NLRS_ASSERT((const T*)(storage_.at(0)) != nullptr);
    return (const T*)(storage_.at(0)) + size_;
}

template<typename T>
typename Array<T>::RIterator Array<T>::rbegin()
{
    NLRS_ASSERT(storage_.capacity() != 0u);
    if (size_ == 0u)
    {
        return rend();
    }
    return RIterator(storage_.at(size_ - 1u));
}

template<typename T>
typename Array<T>::RIterator Array<T>::rend()
{
    NLRS_ASSERT(storage_.capacity() != 0u);
    return RIterator(storage_.at(0u) - 1u);
}

template<typename T>
typename Array<T>::ConstRIterator Array<T>::rbegin() const
{
    NLRS_ASSERT(storage_.capacity() != 0u);
    if (size_ == 0u)
    {
        return rend();
    }
    return ConstRIterator(storage_.at(size_ - 1u));
}

template<typename T>
typename Array<T>::ConstRIterator Array<T>::rend() const
{
    NLRS_ASSERT(storage_.capacity() != 0u);
    return ConstRIterator(storage_.at(0u) - 1u);
}

template<typename T>
T* Array<T>::data()
{
    return const_cast<T*>(static_cast<const Array<T>&>(*this).data());
}

template<typename T>
const T* Array<T>::data() const
{
    NLRS_ASSERT(storage_.capacity() != 0u);
    return storage_.at(0);
}

template<typename T>
void Array<T>::ensureCapacity_()
{
    if (storage_.capacity() == 0u)
    {
        storage_.reserve(8u);
    }
    else if (size_ == storage_.capacity())
    {
        storage_.reserve(storage_.capacity() * 2u);
    }
}

template<typename T>
usize Array<T>::pushBack(const T& elem)
{
    ensureCapacity_();
    new (storage_.at(size_)) T(elem);
    return size_++;
}

template<typename T>
usize Array<T>::pushBack(T&& elem)
{
    ensureCapacity_();
    new (storage_.at(size_)) T(std::move(elem));
    return size_++;
}

template<typename T>
template<typename... Args>
usize Array<T>::emplaceBack(Args&&... args)
{
    ensureCapacity_();
    new (storage_.at(size_)) T{ std::forward<Args>(args)... };
    return size_++;
}

template<typename T>
void Array<T>::popBack()
{
    if (size_ == 0u)
    {
        return;
    }
    storage_.at(--size_)->~T();
}

template<typename T>
void Array<T>::resize(usize newSize)
{
    if (newSize < size_)
    {
        usize i = size_;
        while (i-- > newSize)
        {
            popBack();
        }
    }
    storage_.reserve(newSize);
    size_ = newSize;
}

template<typename T>
void Array<T>::swapAndRemove(usize index)
{
    NLRS_ASSERT(index < size_);
    std::swap(*storage_.at(index), *storage_.at(size_ - 1u));
    popBack();
}

template<typename T>
void Array<T>::swapAndRemove(typename Array<T>::Iterator it)
{
    NLRS_ASSERT(it < end());
    NLRS_ASSERT(it >= begin());
    std::swap(*it, *storage_.at(size_ - 1u));
    popBack();
}

template<typename T>
void Array<T>::swapAndRemove(typename Array<T>::RIterator it)
{
    // TODO: the range should be changed to include the last element
    NLRS_ASSERT(it != rend());
    NLRS_ASSERT(it != rbegin());
    std::swap(*it, *storage_.at(size_ - 1u));
    popBack();
}

template<typename T>
void Array<T>::clear()
{
    for (ReverseIterator<T> iter = rbegin(); iter != rend(); ++iter)
    {
        (*iter).~T();
    }
    size_ = 0u;
}

template<typename T, usize N>
class StaticArray
{
public:
    StaticArray() = default;
    StaticArray(const StaticArray&) = default;
    StaticArray& operator=(const StaticArray&) = default;
    StaticArray(std::initializer_list<T>);
    ~StaticArray();

    StaticArray(StaticArray&&) = delete;
    StaticArray& operator=(StaticArray&&) = delete;

    using Iterator = T*;
    using ConstIterator = const T*;
    using RIterator = ReverseIterator<T>;
    using ConstRIterator = ReverseIterator<const T>;

    T&          at(usize);
    const T&    at(usize) const;
    T&          operator[](usize);
    const T&    operator[](usize) const;
    Iterator        begin();
    Iterator        end();
    ConstIterator   begin() const;
    ConstIterator   end()   const;
    RIterator       rbegin();
    RIterator       rend();
    ConstRIterator  rbegin() const;
    ConstRIterator  rend()   const;

    T* ptr()
    {
        return reinterpret_cast<T*>(&storage_[0]);
    }

    const T* ptr() const
    {
        return reinterpret_cast<const T*>(&storage_[0]);
    }

    // the element must be copy-constructible
    usize pushBack(const T&);
    // the element must be move-constructible
    usize emplaceBack(T&&);
    template<typename... Args>
    usize emplaceBack(Args&&... args);

    std::size_t size() const { return size_; }
    std::size_t maxSize() const { return N; }

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type storage_[N];
    std::size_t size_{ 0u };
};

template<typename T, usize N>
StaticArray<T, N>::StaticArray(std::initializer_list<T> list)
    : storage_{ 0 }
{
    NLRS_ASSERT(list.size() <= N);
    for (auto& elem : list)
    {
        pushBack(elem);
    }
}

template<typename T, usize N>
StaticArray<T, N>::~StaticArray()
{
    for (usize i = 0u; i < size_; ++i)
    {
        at(i).~T();
    }
}

template<typename T, usize N>
T& StaticArray<T, N>::at(usize i)
{
    return const_cast<T&>(static_cast<const StaticArray*>(this)->at(i));
}

template<typename T, usize N>
const T& StaticArray<T, N>::at(usize i) const
{
    NLRS_ASSERT(i < size_);
    NLRS_ASSERT(size_ <= N);
    return *(ptr() + i);
}

template<typename T, usize N>
T& StaticArray<T, N>::operator[](usize i)
{
    return at(i);
}

template<typename T, usize N>
const T& StaticArray<T, N>::operator[](usize i) const
{
    return at(i);
}

template<typename T, usize N>
typename StaticArray<T, N>::Iterator StaticArray<T, N>::begin()
{
    NLRS_ASSERT(size_ <= N);
    return ptr();
}

template<typename T, usize N>
typename StaticArray<T, N>::Iterator StaticArray<T, N>::end()
{
    NLRS_ASSERT(size_ <= N);
    return (T*)(ptr() + size_);
}

template<typename T, usize N>
typename StaticArray<T, N>::ConstIterator StaticArray<T, N>::begin() const
{
    NLRS_ASSERT(size_ <= N);
    return ptr();
}

template<typename T, usize N>
typename StaticArray<T, N>::ConstIterator StaticArray<T, N>::end() const
{
    NLRS_ASSERT(size_ <= N);
    return (const T*)(ptr() + size_);
}

template<typename T, usize N>
typename StaticArray<T, N>::RIterator StaticArray<T, N>::rbegin()
{
    NLRS_ASSERT(size_ <= N);
    return RIterator(reinterpret_cast<T*>(&storage_[size_ - 1u]));
}

template<typename T, usize N>
typename StaticArray<T, N>::RIterator StaticArray<T, N>::rend()
{
    NLRS_ASSERT(size_ <= N);
    return RIterator(reinterpret_cast<T*>(&storage_[0u]) - 1u);
}

template<typename T, usize N>
typename StaticArray<T, N>::ConstRIterator StaticArray<T, N>::rbegin() const
{
    NLRS_ASSERT(size_ <= N);
    return ConstRIterator(reinterpret_cast<const T*>(&storage_[size_ - 1u]));
}

template<typename T, usize N>
typename StaticArray<T, N>::ConstRIterator StaticArray<T, N>::rend() const
{
    NLRS_ASSERT(size_ <= N);
    return ConstRIterator(ptr() - 1u);
}

template<typename T, usize N>
usize StaticArray<T, N>::pushBack(const T& elem)
{
    NLRS_ASSERT(size_ < N);
    new (ptr() + size_) T(elem);
    return size_++;
}

template<typename T, usize N>
usize StaticArray<T, N>::emplaceBack(T&& elem)
{
    NLRS_ASSERT(size_ < N);
    new (ptr() + size_) T(elem);
    return size_++;
}

template<typename T, usize N>
template<typename... Args>
usize StaticArray<T, N>::emplaceBack(Args&&... args)
{
    NLRS_ASSERT(size_ < N);
    new (ptr() + size_) T{ std::forward<Args>(args)... };
    return size_++;
}

}
