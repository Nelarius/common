#include "memory_arena.h"
#include "nlrs_assert.h"
#include "bit_math.h"
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

nlrs::u8 align_address_forward(void* addr, nlrs::u8 alignment)
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

system_arena::~system_arena()
{
    NLRS_ASSERT(alloc_count_ == 0);
}

system_arena& system_arena::get_instance()
{
    static system_arena system;

    return system;
}

void* system_arena::allocate(usize bytes, u8 alignment)
{
    alloc_count_++;

    // TODO: figure out how to get aligned_alloc working
     //return std::aligned_alloc(bytes);

    return std::malloc(bytes);
}

void* system_arena::reallocate(void* ptr, usize bytes)
{
    NLRS_ASSERT(ptr);
    return std::realloc(ptr, bytes);
}

void system_arena::free(void* ptr)
{
    alloc_count_--;
    std::free(ptr);
}

#ifdef NLRS_DEBUG
#define mark_alloc_bytes(memory, num_bytes) std::memset(memory, 0xa5, num_bytes)
#define mark_free_bytes(memory, num_bytes) std::memset(memory, 0xee, num_bytes)
#else
#define mark_alloc_bytes(memory, bytes)
#define mark_free_bytes(memory, bytes)
#endif

#define BEEFCAFE 0xfecaefbe

/***
 *       ____            __   _     __  ___   ____              __
 *      / __/______ ___ / /  (_)__ / /_/ _ | / / /__  _______ _/ /____  ____
 *     / _// __/ -_) -_) /__/ (_-</ __/ __ |/ / / _ \/ __/ _ `/ __/ _ \/ __/
 *    /_/ /_/  \__/\__/____/_/___/\__/_/ |_/_/_/\___/\__/\_,_/\__/\___/_/
 *
 */

free_list_arena::free_list_arena(void* memory, usize numBytes)
    : alloc_count_(0),
    free_list_size_(0u),
    arena_(memory),
    offset_(0u),
    size_(numBytes),
    free_list_head_(nullptr)
{}

free_list_arena::~free_list_arena()
{
    NLRS_ASSERT(alloc_count_ == 0);
}

void* free_list_arena::annotate_memory(void* memory, usize block_size, u8 alignment)
{
    u8 align_offset = align_address_forward((static_cast<u8*>(memory) + guard_byte + num_header_bytes), alignment);

    // given a block of memory, write the magic number at the beginning & end of the block
    // let's reinterpret the memory block as an uint32_t pointer
    u32* guard_mem = (u32*)memory;
    // if the block size isn't a multiple of uint32 at this point, then we're in trouble
    NLRS_ASSERT(block_size % sizeof(u32) == 0u);
    guard_mem[0u] = BEEFCAFE;
    guard_mem[block_size / sizeof(u32) - 1u] = BEEFCAFE;
    // advance the memory pointer past the first memory guard
    guard_mem += 1u;

    // write the size of the block in memory
    header* hdr = reinterpret_cast<header*>(reinterpret_cast<u8*>(guard_mem) + align_offset);
    hdr->size = block_size;
    hdr->offset = align_offset;
    hdr->alignment = alignment;
    // advance past the header
    hdr += 1u;
    mark_alloc_bytes(hdr, block_size - num_guard_bytes - num_header_bytes - align_offset);

    return hdr;
}

// TODO:
// WTF, no idea why this needs to be here while the others can be initialized in the class declaration
const usize free_list_arena::min_block_size = sizeof(free_list_arena::free_block);

