#include "nlrsBuffer.h"
#include "nlrsLiterals.h"
#include "UnitTest++/UnitTest++.h"
#include <utility>

namespace nlrs
{

SUITE(BufferTest)
{
    struct BufferWithAllocator
    {
        BufferWithAllocator()
            : buffer(SystemAllocator::getInstance())
        {}

        Buffer<int> buffer;
    };

    TEST_FIXTURE(BufferWithAllocator, OnDefaultInitializationCapacityIsEight)
    {
        CHECK_EQUAL(8, buffer.capacity());
    }

    TEST_FIXTURE(BufferWithAllocator, CapacityGrowsAfterLargerResize)
    {
        buffer.reserve(32);
        CHECK_EQUAL(32, buffer.capacity());
    }

    TEST_FIXTURE(BufferWithAllocator, ContentsRemainTheSameAfterResize)
    {
        *buffer[0] = 1;
        *buffer[1] = 2;
        *buffer[2] = 3;
        *buffer[3] = 4;
        buffer.reserve(32);
        CHECK_EQUAL(1, *buffer[0]);
        CHECK_EQUAL(2, *buffer[1]);
        CHECK_EQUAL(3, *buffer[2]);
        CHECK_EQUAL(4, *buffer[3]);
    }

    TEST_FIXTURE(BufferWithAllocator, CapacityDoesNotChangeAfterSmallerResize)
    {
        std::size_t oldSize = buffer.capacity();
        buffer.reserve(4);
        CHECK(buffer.capacity() == oldSize);
    }

    TEST_FIXTURE(BufferWithAllocator, MoveConstructorWorks)
    {
        *buffer.at(0) = 0;
        *buffer.at(1) = 1;

        auto sizeWas = buffer.capacity();
        Buffer<int> newBuffer(std::move(buffer));

        CHECK_EQUAL(sizeWas, newBuffer.capacity());
        CHECK_EQUAL(0_sz, buffer.capacity());

        CHECK_EQUAL(0, *newBuffer.at(0));
        CHECK_EQUAL(1, *newBuffer.at(1));
    }

    TEST_FIXTURE(BufferWithAllocator, MoveAssignmentOperatorWorks)
    {
        *buffer.at(0) = 0;
        *buffer.at(1) = 1;
        auto sizeWas = buffer.capacity();
        Buffer<int> newBuffer = std::move(buffer);

        CHECK_EQUAL(sizeWas, newBuffer.capacity());
        CHECK_EQUAL(0_sz, buffer.capacity());

        CHECK_EQUAL(0, *newBuffer.at(0));
        CHECK_EQUAL(1, *newBuffer.at(1));
    }

    TEST_FIXTURE(BufferWithAllocator, ResizeAfterMove)
    {
        Buffer<int> newBuffer = std::move(buffer);
        buffer.reserve(8);
        CHECK_EQUAL(8_sz, buffer.capacity());
        *buffer[0] = 10;
        *buffer[1] = 20;
        CHECK_EQUAL(10, *buffer[0]);
        CHECK_EQUAL(20, *buffer[1]);
    }
}

}
