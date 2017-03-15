#pragma once

#include "aliases.h"
#include "memory_arena.h"
#include "nlrs_assert.h"
#include "buffer.h"

#include <utility>

namespace nlrs
{

template<typename T, usize N = 32u>
class object_pool
{
public:
    object_pool(memory_arena& allocator);
    object_pool(object_pool&&);
    object_pool& operator=(object_pool&&);
    ~object_pool() = default;

    object_pool() = delete;
    object_pool(const object_pool&) = delete;
    object_pool& operator=(const object_pool&) = delete;

    template<typename... Arg>
    T* create(Arg&&... args);
    void release(T* object);

    usize size() const { return size_; }

private:
    struct alignas(8) element
    {
        union
        {
            element* next;
            u8 buffer[sizeof(T)];
        };
    };

    buffer<element> buffer_;
    usize size_;
    element* head_;
};

template<typename T, usize N>
object_pool<T, N>::object_pool(memory_arena& allocator)
    : buffer_(allocator, N),
    size_(0u),
    head_(nullptr)
{}

template<typename T, usize N>
object_pool<T, N>::object_pool(object_pool&& other)
    : buffer_(std::move(other.buffer_)),
    size_(other.size_),
    head_(other.head_)
{
    other.size_ = 0u;
    other.head_ = nullptr;
}

template<typename T, usize N>
object_pool<T, N>& object_pool<T, N>::operator=(object_pool<T, N>&& rhs)
{
    buffer_ = std::move(rhs.buffer_);
    size_ = rhs.size_;
    head_ = rhs.head_;

    rhs.size_ = 0u;
    rhs.head_ = nullptr;

    return *this;
}

template<typename T, usize N>
template<typename... Args>
T* object_pool<T, N>::create(Args&&... args)
{
    if (size_ == buffer_.capacity())
    {
        return nullptr;
    }
    T* obj = nullptr;
    if (head_)
    {
        obj = reinterpret_cast<T*>(&head_->buffer[0]);
        head_ = head_->next;
    }
    else
    {
        obj = reinterpret_cast<T*>(&buffer_.at(size_)->buffer[0]);
    }

    new (obj) T{ std::forward<Args>(args)... };
    ++size_;

    return obj;
}

template<typename T, usize N>
void object_pool<T, N>::release(T* obj)
{
    NLRS_ASSERT(size_ > 0);
    obj->~T();
    element* elem = reinterpret_cast<element*>(obj);
    elem->next = head_;
    head_ = elem;
    --size_;
}

}
