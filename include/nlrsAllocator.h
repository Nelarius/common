#pragma once

#include "nlrsAliases.h"
#include "nlrsLocator.h"

namespace nlrs
{

class IAllocator
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
// This is intended for use in tests.
class SystemAllocator : public IAllocator
{
public:
    ~SystemAllocator();
    SystemAllocator(const SystemAllocator&) = delete;
    SystemAllocator& operator=(const SystemAllocator&) = delete;
    SystemAllocator(SystemAllocator&&) = delete;
    SystemAllocator& operator=(SystemAllocator&&) = delete;

    static SystemAllocator& getInstance();

    void* allocate(usize bytes, u8 alignment = 8u)  override;
    void* reallocate(void* ptr, usize newSize)      override;
    void  free(void* ptr)                           override;

private:
    SystemAllocator() = default;

    int allocCount_{ 0 };
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
class FreeListAllocator : public IAllocator
{
public:
    FreeListAllocator(void* memory, usize numBytes);
    FreeListAllocator() = delete;
    FreeListAllocator(const FreeListAllocator&) = delete;
    FreeListAllocator(FreeListAllocator&&) = delete;
    FreeListAllocator& operator=(const FreeListAllocator&) = delete;
    FreeListAllocator& operator=(FreeListAllocator&&) = delete;
    ~FreeListAllocator();

    void* allocate(usize bytes, u8 alignment = 8u)  override;
    void* reallocate(void* ptr, usize newSize)      override;
    void  free(void* ptr)                           override;

    inline unsigned int numFreeBlocks() const
    {
        return freeListSize_;
    }

    inline unsigned int numAllocations() const
    {
        return (unsigned int)allocCount_;
    }

    inline usize blockSize(void* ptr) const
    {
        Header* header = static_cast<Header*>(ptr) - 1u;
        return header->size;
    }

private:
    // the location of the allocated memory will be
    // sizeof(Header) + offset bytes after the guard bytes
    struct Header
    {
        usize   size;
        u8      offset;
        u8      alignment;
    };

    struct FreeBlock
    {
        usize size;
        FreeBlock*  next;
    };

    static void* annotateMemory_(void* memory, usize bytes, u8 alignOffset);

    const static u32    GuardByte{ sizeof(u32) };
    const static u32    GuardBytes{ 2u * sizeof(u32) };
    const static u32    HeaderBytes{ sizeof(Header) };
    const static usize  MinimumBlockSize;

    int               allocCount_;
    unsigned int      freeListSize_;
    void*             arena_;
    usize             offset_;
    const usize       Size_;
    FreeBlock*        freeListHead_;
};

using HeapAllocatorLocator = Locator<IAllocator, 0>;
using SystemAllocatorLocator = Locator<IAllocator, 1>;

}
