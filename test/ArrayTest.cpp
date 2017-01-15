#include "nlrsAliases.h"
#include "nlrsAllocator.h"
#include "nlrsArray.h"
#include "nlrsLiterals.h"
#include "nlrsRandom.h"
#include "UnitTest++/UnitTest++.h"
#include <utility>
#include <vector>

namespace nlrs
{

SUITE(ArrayTest)
{
    struct ArrayWithAllocator
    {
        ArrayWithAllocator()
            : array(SystemAllocator::getInstance())
        {}

        Array<int> array;
    };

    struct ArrayWithElements
    {
        Array<int> array;

        ArrayWithElements()
            : array(SystemAllocator::getInstance(), 10)
        {
            for (int i = 0; i < 10; i++)
            {
                array.pushBack(i);
            }
        }
    };

    TEST_FIXTURE(ArrayWithAllocator, AfterInitializationSizeIsZero)
    {
        CHECK_EQUAL(0u, array.size());
    }

    TEST_FIXTURE(ArrayWithAllocator, ExpectedResultAfterPushBack)
    {
        array.pushBack(3);
        CHECK_EQUAL(1u, array.size());
        CHECK_EQUAL(3, array[0]);
        CHECK_EQUAL(3, array.front());
    }

    TEST_FIXTURE(ArrayWithElements, CorrectIndexReturnedFromEmplaceBack)
    {
        CHECK_EQUAL(10_sz, array.emplaceBack(5));
        CHECK_EQUAL(11_sz, array.emplaceBack(5));
    }

    TEST_FIXTURE(ArrayWithElements, CorrectIndexReturnedFromPushBack)
    {
        CHECK_EQUAL(10_sz, array.pushBack(5));
        CHECK_EQUAL(11_sz, array.pushBack(5));
    }

    TEST_FIXTURE(ArrayWithAllocator, ExpectedResultAfterEmplaceBack)
    {
        usize idx = array.emplaceBack(5);
        CHECK_EQUAL(0_sz, idx);
        CHECK_EQUAL(1u, array.size());
        CHECK_EQUAL(5, array[0]);
    }

    TEST_FIXTURE(ArrayWithAllocator, SwapAndRemoveIndexHasExpectedResultWithTwoElements)
    {
        array.pushBack(0);
        array.pushBack(2);
        array.swapAndRemove(0_sz);
        CHECK_EQUAL(2, array[0]);
    }

    TEST_FIXTURE(ArrayWithElements, SwapAndRemoveIteratorHasExpectedResult)
    {
        auto it = array.begin();
        array.swapAndRemove(it);
        CHECK_EQUAL(9, array.size());
        CHECK_EQUAL(9, array[0]);
        CHECK_EQUAL(9, array.front());
    }

    TEST_FIXTURE(ArrayWithElements, SwapAndRemoveReverseIteratorHasExpectedResult)
    {
        auto it = array.rbegin();
        ++it;
        array.swapAndRemove(it);
        CHECK_EQUAL(9, array.size());
        CHECK_EQUAL(9, array[8]);
        CHECK_EQUAL(9, array.back());
    }

    TEST_FIXTURE(ArrayWithElements, ExpectedResultAfterClear)
    {
        array.clear();
        CHECK_EQUAL(0u, array.size());
        array.pushBack(9);
        CHECK_EQUAL(9, array[0]);
        CHECK_EQUAL(1, array.size());
    }

    TEST_FIXTURE(ArrayWithElements, ExpectedResultAfterPopBack)
    {
        array.popBack();
        CHECK_EQUAL(9, array.size());
        CHECK_EQUAL(8, array.back());
        CHECK_EQUAL(0, array.front());
    }

    TEST_FIXTURE(ArrayWithElements, ElementsRemainAfterResize)
    {
        array.resize(20u);
        CHECK_EQUAL(20u, array.size());
        CHECK_EQUAL(0, array[0]);
        CHECK_EQUAL(1, array[1]);
        CHECK_EQUAL(2, array[2]);
        CHECK_EQUAL(3, array[3]);
        CHECK_EQUAL(4, array[4]);
        CHECK_EQUAL(5, array[5]);
        CHECK_EQUAL(6, array[6]);
        CHECK_EQUAL(7, array[7]);
        CHECK_EQUAL(8, array[8]);
        CHECK_EQUAL(9, array[9]);
    }

    TEST_FIXTURE(ArrayWithAllocator, EmptyArrayBeginAndEndIteratorsAreTheSame)
    {
        CHECK(array.begin() == array.end());
    }

    TEST_FIXTURE(ArrayWithAllocator, EmptyArrayReverseBeginAndEndIteratorAreTheSame)
    {
        CHECK(array.rbegin() == array.rend());
    }

    TEST_FIXTURE(ArrayWithAllocator, ForwardIteratorWorksAsExpected)
    {
        array.pushBack(1);
        array.pushBack(2);
        array.pushBack(3);
        auto it = array.begin();
        CHECK_EQUAL(1, *(it++));
        CHECK_EQUAL(2, *(it++));
        CHECK_EQUAL(3, *(it++));
        CHECK_EQUAL(array.end(), it);
    }

    TEST_FIXTURE(ArrayWithAllocator, ReverseIteratorWorksAsExpected)
    {
        array.pushBack(1);
        array.pushBack(2);
        array.pushBack(3);
        auto it = array.rbegin();
        CHECK_EQUAL(3, *(it++));
        CHECK_EQUAL(2, *(it++));
        CHECK_EQUAL(1, *(it++));
        //CHECK_EQUAL(array.rend(), it);
        CHECK(array.rend() == it);
    }

    TEST_FIXTURE(ArrayWithAllocator, MoveConstructorWorks)
    {
        array.pushBack(10);
        array.pushBack(20);
        Array<int> newArray(std::move(array));

        CHECK_EQUAL(0_sz, array.size());
        CHECK_EQUAL(2_sz, newArray.size());

        CHECK_EQUAL(10, newArray[0]);
        CHECK_EQUAL(20, newArray[1]);
    }

    TEST_FIXTURE(ArrayWithAllocator, MoveAssignmentOperatorWorks)
    {
        array.pushBack(10);
        array.pushBack(20);
        Array<int> newArray = std::move(array);

        CHECK_EQUAL(0_sz, array.size());
        CHECK_EQUAL(2_sz, newArray.size());

        CHECK_EQUAL(10, newArray[0]);
        CHECK_EQUAL(20, newArray[1]);
    }

    TEST_FIXTURE(ArrayWithAllocator, ResizeWorksAfterMove)
    {
        array.pushBack(10);
        array.pushBack(20);
        Array<int> newArray = std::move(array);

        CHECK_EQUAL(0_sz, array.size());
        CHECK_EQUAL(2_sz, newArray.size());

        array.pushBack(50);
        array.pushBack(60);
        CHECK_EQUAL(50, array[0u]);
        CHECK_EQUAL(60, array[1u]);
    }

    TEST_FIXTURE(ArrayWithAllocator, RandomTest)
    {
        const unsigned int NumRandomTests = 100000u;

        Random<float> randf;
        Random<int> randi;
        randf.seed(1234567u);

        std::vector<i32> values;

        enum class Operation
        {
            Add,
            Remove,
            Eval,
            Resize
        };

        const float probabilities[3] = { 0.68f, 0.48f, 0.05f };

        for (int i = 0; i < NumRandomTests; ++i)
        {
            float f = randf();
            Operation op;

            if (f - probabilities[2] < 0.f)
            {
                op = Operation::Resize;
            }
            else if (f - probabilities[1] < 0.f)
            {
                op = Operation::Eval;
            }
            else if (f - probabilities[0] < 0.f)
            {
                op = Operation::Remove;
            }
            else
            {
                op = Operation::Add;
            }

            switch (op)
            {
                case Operation::Add:
                {
                    i32 val = randi(0, 9999999);
                    values.push_back(val);
                    array.pushBack(val);
                    CHECK_EQUAL(usize(values.size()), array.size());
                    break;
                }
                case Operation::Remove:
                {
                    if (values.size() == 0u)
                    {
                        break;
                    }
                    usize index = usize(randi(0, i32(values.size() - 1u)));
                    std::swap(values[index], values[values.size() - 1u]);
                    values.pop_back();
                    array.swapAndRemove(index);
                    CHECK_EQUAL(usize(values.size()), array.size());
                    break;
                }
                case Operation::Eval:
                {
                    if (values.size() == 0u)
                    {
                        break;
                    }
                    usize index = usize(randi(0, i32(values.size() - 1u)));
                    CHECK_EQUAL(values[index], array[index]);
                    break;
                }
                case Operation::Resize:
                {
                    if (values.size() == 0u)
                    {
                        break;
                    }
                    values.resize(values.size() / 2u);
                    array.resize(array.size() / 2u);
                    CHECK_EQUAL(values.size(), array.size());
                    break;
                }
            }
        }
    }
}

SUITE(StaticArrayTest)
{
    TEST(ConstructStaticArrayFromInitializerList)
    {
        StaticArray<int, 3> array{1, 2, 3};
        CHECK_EQUAL(1, array.at(0u));
        CHECK_EQUAL(2, array.at(1u));
        CHECK_EQUAL(3, array.at(2u));
        CHECK_EQUAL(3_sz, array.size());
    }

    TEST(DefaultConstructedStaticArrayContainsNoElements)
    {
        StaticArray<int, 3> array;
        CHECK_EQUAL(0_sz, array.size());
    }

    TEST(PushBackElementsIntoStaticArray)
    {
        StaticArray<int, 5> array;
        array.pushBack(1);
        array.pushBack(2);
        array.pushBack(3);
        CHECK_EQUAL(1, array[0u]);
        CHECK_EQUAL(2, array[1u]);
        CHECK_EQUAL(3, array[2u]);
        CHECK_EQUAL(3_sz, array.size());
    }

    TEST(IterationOverElements)
    {
        StaticArray<int, 5> array = { 1, 2, 3, 4 };
        int values[4] = { 1, 2, 3, 4 };
        usize index = 0u;
        for (int value : array)
        {
            CHECK_EQUAL(values[index], value);
            index++;
        }
    }

    TEST(ReverseIterationOverElements)
    {
        StaticArray<int, 5> array = { 1, 2, 3, 4 };
        int values[4] = { 4, 3, 2, 1 };
        usize index = 0u;
        for (auto it = array.rbegin(); it != array.rend(); ++it)
        {
            CHECK_EQUAL(values[index], *it);
            index++;
        }
    }

    TEST(BeginEndIteratorsAreTheSameForEmptyContainer)
    {
        StaticArray<int, 3> array;
        CHECK(array.begin() == array.end());
    }

    TEST(ReverseBeginReverseEndIteratorsAreTheSameForEmptyContainer)
    {
        StaticArray<int, 3> array;
        CHECK(array.rbegin() == array.rend());
    }

    TEST(IsCopyAssignable)
    {
        StaticArray<int, 3> a1;
        a1.pushBack(1);
        a1.pushBack(2);
        a1.pushBack(3);
        StaticArray<int, 3> a2 = a1;

        CHECK_EQUAL(a1[0], a2[0]);
        CHECK_EQUAL(a1[1], a2[1]);
        CHECK_EQUAL(a1[2], a2[2]);
    }

    TEST(IsCopyConstructable)
    {
        StaticArray<int, 3> a1;
        a1.pushBack(1);
        a1.pushBack(2);
        a1.pushBack(3);
        StaticArray<int, 3> a2(a1);

        CHECK_EQUAL(a1[0], a2[0]);
        CHECK_EQUAL(a1[1], a2[1]);
        CHECK_EQUAL(a1[2], a2[2]);
    }
}

}
