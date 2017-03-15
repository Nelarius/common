#pragma once

#include "aliases.h"
#include "locator.h"

#include <scoped_allocator>
#include <type_traits>

namespace nlrs
{

class memory_arena
{
public:

    // Allocate a block of memory. Returns nullptr if the number of bytes
    // is zero.
    //
    // The memory alignment must be a power of 2!
    virtual void* allocate(usize bytes, u8 alignment = 8u) = 0;

    // This function must be called on already allocated memory.
    //
    // This function will return the original pointer if the new size fits in the old
    // memory block, or a new pointer if the memory was moved to a new location.
    //
    // The contents of the returned memory block will be the same up to the lesser
    // of the two sizes.
    virtual void* reallocate(void* ptr, usize newSize) = 0;

    //Free a block of memory previously allocated here. If the pointer is null,
    //then nothing happens.
    virtual void free(void* ptr) = 0;
};

// A wrapper around malloc, realloc, and free.
class system_arena : public memory_arena
{
public:
    ~system_arena();
    system_arena(const system_arena&) = delete;
    system_arena& operator=(const system_arena&) = delete;
    system_arena(system_arena&&) = delete;
    system_arena& operator=(system_arena&&) = delete;

    static system_arena& get_instance();

    void* allocate(usize bytes, u8 alignment = 8u)  override;
    void* reallocate(void* ptr, usize newSize)      override;
    void  free(void* ptr)                           override;

private:
    system_arena() = default;

    int alloc_count_{ 0 };
};

// This allocator manages the memory within a memory arena by mainting a linked list
// of freed memory blocks. As memory is freed, it merges adjacent free blocks into
// larger free blocks. Memory is allocated from the free list on a first-fit policy.
//
// The actual block size allocated is never exactly the same as the requested size.
// The allocated block size is rounded up to the next power of 2. The largest expected
// user of this allocator is the array, which uses realloc on resize. If the block is
// already large enough to hold the new size, then nothing need be done.
//
// Ideally, larger, power-of-two blocks would reduce memory fragmentation.
//
// This allocator writes certain magic numbers into memory for debugging purposes.
// Memory blocks are surrounded by the 0xBEEFCAFE magic number.
// When NARWHAL_DEBUG is defined, allocated but uninitialized memory is set to 0xA5
// When NARWHAL_DEBUG is defined, freed memory is set to 0xEE.
class free_list_arena : public memory_arena
{
public:
    free_list_arena(void* memory, usize num_bytes);
    free_list_arena() = delete;
    free_list_arena(const free_list_arena&) = delete;
    free_list_arena(free_list_arena&&) = delete;
    free_list_arena& operator=(const free_list_arena&) = delete;
    free_list_arena& operator=(free_list_arena&&) = delete;
    ~free_list_arena();

    void* allocate(usize bytes, u8 alignment = 8u)  override;
    void* reallocate(void* ptr, usize new_size)      override;
    void  free(void* ptr)                           override;

    inline unsigned int num_free_blocks() const
    {
        return free_list_size_;
    }

    inline unsigned int num_allocations() const
    {
        return (unsigned int)alloc_count_;
    }

    inline usize block_size(void* ptr) const
    {
        header* hdr = static_cast<header*>(ptr) - 1u;
        return hdr->size;
    }

private:
    // the location of the allocated memory will be
    // sizeof(header) + offset bytes after the guard bytes
    struct header
    {
        usize   size;
        u8      offset;
        u8      alignment;
    };

    struct free_block
    {
        usize size;
        free_block*  next;
    };

    static void* annotate_memory(void* memory, usize bytes, u8 align_offset);

    const static u32    guard_byte{ sizeof(u32) };
    const static u32    num_guard_bytes{ 2u * sizeof(u32) };
    const static u32    num_header_bytes{ sizeof(header) };
    const static usize  min_block_size;

    int               alloc_count_;
    unsigned int      free_list_size_;
    void*             arena_;
    usize             offset_;
    const usize       size_;
    free_block*       free_list_head_;
};

using free_list_locator = locator<memory_arena, 0>;
using system_memory_locator = locator<memory_arena, 1>;

// This class implements the Allocator concept in C++
// http://en.cppreference.com/w/cpp/memory/allocator
template<class T>
class polymorphic_allocator
{
public:
    template<class U> friend class polymorphic_allocator;
    using value_type = T;

    using pointer = T*;
    using const_pointer = T*;

    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    explicit polymorphic_allocator(memory_arena& arena = system_arena::get_instance())
        : arena_(arena)
    {}
    polymorphic_allocator(const polymorphic_allocator<T>&) = default;
    polymorphic_allocator<T>& operator=(const polymorphic_allocator<T>&) = default;
    polymorphic_allocator(polymorphic_allocator<T>&&) = default;
    polymorphic_allocator<T>& operator=(polymorphic_allocator&&) = default;
    ~polymorphic_allocator() = default;

    template<class U>
    polymorphic_allocator(polymorphic_allocator<U>&& other)
        : arena_(other.arena_)
    {}

    template<class U>
    polymorphic_allocator(const polymorphic_allocator<U>& other)
        : arena_(other.arena_)
    {}

    T* allocate(usize n)
    {
        return (T*)arena_.allocate(sizeof(T) * n, alignof(T));
    }

    void deallocate(T* ptr, usize)
    {
        // TODO: assert that n is the same as before
        arena_.free(ptr);
    }

    template<typename U>
    bool operator==(const polymorphic_allocator<U>& rhs) const
    {
        return &arena_ == &rhs.arena_;
    }

    template<typename U>
    bool operator!=(const polymorphic_allocator<U>& rhs) const
    {
        return &arena_ != &rhs.arena_;
    }

private:
    memory_arena& arena_;
};

}
