#pragma once

#include "nlrsAliases.h"
#include "nlrsAllocator.h"
#include "nlrsAssert.h"
#include "nlrsBuffer.h"

namespace nlrs
{

template<typename T, usize N = 32u>
class ObjectPool
{
public:
    ObjectPool(IAllocator& allocator);
    ObjectPool(ObjectPool&&);
    ObjectPool& operator=(ObjectPool&&);
    ~ObjectPool() = default;

    ObjectPool() = delete;
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    template<typename... Arg>
    T* create(Arg&&... args);
    void release(T* object);

    usize size() const { return size_; }

private:
    struct Element alignas(8)
    {
        union
        {
            Element* next;
            u8 buffer[sizeof(T)];
        };
    };

    Buffer<Element> buffer_;
    usize size_;
    Element* head_;
};

template<typename T, usize N>
ObjectPool<T, N>::ObjectPool(IAllocator& allocator)
    : buffer_(allocator, N),
    size_(0u),
    head_(nullptr)
{}

template<typename T, usize N>
ObjectPool<T, N>::ObjectPool(ObjectPool&& other)
    : buffer_(std::move(other.buffer_)),
    size_(other.size_),
    head_(other.head_)
{
    other.size_ = 0u;
    other.head_ = nullptr;
}

template<typename T, usize N>
ObjectPool<T, N>& ObjectPool<T, N>::operator=(ObjectPool<T, N>&& rhs)
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
T* ObjectPool<T, N>::create(Args&&... args)
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
void ObjectPool<T, N>::release(T* obj)
{
    NLRS_ASSERT(size_ > 0);
    obj->~T();
    Element* elem = reinterpret_cast<Element*>(obj);
    elem->next = head_;
    head_ = elem;
    --size_;
}

}
