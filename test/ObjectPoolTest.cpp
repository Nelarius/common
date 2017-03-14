#include "nlrsObjectPool.h"
#include "UnitTest++/UnitTest++.h"

namespace nlrs
{

SUITE(ObjectPoolTest)
{
    struct TestObject
    {
        u64 uint;
        u8 byte;
    };
    struct ObjectPoolWithAllocator
    {
        ObjectPoolWithAllocator()
            : pool(system_arena::get_instance())
        {}

        ObjectPool<int, 4> pool;
    };

    struct ObjectPoolForTestObject
    {
        ObjectPoolForTestObject()
            : pool(system_arena::get_instance())
        {}

        ObjectPool<TestObject, 4> pool;
    };

    TEST_FIXTURE(ObjectPoolWithAllocator, AfterInitializationSizeIsZero)
    {
        CHECK_EQUAL(0u, pool.size());
    }

    TEST_FIXTURE(ObjectPoolWithAllocator, AfterCreatingObjectSizeIsOne)
    {
        int& i = *pool.create(4);
        CHECK_EQUAL(1u, pool.size());
        CHECK_EQUAL(4, i);
    }

    TEST_FIXTURE(ObjectPoolWithAllocator, SizeShrinksAfterReleasingObject)
    {
        pool.create(1);
        pool.create(2);
        int* i = pool.create(3);
        CHECK_EQUAL(3u, pool.size());
        pool.release(i);
        CHECK_EQUAL(2u, pool.size());
    }

    TEST_FIXTURE(ObjectPoolWithAllocator, CreationWorksAfterRelease)
    {
        pool.create(1);
        pool.create(2);
        int* i = pool.create(3);
        pool.release(i);
        i = pool.create(4);
        CHECK_EQUAL(3u, pool.size());
        CHECK_EQUAL(4, *i);
    }

    TEST_FIXTURE(ObjectPoolWithAllocator, CreationWorksAfterFillingAndEmptyingPool)
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

    TEST_FIXTURE(ObjectPoolForTestObject, CanInitializeObject)
    {
        TestObject* obj = pool.create(5u, u8(64u));
        CHECK_EQUAL(5u, obj->uint);
        CHECK_EQUAL(64u, obj->byte);
    }

    TEST_FIXTURE(ObjectPoolForTestObject, AfterReleaseTheObjectIsReused)
    {
        TestObject* obj = pool.create(5u, u8(64u));
        TestObject* ptr = obj;
        pool.release(obj);
        obj = pool.create(128u, u8(3u));
        CHECK_EQUAL(ptr, obj);
    }

    TEST(PoolReturnsNullAtMaxCapacity)
    {
        ObjectPool<int, 3> pool(system_arena::get_instance());
        pool.create();
        pool.create();
        pool.create();
        CHECK_EQUAL((int*)nullptr, pool.create());
    }
}

}
