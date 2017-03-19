#include "signal.h"
#include "literals.h"
#include "UnitTest++/UnitTest++.h"

namespace nlrs
{

SUITE(signal_test)
{
    struct callable
    {
        callable()
            : was_called(false)
        {}

        void call()
        {
            was_called = true;
        }

        void call_with_value(bool value)
        {
            was_called = value;
        }

        bool was_called;
    };

    TEST(connection_is_called)
    {
        bool was_called = false;

        signal<void> sig;
        sig.connect([&was_called]() -> void { was_called = true; });
        sig.emit();

        CHECK(was_called);
    }

    TEST(calling_after_removing_connection_has_no_side_effects)
    {
        bool was_not_called = true;

        signal<void> sig;
        signal<void>::handle handle = sig.connect([&was_not_called]() -> void { was_not_called = false; });

        CHECK_EQUAL(1_sz, sig.num_connections());

        sig.disconnect(handle);

        CHECK_EQUAL(0_sz, sig.num_connections());

        sig.emit();

        CHECK(was_not_called);
    }

    TEST(multiple_connections_are_called)
    {
        bool was_called1 = false;
        bool was_called2 = false;

        signal<void> sig;
        sig.connect([&was_called1]() -> void { was_called1 = true; });
        sig.connect([&was_called2]() ->void { was_called2 = true; });

        CHECK_EQUAL(2_sz, sig.num_connections());

        sig.emit();

        CHECK(was_called1);
        CHECK(was_called2);
    }

    TEST(adding_class_member_works)
    {
        callable c;

        signal<void> sig;
        sig.connect(&c, &callable::call);
        sig.emit();

        CHECK(c.was_called);
    }

    TEST(arguments_are_passed_to_slots)
    {
        bool arguments_forwarded = false;

        signal<bool> sig;
        sig.connect([&arguments_forwarded](bool value) -> void { arguments_forwarded = value; });
        sig.emit(true);

        CHECK(arguments_forwarded);
    }

    TEST(arguments_are_passed_to_member_functions)
    {
        callable c;

        signal<bool> sig;
        sig.connect(&c, &callable::call_with_value);
        sig.emit(true);

        CHECK(c.was_called);
    }
}

}
