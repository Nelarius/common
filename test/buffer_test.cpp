#include "buffer.h"
#include "literals.h"
#include "UnitTest++/UnitTest++.h"
#include <utility>

namespace nlrs
{

SUITE(BufferTest)
{
    struct buffer_with_allocator
    {
        buffer_with_allocator()
            : buf(system_arena::get_instance())
        {}

        buffer<int> buf;
    };

    TEST_FIXTURE(buffer_with_allocator, on_default_initialization_capacity_is_eight)
    {
        CHECK_EQUAL(8, buf.capacity());
    }

    TEST_FIXTURE(buffer_with_allocator, capacity_grows_after_larger_resize)
    {
        buf.reserve(32);
        CHECK_EQUAL(32, buf.capacity());
    }

    TEST_FIXTURE(buffer_with_allocator, contents_remain_the_same_after_resize)
    {
        *buf[0] = 1;
        *buf[1] = 2;
        *buf[2] = 3;
        *buf[3] = 4;
        buf.reserve(32);
        CHECK_EQUAL(1, *buf[0]);
        CHECK_EQUAL(2, *buf[1]);
        CHECK_EQUAL(3, *buf[2]);
        CHECK_EQUAL(4, *buf[3]);
    }

    TEST_FIXTURE(buffer_with_allocator, capacity_does_not_change_after_smaller_resize)
    {
        std::size_t oldSize = buf.capacity();
        buf.reserve(4);
        CHECK(buf.capacity() == oldSize);
    }

    TEST_FIXTURE(buffer_with_allocator, move_constructor_works)
    {
        *buf.at(0) = 0;
        *buf.at(1) = 1;

        auto sizeWas = buf.capacity();
        buffer<int> new_buffer(std::move(buf));

        CHECK_EQUAL(sizeWas, new_buffer.capacity());
        CHECK_EQUAL(0_sz, buf.capacity());

        CHECK_EQUAL(0, *new_buffer.at(0));
        CHECK_EQUAL(1, *new_buffer.at(1));
    }

    TEST_FIXTURE(buffer_with_allocator, move_assignment_operator_works)
    {
        *buf.at(0) = 0;
        *buf.at(1) = 1;
        auto sizeWas = buf.capacity();
        buffer<int> new_buffer = std::move(buf);

        CHECK_EQUAL(sizeWas, new_buffer.capacity());
        CHECK_EQUAL(0_sz, buf.capacity());

        CHECK_EQUAL(0, *new_buffer.at(0));
        CHECK_EQUAL(1, *new_buffer.at(1));
    }

    TEST_FIXTURE(buffer_with_allocator, resize_after_move)
    {
        buffer<int> new_buffer = std::move(buf);
        buf.reserve(8);
        CHECK_EQUAL(8_sz, buf.capacity());
        *buf[0] = 10;
        *buf[1] = 20;
        CHECK_EQUAL(10, *buf[0]);
        CHECK_EQUAL(20, *buf[1]);
    }
}

}
