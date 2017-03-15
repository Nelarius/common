#include "object_pool.h"
#include "UnitTest++/UnitTest++.h"

namespace nlrs
{

SUITE(object_pool_test)
{
    struct test_object
    {
        u64 uint;
        u8 byte;
    };

    struct object_pool_with_allocator
    {
        object_pool_with_allocator()
            : pool(system_arena::get_instance())
        {}

        object_pool<int, 4> pool;
    };

    struct object_pool_for_test_object
    {
        object_pool_for_test_object()
            : pool(system_arena::get_instance())
        {}

        object_pool<test_object, 4> pool;
    };

    TEST_FIXTURE(object_pool_with_allocator, after_initialization_size_is_zero)
    {
        CHECK_EQUAL(0u, pool.size());
    }

    TEST_FIXTURE(object_pool_with_allocator, after_creating_object_size_is_one)
    {
        int& i = *pool.create(4);
        CHECK_EQUAL(1u, pool.size());
        CHECK_EQUAL(4, i);
    }

    TEST_FIXTURE(object_pool_with_allocator, size_shrinks_after_releasing_object)
    {
        pool.create(1);
        pool.create(2);
        int* i = pool.create(3);
        CHECK_EQUAL(3u, pool.size());
        pool.release(i);
        CHECK_EQUAL(2u, pool.size());
    }

    TEST_FIXTURE(object_pool_with_allocator, creation_works_after_release)
    {
        pool.create(1);
        pool.create(2);
        int* i = pool.create(3);
        pool.release(i);
        i = pool.create(4);
        CHECK_EQUAL(3u, pool.size());
        CHECK_EQUAL(4, *i);
    }

    TEST_FIXTURE(object_pool_with_allocator, creation_works_after_filling_and_emptying_pool)
    {
        int* i1 = pool.create(1);
        int* i2 = pool.create(2);
        int* i3 = pool.create(3);
        int* i4 = pool.create(4);
        CHECK_EQUAL(4u, pool.size());
        pool.release(i1);
        pool.release(i2);
        pool.release(i3);
        pool.release(i4);
        CHECK_EQUAL(0u, pool.size());
        int* i = pool.create(5);
        CHECK_EQUAL(1u, pool.size());
        CHECK_EQUAL(5, *i);
    }

    TEST_FIXTURE(object_pool_for_test_object, can_initialize_object)
    {
        test_object* obj = pool.create(5u, u8(64u));
        CHECK_EQUAL(5u, obj->uint);
        CHECK_EQUAL(64u, obj->byte);
    }

    TEST_FIXTURE(object_pool_for_test_object, after_release_the_object_is_reused)
    {
        test_object* obj = pool.create(5u, u8(64u));
        test_object* ptr = obj;
        pool.release(obj);
        obj = pool.create(128u, u8(3u));
        CHECK_EQUAL(ptr, obj);
    }

    TEST(pool_returns_null_at_max_capacity)
    {
        object_pool<int, 3> pool(system_arena::get_instance());
        pool.create();
        pool.create();
        pool.create();
        CHECK_EQUAL((int*)nullptr, pool.create());
    }
}

}
