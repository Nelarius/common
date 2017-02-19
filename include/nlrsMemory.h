#pragma once

#include "nlrsAllocator.h"

#include <utility>

namespace nlrs
{

template<typename T>
class ScopedPtr
{
public:
    ScopedPtr(IAllocator& allocator, T* ptr = nullptr)
        : allocator_(allocator),
        ptr_(ptr)
    {}

    ScopedPtr(const ScopedPtr<T>&) = delete;
    ScopedPtr<T>& operator=(ScopedPtr<T>&) = delete;

    ScopedPtr(ScopedPtr<T>&& other)
        : allocator_(other.allocator_),
        ptr_(other.ptr_)
    {
        other.ptr_ = nullptr;
    }

    ScopedPtr<T>& operator=(ScopedPtr<T>&& rhs)
    {
        allocator_ = rhs.allocator_;
        ptr_ = rhs.ptr_;
        rhs.ptr_ = nullptr;

        return *this;
    }

    ~ScopedPtr()
    {
        if (ptr_)
        {
            ptr_->~T();
            allocator_.free(ptr_);
            ptr_ = nullptr;
        }
    }

    operator bool() const
    {
        return ptr_ != nullptr;
    }

    T* operator->()
    {
        return ptr_;
    }

    const T* operator->() const
    {
        return ptr_;
    }

    T& operator*()
    {
        return *ptr_;
    }

    const T& operator*() const
    {
        return *ptr_;
    }

    T* get()
    {
        return ptr_;
    }

    const T* get() const
    {
        return ptr_;
    }

private:
    IAllocator& allocator_;
    T* ptr_;
};

template<typename T, typename... Args>
ScopedPtr<T> makeScopedPtr(IAllocator& allocator, Args&&... args)
{
    void* mem = allocator.allocate(sizeof(T), alignof(T));
    T* obj = new (mem) T{ std::forward<Args>(args)... };

    return ScopedPtr<T>(allocator, obj);
}

template<typename Base, typename Derived, typename... Args>
ScopedPtr<Base> makeScopedPtrForBaseClass(IAllocator& allocator, Args&&... args)
{
    void* mem = allocator.allocate(sizeof(Derived), alignof(Derived));
    Derived* obj = new (mem) Derived{ std::forward<Args>(args)... };

    return ScopedPtr<Base>(allocator, dynamic_cast<Base*>(obj));
}

}
