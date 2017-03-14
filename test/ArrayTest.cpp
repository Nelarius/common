#include "aliases.h"
#include "resizable_array.h"
#include "nlrsLiterals.h"
#include "nlrsRandom.h"
#include "UnitTest++/UnitTest++.h"
#include <utility>
#include <vector>

namespace nlrs
{

SUITE(StaticArrayTest)
{
    TEST(ConstructStaticArrayFromInitializerList)
    {
        resizable_array<int, 3> array{1, 2, 3};
        CHECK_EQUAL(1, array.at(0u));
        CHECK_EQUAL(2, array.at(1u));
        CHECK_EQUAL(3, array.at(2u));
        CHECK_EQUAL(3_sz, array.size());
    }

    TEST(DefaultConstructedStaticArrayContainsNoElements)
    {
        resizable_array<int, 3> array;
        CHECK_EQUAL(0_sz, array.size());
    }

    TEST(PushBackElementsIntoStaticArray)
    {
        resizable_array<int, 5> array;
        array.push_back(1);
        array.push_back(2);
        array.push_back(3);
        CHECK_EQUAL(1, array[0u]);
        CHECK_EQUAL(2, array[1u]);
        CHECK_EQUAL(3, array[2u]);
        CHECK_EQUAL(3_sz, array.size());
    }

    TEST(IterationOverElements)
    {
        resizable_array<int, 5> array = { 1, 2, 3, 4 };
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
        resizable_array<int, 5> array = { 1, 2, 3, 4 };
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
        resizable_array<int, 3> array;
        CHECK(array.begin() == array.end());
    }

    TEST(ReverseBeginReverseEndIteratorsAreTheSameForEmptyContainer)
    {
        resizable_array<int, 3> array;
        CHECK(array.rbegin() == array.rend());
    }

    TEST(IsCopyAssignable)
    {
        resizable_array<int, 3> a1;
        a1.push_back(1);
        a1.push_back(2);
        a1.push_back(3);
        resizable_array<int, 3> a2 = a1;

        CHECK_EQUAL(a1[0], a2[0]);
        CHECK_EQUAL(a1[1], a2[1]);
        CHECK_EQUAL(a1[2], a2[2]);
    }

    TEST(IsCopyConstructable)
    {
        resizable_array<int, 3> a1;
        a1.push_back(1);
        a1.push_back(2);
        a1.push_back(3);
        resizable_array<int, 3> a2(a1);

        CHECK_EQUAL(a1[0], a2[0]);
        CHECK_EQUAL(a1[1], a2[1]);
        CHECK_EQUAL(a1[2], a2[2]);
    }
}

}
