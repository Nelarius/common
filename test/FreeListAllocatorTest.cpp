#include "aliases.h"
#include "memory_arena.h"
#include "nlrsRandom.h"
#include "nlrsLiterals.h"
#include "UnitTest++/UnitTest++.h"

#include "stl/vector.h"
#include <scoped_allocator>

namespace nlrs
{

// There are some tests here that don't really conform to the unit testing philosophy.
// The following tests mainly make sure that the implementation details of the allocator
// actually work as intended. Thus, if the allocator implementation changes, this needs
// to be changed too.

struct memory_container
{
    memory_container()
        : memory(std::malloc(1024 * 1024)),
        heap(memory, 1024 * 1024)
    {}

    ~memory_container()
    {
        std::free(memory);
    }

    void* memory;
    free_list_arena heap;
};

struct header
{
    usize   size;
    u8      offset;
    u8      alignment;
};

struct free_block
{
    usize       size;
    free_block*  next;
};

SUITE(FreeListAllocatorTest)
{

    TEST_FIXTURE(memory_container, AlignmentIsCorrect)
    {
        void* block1 = heap.allocate(64, 4);
        void* block2 = heap.allocate(64, 8);
        void* block3 = heap.allocate(64, 16);
        void* block4 = heap.allocate(64, 32);

        NLRS_ASSERT(reinterpret_cast<uptr>(block1) % 4 == 0);
        NLRS_ASSERT(reinterpret_cast<uptr>(block2) % 8 == 0);
        NLRS_ASSERT(reinterpret_cast<uptr>(block3) % 16 == 0);
        NLRS_ASSERT(reinterpret_cast<uptr>(block4) % 32 == 0);

        header* header1 = reinterpret_cast<header*>(block1) - 1u;
        header* header2 = reinterpret_cast<header*>(block2) - 1u;
        header* header3 = reinterpret_cast<header*>(block3) - 1u;
        header* header4 = reinterpret_cast<header*>(block4) - 1u;
        NLRS_ASSERT(header1->alignment == 4u);
        NLRS_ASSERT(header2->alignment == 8u);
        NLRS_ASSERT(header3->alignment == 16u);
        NLRS_ASSERT(header4->alignment == 32u);

        heap.free(block1);
        heap.free(block2);
        heap.free(block3);
        heap.free(block4);
    }

    TEST_FIXTURE(memory_container, FreeListMerge)
    {
        void* block1 = heap.allocate(64);
        void* block2 = heap.allocate(64);
        void* block3 = heap.allocate(64);

        CHECK_EQUAL(heap.num_allocations(), 3);

        heap.free(block3);
        heap.free(block1);

        CHECK_EQUAL(1, heap.num_allocations());
        CHECK_EQUAL(2, heap.num_free_blocks());

        heap.free(block2);
        CHECK_EQUAL(1, heap.num_free_blocks());
        CHECK_EQUAL(0, heap.num_allocations());
    }

    TEST_FIXTURE(memory_container, FreeListReallocateContainsOriginalData)
    {
        auto alloc = polymorphic_allocator<u8>(heap);
        std::pmr::vector<u8> vec(alloc);

        vec.resize(64);
        std::memset(vec.data(), 0, 64);
        vec[0] = 128u;
        vec[63] = 128;

        CHECK_EQUAL(vec[0], 128u);
        CHECK_EQUAL(vec[63], 128u);
    }
}

}
