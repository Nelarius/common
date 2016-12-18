#pragma once

#include "nlrsAliases.h"
#include "nlrsAllocator.h"
#include "nlrsAssert.h"

namespace nlrs
{

/*
 * Provides an uninitialized buffer for an array of T.
 * This owns the memory uniquely, which means that it is movable, but not copyable.
 *
 * This class is not type-safe, so you must call the destructor yourself
 * on objects stored there.
 */
template< typename T, size_t alignment = alignof(T) >
class Buffer
{
public:
    Buffer(IAllocator& allocator, usize capacity = 8u);
    Buffer() = delete;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&&);
    Buffer& operator=(Buffer&&);
    ~Buffer();

    T*          at(usize index);
    const T*    at(usize index) const;
    T*          operator[](usize index);
    const T*    operator[](usize index) const;

    // Increase the capacity of the container to at least newSize.
    // If newSize is smaller than the current capacity, then nothing happens
    void        reserve(usize newSize);
    usize       capacity() const;

private:
    IAllocator&     allocator_;
    u8*             buffer_;
    usize           capacity_;
};

template< typename T, size_t alignment >
Buffer<T, alignment>::Buffer(IAllocator& allocator, usize capacity)
    : allocator_(allocator),
    buffer_(nullptr),
    capacity_(0u)
{
    reserve(capacity);
}

template< typename T, size_t alignment >
Buffer<T, alignment>::Buffer(Buffer&& other)
    : allocator_(other.allocator_),
    buffer_(other.buffer_),
    capacity_(other.capacity_)
{
    other.buffer_ = nullptr;
    other.capacity_ = 0u;
}

template< typename T, size_t alignment  >
Buffer<T, alignment>& Buffer<T, alignment>::operator=(Buffer&& rhs)
{
    if (buffer_)
    {
        allocator_.free(buffer_);
    }
    allocator_ = rhs.allocator_;
    buffer_ = rhs.buffer_;
    capacity_ = rhs.capacity_;
    rhs.buffer_ = nullptr;
    rhs.capacity_ = 0u;
    return *this;
}

template< typename T, size_t alignment >
Buffer<T, alignment>::~Buffer()
{
    if (buffer_)
    {
        allocator_.free(buffer_);
    }
}

template< typename T, size_t alignment >
T* Buffer<T, alignment>::at(usize index)
{
    NLRS_ASSERT(index < capacity_);
    return reinterpret_cast<T*>(buffer_) + index;
}

template< typename T, size_t alignment >
const T* Buffer<T, alignment>::at(usize index) const
{
    NLRS_ASSERT(index < capacity_);
    return reinterpret_cast<const T*>(buffer_) + index;
}

template< typename T, size_t alignment >
T* Buffer<T, alignment>::operator[](usize index)
{
    return at(index);
}

template< typename T, size_t alignment >
const T* Buffer<T, alignment>::operator[](usize index) const
{
    return at(index);
}

template< typename T, size_t alignment >
void Buffer<T, alignment>::reserve(usize newSize)
{
    if (newSize == 0u)
    {
        return;
    }
    if (!buffer_)
    {
        buffer_ = (u8*)allocator_.allocate(sizeof(T) * newSize, alignment);
        capacity_ = newSize;
    }
    else if (capacity_ < newSize)
    {
        buffer_ = (u8*)allocator_.reallocate(buffer_, sizeof(T) * newSize);
        capacity_ = newSize;
    }
}

template< typename T, size_t alignment >
usize Buffer<T, alignment >::capacity() const
{
    return capacity_;
}

}
