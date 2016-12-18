#include "nlrsAllocator.h"
#include "nlrsAssert.h"
#include "nlrsBitMath.h"
#include <algorithm>
#include <cstdlib>
#include <cstdint>

namespace
{

// Returns the offset as the number of bytes for the given pointer. The alignment
// must be a power of 2!
//
// This works by masking off the log2(alignment) least significant bits off
// the given address.

nlrs::u8 alignAddressForward(void* addr, nlrs::u8 alignment)
{
    NLRS_ASSERT((alignment & (alignment - 1u)) == 0u);
    nlrs::u8 offset = alignment - reinterpret_cast<nlrs::uptr>(addr) & static_cast<nlrs::uptr>(alignment - 1u);

    if (offset == alignment)
    {
        return 0u;
    }

    return offset;
}

}

namespace nlrs
{

/***
 *       ____         __            ___   ____              __
 *      / __/_ _____ / /____ __ _  / _ | / / /__  _______ _/ /____  ____
 *     _\ \/ // (_-</ __/ -_)  ' \/ __ |/ / / _ \/ __/ _ `/ __/ _ \/ __/
 *    /___/\_, /___/\__/\__/_/_/_/_/ |_/_/_/\___/\__/\_,_/\__/\___/_/
 *        /___/
 */

SystemAllocator::~SystemAllocator()
{
    NLRS_ASSERT(allocCount_ == 0);
}

SystemAllocator& SystemAllocator::getInstance()
{
    static SystemAllocator system;

    return system;
}

void* SystemAllocator::allocate(usize bytes, u8 alignment)
{
    allocCount_++;

    // TODO: figure out how to get aligned_alloc working
     //return std::aligned_alloc(bytes);

    return std::malloc(bytes);
}

void* SystemAllocator::reallocate(void* ptr, usize bytes)
{
    NLRS_ASSERT(ptr);
    return std::realloc(ptr, bytes);
}

void SystemAllocator::free(void* ptr)
{
    allocCount_--;
    std::free(ptr);
}

#ifdef NARWHAL_DEBUG
#define markAllocBytes(memory, numBytes) std::memset(memory, 0xa5, numBytes)
#define markFreeBytes(memory, numBytes) std::memset(memory, 0xee, numBytes)
#else
#define markAllocBytes(memory, bytes)
#define markFreeBytes(memory, bytes)
#endif

#define BEEFCAFE 0xfecaefbe

/***
 *       ____            __   _     __  ___   ____              __
 *      / __/______ ___ / /  (_)__ / /_/ _ | / / /__  _______ _/ /____  ____
 *     / _// __/ -_) -_) /__/ (_-</ __/ __ |/ / / _ \/ __/ _ `/ __/ _ \/ __/
 *    /_/ /_/  \__/\__/____/_/___/\__/_/ |_/_/_/\___/\__/\_,_/\__/\___/_/
 *
 */

FreeListAllocator::FreeListAllocator(void* memory, usize numBytes)
    : allocCount_(0),
    freeListSize_(0u),
    arena_(memory),
    offset_(0u),
    Size_(numBytes),
    freeListHead_(nullptr)
{}

FreeListAllocator::~FreeListAllocator()
{
    NLRS_ASSERT(allocCount_ == 0);
}

void* FreeListAllocator::annotateMemory_(void* memory, usize blockSize, u8 alignment)
{
    u8 alignOffset = alignAddressForward((static_cast<u8*>(memory) + GuardByte + HeaderBytes), alignment);

    // given a block of memory, write the magic number at the beginning & end of the block
    // let's reinterpret the memory block as an uint32_t pointer
    u32* guardMem = (u32*)memory;
    // if the block size isn't a multiple of uint32 at this point, then we're in trouble
    NLRS_ASSERT(blockSize % sizeof(u32) == 0u);
    guardMem[0u] = BEEFCAFE;
    guardMem[blockSize / sizeof(u32) - 1u] = BEEFCAFE;
    // advance the memory pointer past the first memory guard
    guardMem += 1u;

    // write the size of the block in memory
    Header* headerMem = reinterpret_cast<Header*>(reinterpret_cast<u8*>(guardMem) + alignOffset);
    headerMem->size = blockSize;
    headerMem->offset = alignOffset;
    headerMem->alignment = alignment;
    // advance past the header
    headerMem += 1u;
    markAllocBytes(headerMem, blockSize - GuardBytes - HeaderBytes - alignOffset);

    return headerMem;
}

// TODO:
// WTF, no idea why this needs to be here while the others can be initialized in the class declaration
const usize FreeListAllocator::MinimumBlockSize = sizeof(FreeListAllocator::FreeBlock);

void* FreeListAllocator::allocate(usize requestedBytes, u8 alignment)
{
    if (requestedBytes == 0u)
    {
        return nullptr;
    }

    usize blockSize = requestedBytes;
    // we need space for the header, as well as the guard bytes. Inflate the block size accordingly
    blockSize += HeaderBytes + GuardBytes + alignment;
    blockSize = std::max(MinimumBlockSize, blockSize);

    // we want all blocks to be powers of tywo in size to reduce fragmentation
    // the extra memory could be used in a realloc, for instance

    blockSize = usize(nextPowerOf2(blockSize));
    NLRS_ASSERT(blockSize >= sizeof(FreeBlock));
    NLRS_ASSERT(blockSize - HeaderBytes - GuardBytes >= requestedBytes);

    NLRS_ASSERT(blockSize < Size_ - offset_);

    ++allocCount_;

    // fetch the memory from the free list if possible
    FreeBlock* curBlock = freeListHead_;
    FreeBlock* prevBlock = nullptr;
    while (curBlock)
    {
        // TODO: this is not totally optimum, as it's done again in prepareMemory_
        u8 alignOffset = alignAddressForward((u8*)curBlock + GuardByte + HeaderBytes, alignment);
        if (curBlock->size < blockSize + usize(alignOffset))
        {
            prevBlock = curBlock;
            curBlock = curBlock->next;
            continue;
        }
        if (prevBlock)
        {
            prevBlock->next = curBlock->next;
        }
        else
        {
            freeListHead_ = freeListHead_->next;
        }

        --freeListSize_;

        //TODO:
        // If the rest of the block is large enough, then return it back to the free list!
        void* memory = curBlock;
        usize size = curBlock->size;
        return annotateMemory_(memory, size, alignment);
    }

    void* mem = static_cast<u8*>(arena_) + offset_;
    offset_ += blockSize;
    return annotateMemory_(mem, blockSize, alignment);
}

void* FreeListAllocator::reallocate(void* ptr, usize newSize)
{
    NLRS_ASSERT(ptr);
    NLRS_ASSERT(newSize != 0u);

    Header* header = reinterpret_cast<Header*>(ptr) - 1u;

    if (header->size - GuardBytes - HeaderBytes - header->offset >= newSize)
    {
        return ptr;
    }

    void* newPtr = allocate(newSize, header->alignment);
    std::memcpy(newPtr, ptr, header->size - GuardBytes - HeaderBytes - header->offset);
    free(ptr);
    return newPtr;
}

void FreeListAllocator::free(void* ptr)
{
    if (!ptr)
    {
        return;
    }

    Header* header = static_cast<Header*>(ptr) - 1u;
    usize blockSize = header->size;
    NLRS_ASSERT(blockSize >= sizeof(FreeBlock));

    u32* guardPtr = (u32*)((u8*)(header)-header->offset - GuardByte);

    NLRS_ASSERT(*guardPtr == BEEFCAFE);
    NLRS_ASSERT(*(guardPtr + blockSize / 4u - 1u) == BEEFCAFE);
    markFreeBytes(guardPtr, blockSize);

    // the following algorithm adds the newly freed block into the free list
    // it tries to merge adjacent free blocks into a larger block
    uptr blockStart = reinterpret_cast<uptr>(guardPtr);
    uptr blockEnd = blockStart + blockSize;
    FreeBlock* curBlock = freeListHead_;
    FreeBlock* prevBlock = nullptr;
    while (curBlock != nullptr)
    {
        if (reinterpret_cast<uptr>(curBlock) >= blockEnd)
        {
            break;
        }
        prevBlock = curBlock;
        curBlock = curBlock->next;
    }

    // freeListHead_ was null or freeListHead is beoynd the end of the block that we're
    // freeing
    if (prevBlock == nullptr)
    {
        prevBlock = reinterpret_cast<FreeBlock*>(blockStart);
        prevBlock->size = blockSize;
        prevBlock->next = freeListHead_;
        freeListHead_ = prevBlock;

        ++freeListSize_;
    }
    // prevBlock is adjacent to the block being freed
    else if (reinterpret_cast<uptr>(prevBlock) + prevBlock->size == blockStart)
    {
        prevBlock->size += blockSize;
    }
    // insert a new block in between curBlock and prevBlock
    // set the new block as prevBlock in preparation to check if the following
    // block is adjacent to the new block
    else
    {
        FreeBlock* newBlock = reinterpret_cast<FreeBlock*>(blockStart);
        newBlock->next = prevBlock->next;
        newBlock->size = blockSize;
        prevBlock->next = newBlock;
        prevBlock = newBlock;

        ++freeListSize_;
    }

    // check if curBlock is adjacent to the previous block
    // the previous block is either a merged block, or a new block
    if (curBlock != nullptr && reinterpret_cast<uptr>(curBlock) == blockEnd)
    {
        // we merge curBlock into prevBlock
        prevBlock->size += curBlock->size;
        prevBlock->next = curBlock->next;
        NLRS_ASSERT(freeListSize_ != 0u);
        --freeListSize_;
    }

    NLRS_ASSERT(allocCount_ > 0);
    --allocCount_;
}

}
