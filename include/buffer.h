#pragma once

#include "aliases.h"
#include "memory_arena.h"
#include "nlrs_assert.h"

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
class buffer
{
public:
    buffer(memory_arena& allocator, usize capacity = 8u);
    buffer() = delete;
    buffer(const buffer&) = delete;
    buffer& operator=(const buffer&) = delete;
    buffer(buffer&&);
    buffer& operator=(buffer&&);
    ~buffer();

    T*          at(usize index);
    const T*    at(usize index) const;
    T*          operator[](usize index);
    const T*    operator[](usize index) const;

    // Increase the capacity of the container to at least new_size.
    // If new_size is smaller than the current capacity, then nothing happens
    void        reserve(usize newSize);
    usize       capacity() const;

private:
    memory_arena&   allocator_;
    u8*             buffer_;
    usize           capacity_;
};

template<typename T, size_t alignment>
buffer<T, alignment>::buffer(memory_arena& allocator, usize capacity)
    : allocator_(allocator),
    buffer_(nullptr),
    capacity_(0u)
{
    reserve(capacity);
}

template<typename T, size_t alignment>
buffer<T, alignment>::buffer(buffer&& other)
    : allocator_(other.allocator_),
    buffer_(other.buffer_),
    capacity_(other.capacity_)
{
    other.buffer_ = nullptr;
    other.capacity_ = 0u;
}

template<typename T, size_t alignment>
buffer<T, alignment>& buffer<T, alignment>::operator=(buffer&& rhs)
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

template<typename T, size_t alignment>
buffer<T, alignment>::~buffer()
{
    if (buffer_)
    {
        allocator_.free(buffer_);
    }
}

template<typename T, size_t alignment>
T* buffer<T, alignment>::at(usize index)
{
    NLRS_ASSERT(index < capacity_);
    return reinterpret_cast<T*>(buffer_) + index;
}

template<typename T, size_t alignment>
const T* buffer<T, alignment>::at(usize index) const
{
    NLRS_ASSERT(index < capacity_);
    return reinterpret_cast<const T*>(buffer_) + index;
}

template<typename T, size_t alignment>
T* buffer<T, alignment>::operator[](usize index)
{
    return at(index);
}

template<typename T, size_t alignment>
const T* buffer<T, alignment>::operator[](usize index) const
{
    return at(index);
}

template<typename T, size_t alignment>
void buffer<T, alignment>::reserve(usize new_size)
{
    if (new_size == 0u)
    {
        return;
    }
    if (!buffer_)
    {
        buffer_ = (u8*)allocator_.allocate(sizeof(T) * new_size, alignment);
        capacity_ = new_size;
    }
    else if (capacity_ < new_size)
    {
        buffer_ = (u8*)allocator_.reallocate(buffer_, sizeof(T) * new_size);
        capacity_ = new_size;
    }
}

template<typename T, size_t alignment>
usize buffer<T, alignment >::capacity() const
{
    return capacity_;
}

}
