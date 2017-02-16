#include "nlrsMemory.h"
#include "UnitTest++/UnitTest++.h"

#include <utility>

namespace nlrs
{

SUITE(ScopedPtrTest)
{
    struct TestObject
    {
        bool* bptr;

        TestObject(bool* ptrBool)
            : bptr(ptrBool)
        {}

        ~TestObject()
        {
            *bptr = !*bptr;
        }
    };

    TEST(BoolOperatorFalseWhenNull)
    {
        ScopedPtr<TestObject> ptr(SystemAllocator::getInstance());
        CHECK(!bool(ptr));
    }

    TEST(DestructorOfOwnedObjectCalledOnPtrDestructor)
    {
        bool destructorWasCalled = false;
        {
            ScopedPtr<TestObject> ptr = makeScopedPtr<TestObject>(SystemAllocator::getInstance(), &destructorWasCalled);
        }
        CHECK(destructorWasCalled);
    }

    TEST(MoveAssignment)
    {
        bool destructorWasCalledOnce = false;
        {
            ScopedPtr<TestObject> ptr1 = makeScopedPtr<TestObject>(SystemAllocator::getInstance(), &destructorWasCalledOnce);
            CHECK(bool(ptr1));
            ScopedPtr<TestObject> ptr2 = std::move(ptr1);
            CHECK(!bool(ptr1));
            CHECK(bool(ptr2));
        }
        CHECK(destructorWasCalledOnce);
    }

    TEST(MoveConstruction)
    {
        bool destructorWasCalledOnce = false;
        {
            ScopedPtr<TestObject> ptr1 = makeScopedPtr<TestObject>(SystemAllocator::getInstance(), &destructorWasCalledOnce);
            CHECK(bool(ptr1));
            ScopedPtr<TestObject> ptr2(std::move(ptr1));
            CHECK(!bool(ptr1));
            CHECK(bool(ptr2));
        }
        CHECK(destructorWasCalledOnce);
    }
}

}
