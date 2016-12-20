#include "nlrsAliases.h"
#include "nlrsAllocator.h"
#include "nlrsRandom.h"
#include "nlrsLiterals.h"
#include "nlrsArray.h"
#include "UnitTest++/UnitTest++.h"

namespace nlrs
{

// There are some tests here that don't really conform to the unit testing philosophy.
// The following tests mainly make sure that the implementation details of the allocator
// actually work as intended. Thus, if the allocator implementation changes, this needs
// to be changed too.

struct MemoryArena
{
    MemoryArena()
        : memory(std::malloc(1024 * 1024)),
        heap(memory, 1024 * 1024)
    {}

    ~MemoryArena()
    {
        std::free(memory);
    }

    void* memory;
    FreeListAllocator heap;
};

struct Header
{
    usize   size;
    u8      offset;
    u8      alignment;
};

struct FreeBlock
{
    usize       size;
    FreeBlock*  next;
};

SUITE(FreeListAllocatorTest)
{

    TEST_FIXTURE(MemoryArena, AlignmentIsCorrect)
    {
        void* block1 = heap.allocate(64, 4);
        void* block2 = heap.allocate(64, 8);
        void* block3 = heap.allocate(64, 16);
        void* block4 = heap.allocate(64, 32);

        NLRS_ASSERT(reinterpret_cast<uptr>(block1) % 4 == 0);
        NLRS_ASSERT(reinterpret_cast<uptr>(block2) % 8 == 0);
        NLRS_ASSERT(reinterpret_cast<uptr>(block3) % 16 == 0);
        NLRS_ASSERT(reinterpret_cast<uptr>(block4) % 32 == 0);

        Header* header1 = reinterpret_cast<Header*>(block1) - 1u;
        Header* header2 = reinterpret_cast<Header*>(block2) - 1u;
        Header* header3 = reinterpret_cast<Header*>(block3) - 1u;
        Header* header4 = reinterpret_cast<Header*>(block4) - 1u;
        NLRS_ASSERT(header1->alignment == 4u);
        NLRS_ASSERT(header2->alignment == 8u);
        NLRS_ASSERT(header3->alignment == 16u);
        NLRS_ASSERT(header4->alignment == 32u);

        heap.free(block1);
        heap.free(block2);
        heap.free(block3);
        heap.free(block4);
    }

    TEST_FIXTURE(MemoryArena, FreeListMerge)
    {
        void* block1 = heap.allocate(64);
        void* block2 = heap.allocate(64);
        void* block3 = heap.allocate(64);

        CHECK_EQUAL(heap.numAllocations(), 3);

        heap.free(block3);
        heap.free(block1);

        CHECK_EQUAL(1, heap.numAllocations());
        CHECK_EQUAL(2, heap.numFreeBlocks());

        heap.free(block2);
        CHECK_EQUAL(1, heap.numFreeBlocks());
        CHECK_EQUAL(0, heap.numAllocations());
    }

    TEST_FIXTURE(MemoryArena, FreeListReallocateContainsOriginalData)
    {
        Array<u8> array(heap);

        array.resize(64);
        std::memset(array.data(), 0, 64);
        array[0] = 128u;
        array[63] = 128;

        CHECK_EQUAL(array[0], 128u);
        CHECK_EQUAL(array[63], 128u);
    }
}

}