void* free_list_arena::allocate(usize num_requested_bytes, u8 alignment)
{
    if (num_requested_bytes == 0u)
    {
        return nullptr;
    }

    usize block_size = num_requested_bytes;
    // we need space for the header, as well as the guard bytes. Inflate the block size accordingly
    block_size += num_header_bytes + num_guard_bytes + alignment;
    block_size = std::max(min_block_size, block_size);

    // we want all blocks to be powers of tywo in size to reduce fragmentation
    // the extra memory could be used in a realloc, for instance

    block_size = usize(next_power_of_two(block_size));
    NLRS_ASSERT(block_size >= sizeof(free_block));
    NLRS_ASSERT(block_size - num_header_bytes - num_guard_bytes >= num_requested_bytes);

    NLRS_ASSERT(block_size < size_ - offset_);

    ++alloc_count_;

    // fetch the memory from the free list if possible
    free_block* cur_block = free_list_head_;
    free_block* prev_block = nullptr;
    while (cur_block)
    {
        // TODO: this is not totally optimum, as it's done again in prepareMemory_
        u8 alignOffset = align_address_forward((u8*)cur_block + guard_byte + num_header_bytes, alignment);
        if (cur_block->size < block_size + usize(alignOffset))
        {
            prev_block = cur_block;
            cur_block = cur_block->next;
            continue;
        }
        if (prev_block)
        {
            prev_block->next = cur_block->next;
        }
        else
        {
            free_list_head_ = free_list_head_->next;
        }

        --free_list_size_;

        //TODO:
        // If the rest of the block is large enough, then return it back to the free list!
        void* memory = cur_block;
        usize size = cur_block->size;
        return annotate_memory(memory, size, alignment);
    }

    void* mem = static_cast<u8*>(arena_) + offset_;
    offset_ += block_size;
    return annotate_memory(mem, block_size, alignment);
}

void* free_list_arena::reallocate(void* ptr, usize newSize)
{
    NLRS_ASSERT(ptr);
    NLRS_ASSERT(newSize != 0u);

    header* hdr = reinterpret_cast<header*>(ptr) - 1u;

    if (hdr->size - num_guard_bytes - num_header_bytes - hdr->offset >= newSize)
    {
        return ptr;
    }

    void* new_ptr = allocate(newSize, hdr->alignment);
    std::memcpy(new_ptr, ptr, hdr->size - num_guard_bytes - num_header_bytes - hdr->offset);
    free(ptr);
    return new_ptr;
}

void free_list_arena::free(void* ptr)
{
    if (!ptr)
    {
        return;
    }

    header* hdr = static_cast<header*>(ptr) - 1u;
    usize block_size = hdr->size;
    NLRS_ASSERT(block_size >= sizeof(free_block));

    u32* guard_ptr = (u32*)((u8*)(hdr)-hdr->offset - guard_byte);

    NLRS_ASSERT(*guard_ptr == BEEFCAFE);
    NLRS_ASSERT(*(guard_ptr + block_size / 4u - 1u) == BEEFCAFE);
    mark_free_bytes(guard_ptr, block_size);

    // the following algorithm adds the newly freed block into the free list
    // it tries to merge adjacent free blocks into a larger block
    uptr block_start = reinterpret_cast<uptr>(guard_ptr);
    uptr block_end = block_start + block_size;
    free_block* cur_block = free_list_head_;
    free_block* prev_block = nullptr;
    while (cur_block != nullptr)
    {
        if (reinterpret_cast<uptr>(cur_block) >= block_end)
        {
            break;
        }
        prev_block = cur_block;
        cur_block = cur_block->next;
    }

    // free_list_head_ was null or free_list_head_ is beoynd the end of the block that 
    // we're freeing
    if (prev_block == nullptr)
    {
        prev_block = reinterpret_cast<free_block*>(block_start);
        prev_block->size = block_size;
        prev_block->next = free_list_head_;
        free_list_head_ = prev_block;

        ++free_list_size_;
    }
    // prev_block is adjacent to the block being freed
    else if (reinterpret_cast<uptr>(prev_block) + prev_block->size == block_start)
    {
        prev_block->size += block_size;
    }
    // insert a new block in between cur_block and prev_block
    // set the new block as prev_block in preparation to check if the following
    // block is adjacent to the new block
    else
    {
        free_block* new_block = reinterpret_cast<free_block*>(block_start);
        new_block->next = prev_block->next;
        new_block->size = block_size;
        prev_block->next = new_block;
        prev_block = new_block;

        ++free_list_size_;
    }

    // check if cur_block is adjacent to the previous block
    // the previous block is either a merged block, or a new block
    if (cur_block != nullptr && reinterpret_cast<uptr>(cur_block) == block_end)
    {
        // we merge cur_block into prev_block
        prev_block->size += cur_block->size;
        prev_block->next = cur_block->next;
        NLRS_ASSERT(free_list_size_ != 0u);
        --free_list_size_;
    }

    NLRS_ASSERT(alloc_count_ > 0);
    --alloc_count_;
}

}
